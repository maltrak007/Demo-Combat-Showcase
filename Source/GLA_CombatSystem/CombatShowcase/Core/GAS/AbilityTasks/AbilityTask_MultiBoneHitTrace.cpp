// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityTask_MultiBoneHitTrace.h"
#include "AbilitySystemComponent.h"
#include "CollisionShape.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/CombatFeedbackComponent.h"
#include "CombatShowcase/Core/Combat/Data/CombatData.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

static TAutoConsoleVariable<bool> CVarDrawHitboxTrace(
    TEXT("Combat.DrawHitboxTrace"),
    false,
    TEXT("Draw debug spheres and line for the active hitbox trace sweep."),
    ECVF_Cheat
);

bool UAbilityTask_MultiBoneHitTrace::ResolveHitboxLocation(const FCombatHitboxDef& Box, AActor* Avatar, FVector& OutLocation) const
{
    if (Box.Source == ECombatHitboxSource::WeaponSocket)
    {
        ABaseCombatCharacter* Character = Cast<ABaseCombatCharacter>(Avatar);
        UStaticMeshComponent* Weapon = Character ? Character->GetEquippedWeaponMesh() : nullptr;
        if (!Weapon || !Weapon->DoesSocketExist(Box.Bone))
        {
            UE_LOG(LogTemp, Warning, TEXT("HitboxTrace: weapon-socket hitbox '%s' unresolved on %s — skipping."), *Box.Bone.ToString(), *Avatar->GetName());
            return false;
        }
        OutLocation = Weapon->GetSocketLocation(Box.Bone);
        return true;
    }

    USkeletalMeshComponent* Mesh = Avatar->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh || !Mesh->DoesSocketExist(Box.Bone))
    {
        UE_LOG(LogTemp, Warning, TEXT("HitboxTrace: bare-bone hitbox '%s' unresolved on %s — skipping."), *Box.Bone.ToString(), *Avatar->GetName());
        return false;
    }
    OutLocation = Mesh->GetSocketLocation(Box.Bone);
    return true;
}

void UAbilityTask_MultiBoneHitTrace::HandleHitboxOpen(const FGameplayEventData* Payload)
{
    bWindowOpen = true;
    HitGroupsConsumed.Reset();
    LastBonePositions.Reset();

    if (AActor* Avatar = Ability->GetAvatarActorFromActorInfo())
    {
        for (const FCombatHitboxDef& Box : Hitboxes)
        {
            FVector Loc;
            if (ResolveHitboxLocation(Box, Avatar, Loc))
                LastBonePositions.Add(Box.Bone, Loc);
        }
    }
}

void UAbilityTask_MultiBoneHitTrace::HandleHitboxClose(const FGameplayEventData* Payload)
{
    bWindowOpen = false;
    EndTask();
}

void UAbilityTask_MultiBoneHitTrace::TickTask(float DeltaTime)
{
    if (bWindowOpen) PerformTrace();
}

UAbilityTask_MultiBoneHitTrace* UAbilityTask_MultiBoneHitTrace::CreateMultiBoneHitTrace(UGameplayAbility* OwningAbility, const TArray<FCombatHitboxDef>& InHitboxes, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageAmount, FCombatImpactFeel ImpactFeel)
{
    UAbilityTask_MultiBoneHitTrace* Task = NewAbilityTask<UAbilityTask_MultiBoneHitTrace>(OwningAbility);
    Task->Hitboxes = InHitboxes;
    Task->EffectClass = DamageEffectClass;
    Task->Damage = DamageAmount;
    Task->ImpactFeel = ImpactFeel;
    return Task;
}

void UAbilityTask_MultiBoneHitTrace::Activate()
{
    if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
    {
        OpenHandle  = ASC->GenericGameplayEventCallbacks.FindOrAdd(CombatTags::Event_Combat_HitboxOpen).AddUObject(this, &UAbilityTask_MultiBoneHitTrace::HandleHitboxOpen);
        CloseHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(CombatTags::Event_Combat_HitboxClose).AddUObject(this, &UAbilityTask_MultiBoneHitTrace::HandleHitboxClose);
    }
    bTickingTask = true;
}

