// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_HitboxTrace.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Runtime/PhysicsCore/Public/CollisionShape.h"
#include "CombatShowcase/Core/GAS/CombatGameplayTags.h"

static TAutoConsoleVariable<bool> CVarDrawHitboxTrace(
    TEXT("Combat.DrawHitboxTrace"),
    false,
    TEXT("Draw debug spheres and line for the active hitbox trace sweep."),
    ECVF_Cheat
);

UAbilityTask_HitboxTrace* UAbilityTask_HitboxTrace::CreateHitboxTrace(UGameplayAbility* OwningAbility, FName StartSocket, FName EndSocket, float TraceRadius, TSubclassOf<UGameplayEffect> DamageEffectClass, float DamageAmount)
{
    UAbilityTask_HitboxTrace* Task = NewAbilityTask<UAbilityTask_HitboxTrace>(OwningAbility);
    Task->SocketStart = StartSocket;
    Task->SocketEnd = EndSocket;
    Task->Radius = TraceRadius;
    Task->EffectClass = DamageEffectClass;
    Task->Damage = DamageAmount;
    return Task;
}

void UAbilityTask_HitboxTrace::Activate()
{
    if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
    {
        OpenHandle  = ASC->GenericGameplayEventCallbacks.FindOrAdd(CombatTags::Event_Combat_HitboxOpen).AddUObject(this, &UAbilityTask_HitboxTrace::HandleHitboxOpen);
        CloseHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(CombatTags::Event_Combat_HitboxClose).AddUObject(this, &UAbilityTask_HitboxTrace::HandleHitboxClose);
    }
    bTickingTask = true;
}

void UAbilityTask_HitboxTrace::HandleHitboxOpen(const FGameplayEventData* Payload)
{
    bWindowOpen = true;
    AlreadyHitActors.Reset(); // one hit per target per swing, not per frame
}

void UAbilityTask_HitboxTrace::HandleHitboxClose(const FGameplayEventData* Payload)
{
    bWindowOpen = false;
    EndTask(); // ability's ActivateAbility loses its reference here; CancelAbility also reaches this via OnDestroy below
}

void UAbilityTask_HitboxTrace::TickTask(float DeltaTime)
{
    if (bWindowOpen) PerformTrace();
}

void UAbilityTask_HitboxTrace::PerformTrace()
{
    AActor* Avatar = Ability->GetAvatarActorFromActorInfo();
    USkeletalMeshComponent* Mesh = Avatar ? Avatar->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
    if (!Mesh) return;

    const FVector Start = Mesh->GetSocketLocation(SocketStart);
    const FVector End = Mesh->GetSocketLocation(SocketEnd);
    
    if (CVarDrawHitboxTrace.GetValueOnGameThread())
    {
        DrawDebugSphere(GetWorld(), Start, Radius, 12, FColor::Yellow, false, 0.f);
        DrawDebugSphere(GetWorld(), End, Radius, 12, FColor::Red, false, 0.f);
        DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, 0.f, 0, 1.5f);
    }
    
    TArray<FHitResult> Hits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Avatar);
    GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_GameTraceChannel1, FCollisionShape::MakeSphere(Radius), Params);

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || AlreadyHitActors.Contains(HitActor)) continue;
        AlreadyHitActors.Add(HitActor);

        if (IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(HitActor))
        {
            UAbilitySystemComponent* TargetASC = TargetInterface->GetAbilitySystemComponent();
            UAbilitySystemComponent* SourceASC = AbilitySystemComponent.Get();
            if (TargetASC && SourceASC && EffectClass)
            {
                FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
                Context.AddInstigator(Avatar, Avatar);
                Context.AddHitResult(Hit);

                FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(EffectClass, 1.f, Context);
                if (Spec.IsValid())
                {
                    Spec.Data->SetSetByCallerMagnitude(CombatTags::Data_Damage, -Damage); // negative: Health modifier op is Add
                    SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
                }
            }
        }
        OnHitDetected.Broadcast(HitActor);
    }
    
}

void UAbilityTask_HitboxTrace::OnDestroy(bool bInOwnerFinished)
{
    if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
    {
        ASC->GenericGameplayEventCallbacks.FindOrAdd(CombatTags::Event_Combat_HitboxOpen).Remove(OpenHandle);
        ASC->GenericGameplayEventCallbacks.FindOrAdd(CombatTags::Event_Combat_HitboxClose).Remove(CloseHandle);
    }
    Super::OnDestroy(bInOwnerFinished);
}

