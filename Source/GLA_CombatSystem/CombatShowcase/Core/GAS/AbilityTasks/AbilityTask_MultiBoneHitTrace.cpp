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

void UAbilityTask_MultiBoneHitTrace::HandleHitboxOpen(const FGameplayEventData* EventPayload)
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

void UAbilityTask_MultiBoneHitTrace::HandleHitboxClose(const FGameplayEventData* EventPayload)
{
    bWindowOpen = false;
    EndTask();
}

void UAbilityTask_MultiBoneHitTrace::TickTask(float DeltaTime)
{
    if (bWindowOpen) PerformTrace();
}


UAbilityTask_MultiBoneHitTrace* UAbilityTask_MultiBoneHitTrace::CreateMultiBoneHitTrace(UGameplayAbility* OwningAbility,
    const TArray<FCombatHitboxDef>& InHitboxes, const FCombatStrikePayload& InPayload)
{
    UAbilityTask_MultiBoneHitTrace* Task = NewAbilityTask<UAbilityTask_MultiBoneHitTrace>(OwningAbility);
    Task->Hitboxes = InHitboxes;
    Task->Payload = InPayload;
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
            if (!bWindowOpen) break;
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

        EHitOutcome Outcome = EHitOutcome::Unblocked;
        if (ABaseCombatCharacter* TargetChar = Cast<ABaseCombatCharacter>(Pair.Key))
        {
            if (UCombatFeedbackComponent* TargetFeedback = TargetChar->GetFeedbackComponent())
            {
                const FHitResolution Resolution = TargetFeedback->ResolveAndApplyHit(Avatar, Pair.Value.Hit, Payload);
                Outcome = Resolution.Outcome;

                if (CVarDrawHitboxTrace.GetValueOnGameThread())
                {
                    DrawDebugSphere(GetWorld(), Pair.Value.Hit.ImpactPoint, 8.f, 12, FColor::Green, false, 1.5f, 0, 2.f);
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
                            FString::Printf(TEXT("Hit: %s — outcome %d (dmg %.0f)"), *Pair.Key->GetName(), (int32)Outcome, Resolution.FinalDamage));
                    }
                }
            }
        }

        OnMultiHitDetected.Broadcast(Pair.Key, Outcome);
    }
}

void UAbilityTask_MultiBoneHitTrace::OnDestroy(bool bInOwnerFinished)
{
    bWindowOpen = false;
    if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
    {
        ASC->GenericGameplayEventCallbacks.FindOrAdd(CombatTags::Event_Combat_HitboxOpen).Remove(OpenHandle);
        ASC->GenericGameplayEventCallbacks.FindOrAdd(CombatTags::Event_Combat_HitboxClose).Remove(CloseHandle);
    }
    Super::OnDestroy(bInOwnerFinished);
}