void UAbilityTask_MultiBoneHitTrace::PerformTrace()
{
    AActor* Avatar = Ability->GetAvatarActorFromActorInfo();
    if (!Avatar) return;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Avatar);

    struct FPendingHit { const FCombatHitboxDef* Box; FHitResult Hit; };
    TMap<AActor*, FPendingHit> BestHitThisFrame;

    for (const FCombatHitboxDef& Box : Hitboxes)
    {
        FVector Current;
        if (!ResolveHitboxLocation(Box, Avatar, Current)) continue;
        const FVector Previous = LastBonePositions.FindRef(Box.Bone);

        if (CVarDrawHitboxTrace.GetValueOnGameThread())
        {
            DrawDebugSphere(GetWorld(), Current, Box.Radius, 12,
                Box.Source == ECombatHitboxSource::WeaponSocket ? FColor::Purple : FColor::Cyan, false, 0.f);
        }

        TArray<FHitResult> Hits;
        GetWorld()->SweepMultiByChannel(Hits, Previous, Current, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(Box.Radius), Params);

        for (const FHitResult& Hit : Hits)
        {
            AActor* Target = Hit.GetActor();
            if (!Target || HitGroupsConsumed.FindOrAdd(Box.Group).Contains(Target)) continue;

            if (FPendingHit* Existing = BestHitThisFrame.Find(Target))
            {
                if (Box.Rank < Existing->Box->Rank) { Existing->Box = &Box; Existing->Hit = Hit; }
            }
            else
            {
                BestHitThisFrame.Add(Target, { &Box, Hit });
            }
        }

        LastBonePositions[Box.Bone] = Current;
    }

    for (const auto& Pair : BestHitThisFrame)
    {
        HitGroupsConsumed.FindOrAdd(Pair.Value.Box->Group).Add(Pair.Key);
        ApplyDamage(Pair.Key, Pair.Value.Hit);
    }
}

void UAbilityTask_MultiBoneHitTrace::ApplyDamage(AActor* Target, const FHitResult& Hit)
{
    if (CVarDrawHitboxTrace.GetValueOnGameThread())
    {
        // Deliberately distinct from the continuous sweep spheres: fixed size, green,
        // and lingers 1.5s instead of redrawing every frame — a flash, not a trail.
        DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 8.f, 12, FColor::Green, false, 1.5f, 0, 2.f);

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
                FString::Printf(TEXT("Hit: %s for %.0f"), *Target->GetName(), Damage));
        }
    }
    if (IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(Target))
    {
        UAbilitySystemComponent* TargetASC = TargetInterface->GetAbilitySystemComponent();
        UAbilitySystemComponent* SourceASC = AbilitySystemComponent.Get();
        if (TargetASC && SourceASC && EffectClass)
        {
            AActor* Avatar = Ability->GetAvatarActorFromActorInfo();
            FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
            Context.AddInstigator(Avatar, Avatar);
            Context.AddHitResult(Hit); // FindClosestBone slots in here later, exactly as you planned
            
            FCombatImpactEvent Event;
            Event.Target = Target;
            Event.Instigator = Avatar;
            Event.ImpactFeel = ImpactFeel;
            Event.ImpactPoint = Hit.ImpactPoint;

            const float DirSign = (Target->GetActorLocation().X >= Avatar->GetActorLocation().X) ? 1.f : -1.f;
            Event.KnockbackVelocity = FVector(DirSign, 0.f, 0.f).GetSafeNormal() * ImpactFeel.KnockbackMagnitude; // zero Y — respects the locked plane

            if (ABaseCombatCharacter* TargetChar = Cast<ABaseCombatCharacter>(Target))
                if (auto* FC = TargetChar->GetFeedbackComponent()) FC->ReactToImpact(Event);
            if (ABaseCombatCharacter* AttackerChar = Cast<ABaseCombatCharacter>(Avatar))
                if (auto* FC = AttackerChar->GetFeedbackComponent()) FC->ReactToImpact(Event);
            
            FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(EffectClass, 1.f, Context);
            if (Spec.IsValid())
            {
                Spec.Data->SetSetByCallerMagnitude(CombatTags::Data_Damage, -Damage);
                SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
            }
        }
    }
    OnMultiHitDetected.Broadcast(Target);
}

void UAbilityTask_MultiBoneHitTrace::OnDestroy(bool bInOwnerFinished)
{
    if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
    {
        ASC->GenericGameplayEventCallbacks.FindOrAdd(CombatTags::Event_Combat_HitboxOpen).Remove(OpenHandle);
        ASC->GenericGameplayEventCallbacks.FindOrAdd(CombatTags::Event_Combat_HitboxClose).Remove(CloseHandle);
    }
    Super::OnDestroy(bInOwnerFinished);
}