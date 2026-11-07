// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEnemyAIController.h"

#include "AbilitySystemComponent.h"
#include "EnemyBaseCharacter.h"
#include "CombatShowcase/Core/Combat/Components/EnemyBehaviourComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Behaviour/BehaviourData.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AISenseConfig_Sight.h"


// Sets default values
ABaseEnemyAIController::ABaseEnemyAIController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*Perception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	// The component only gets a listener id at registration if a sense is already configured,
	// so this has to happen here, not in OnPossess. The tuning is applied later.
	Perception->ConfigureSense(*SightConfig);
	Perception->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ABaseEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}
FGenericTeamId ABaseEnemyAIController::GetGenericTeamId() const
{
	const IGenericTeamAgentInterface* Agent = Cast<IGenericTeamAgentInterface>(GetPawn());
	return Agent ? Agent->GetGenericTeamId() : FGenericTeamId();
}

void ABaseEnemyAIController::SetGenericTeamId(const FGenericTeamId& NewID)
{
	if (IGenericTeamAgentInterface* Agent = Cast<IGenericTeamAgentInterface>(GetPawn()))
	{
		Agent->SetGenericTeamId(NewID);
	}
}

void ABaseEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (const IGenericTeamAgentInterface* Agent = Cast<IGenericTeamAgentInterface>(InPawn))
	{
		SetGenericTeamId(Agent->GetGenericTeamId());
	}
	ApplyPerceptionSettings(DefaultPerception);
	if (const AEnemyBaseCharacter* BehaviourOwner = Cast<AEnemyBaseCharacter>(InPawn))
	{
		// A behaviour row that sets its own perception replaces the controller's defaults.
		if (const UEnemyBehaviourComponent* Behaviour = BehaviourOwner->GetBehaviourComponent())
		{
			const FWeaponBehaviourRow& Stats = Behaviour->GetStats();
			if (Stats.bOverridePerception) ApplyPerceptionSettings(Stats.Perception);
		}
	}

	AEnemyBaseCharacter* Enemy = Cast<AEnemyBaseCharacter>(InPawn);
	if (!Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s possessed %s, which isn't an AEnemyBaseCharacter. No AI started."), *GetName(), *GetNameSafe(InPawn));
		return;
	}
	if (!BehaviorTreeAsset)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no BehaviorTreeAsset assigned."), *GetName());
		return;
	}
	if (!RunBehaviorTree(BehaviorTreeAsset))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: RunBehaviorTree failed for %s."), *GetName(), *BehaviorTreeAsset->GetName());
		return;
	}
	ValidateBlackboardKeys();

	// Bound only now that the blackboard exists, so no early perception event is dropped.
	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.RemoveAll(this);
		Perception->OnTargetPerceptionUpdated.AddDynamic(this, &ABaseEnemyAIController::OnTargetPerceptionUpdated);
	}

	if (UAbilitySystemComponent* ASC = Enemy->GetAbilitySystemComponent())
	{
		FOnGameplayEffectTagCountChanged& Event = ASC->RegisterGameplayTagEvent(
			CombatTags::State_Combat_BeingExecuted, EGameplayTagEventType::NewOrRemoved);
		Event.RemoveAll(this);
		Event.AddUObject(this, &ABaseEnemyAIController::OnExecutionTagChanged);
	}

	GetBlackboardComponent()->SetValueAsVector(EnemyBBKeys::HomeLocation, InPawn->GetActorLocation());
}

void ABaseEnemyAIController::OnUnPossess()
{
	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.RemoveAll(this);
	}
	if (const ABaseCombatCharacter* OwnerPawn = Cast<ABaseCombatCharacter>(GetPawn()))
	{
		if (UAbilitySystemComponent* ASC = OwnerPawn->GetAbilitySystemComponent())
		{
			ASC->RegisterGameplayTagEvent(CombatTags::State_Combat_BeingExecuted, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
		}
	}
	GetWorldTimerManager().ClearTimer(ForgetTargetHandle);
	Super::OnUnPossess();
}

void ABaseEnemyAIController::OnExecutionTagChanged(FGameplayTag Tag, int32 NewCount)
{
	UBrainComponent* Brain = GetBrainComponent();
	if (!Brain) return;

	if (NewCount > 0)
	{
		Brain->StopLogic(TEXT("Being executed"));
	}
	else if (const ABaseCombatCharacter* OwnerPawn = Cast<ABaseCombatCharacter>(GetPawn()); OwnerPawn && OwnerPawn->GetHealthPercent() > 0.f)
	{
		Brain->RestartLogic(); 
	}
}

bool ABaseEnemyAIController::ValidateBlackboardKeys() const
{
	const UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return false;

	bool bAllPresent = true;
	for (const FName& Key : { EnemyBBKeys::TargetActor, EnemyBBKeys::LastKnownPos, EnemyBBKeys::HasLOS,
							  EnemyBBKeys::AIState, EnemyBBKeys::MoveToLocation, EnemyBBKeys::HomeLocation,
								  EnemyBBKeys::HasAttackToken, EnemyBBKeys::AttackSideSign, EnemyBBKeys::AttackLocation})
	{
		if (BB->GetKeyID(Key) == FBlackboard::InvalidKey)
		{
			UE_LOG(LogTemp, Error, TEXT("%s: the blackboard has no key named '%s'."), *GetName(), *Key.ToString());
			bAllPresent = false;
		}
	}
	return bAllPresent;
}

void ABaseEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UE_LOG(LogTemp, Log, TEXT("%s perception update: %s sensed=%d"), *GetName(), *GetNameSafe(Actor), Stimulus.WasSuccessfullySensed());

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!IsValid(Actor) || !BB) return;
	if (Stimulus.Type != UAISense::GetSenseID<UAISense_Sight>()) return;

	const bool bSensed = Stimulus.WasSuccessfullySensed();
	BB->SetValueAsBool(EnemyBBKeys::HasLOS, bSensed);

	if (bSensed)
	{
		BB->SetValueAsObject(EnemyBBKeys::TargetActor, Actor);
		BB->SetValueAsVector(EnemyBBKeys::LastKnownPos, Stimulus.StimulusLocation);
		GetWorldTimerManager().ClearTimer(ForgetTargetHandle);
	}
	else if (BB->GetValueAsObject(EnemyBBKeys::TargetActor) == Actor)
	{
		// Only the current target counts. Calling SetTimer on the same handle restarts the countdown.
		if (TargetMemorySeconds <= 0.f) ForgetTarget();
		else GetWorldTimerManager().SetTimer(ForgetTargetHandle, this, &ABaseEnemyAIController::ForgetTarget, TargetMemorySeconds, false);
	}
}

void ABaseEnemyAIController::ForgetTarget()
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->ClearValue(EnemyBBKeys::TargetActor);
		BB->SetValueAsBool(EnemyBBKeys::HasLOS, false);
		UE_LOG(LogTemp, Log, TEXT("%s forgot its target."), *GetName());
	}
}

ETeamAttitude::Type ABaseEnemyAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const IGenericTeamAgentInterface* OtherAgent = Cast<const IGenericTeamAgentInterface>(&Other))
	{
		return FGenericTeamId::GetAttitude(GetGenericTeamId(), OtherAgent->GetGenericTeamId());
	}
	return ETeamAttitude::Neutral;
}

void ABaseEnemyAIController::ApplyPerceptionSettings(const FEnemyPerceptionSettings& Settings)
{
	UAIPerceptionComponent* Perception = GetPerceptionComponent();
	if (!Perception || !SightConfig) return;

	SightConfig->SightRadius = Settings.SightRadius;
	SightConfig->LoseSightRadius = FMath::Max(Settings.LoseSightRadius, Settings.SightRadius);
	SightConfig->PeripheralVisionAngleDegrees = Settings.PeripheralVisionAngle;
	TargetMemorySeconds = Settings.TargetMemorySeconds;
	

	// Values only: the sense was configured in the constructor. This pushes them to the live listener.
	Perception->RequestStimuliListenerUpdate();
}

