// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_WeaponTrail.h"

#include "NiagaraFunctionLibrary.h"
#include "CombatShowcase/Core/Combat/BaseCombatCharacter.h"
#include "CombatShowcase/Core/Combat/Components/WeaponComponent.h"
#include "CombatShowcase/Core/Combat/Data/PDT_Weapon.h"

namespace
{
	struct FTrailSockets
	{
		UMeshComponent* Mesh = nullptr;
		FName Start;
		FName End;
		bool IsValid() const { return Mesh != nullptr; }
	};

	// The sockets come from the drawn weapon's asset: the first and last trace points are the two ends.
	// Returns empty when unarmed, sheathed, the asset has fewer than two points, or a socket is missing.
	FTrailSockets ResolveTrail(const USkeletalMeshComponent* MeshComp)
	{
		const ABaseCombatCharacter* Owner = MeshComp ? Cast<ABaseCombatCharacter>(MeshComp->GetOwner()) : nullptr;
		const UWeaponComponent* Weapons = Owner ? Owner->GetWeaponComponent() : nullptr;
		UMeshComponent* Mesh = Owner ? Owner->GetEquippedWeaponMesh() : nullptr;
		const UPDT_Weapon* Data = (Weapons && Weapons->IsDrawn()) ? Weapons->GetCarried() : nullptr;
		if (!Mesh || !Data) return {}; // unarmed, sheathed, or the animation editor's preview


		FName Start, End;
		if (!Data->GetTrailSockets(Start, End)) return {}; // needs a Base and an End point
		if (!Mesh->DoesSocketExist(Start) || !Mesh->DoesSocketExist(End)) return {};
		return {Mesh, Start, End};
	}
}


void UAnimNotify_WeaponTrail::PushEndPoint(UNiagaraComponent* Fx, UMeshComponent* Mesh, FName StartSocket,
                                           FName EndSocket) const
{
	if (!StartPointParameterName.IsNone())
	{
		const FVector StartPoint = Mesh->GetSocketLocation(StartSocket);
		if (bEndPointIsPosition) Fx->SetVariablePosition(StartPointParameterName, StartPoint);
		else Fx->SetVariableVec3(StartPointParameterName, StartPoint);
	}
	FVector EndPoint = Mesh->GetSocketLocation(EndSocket);
	if (bEndPointRelativeToStart)
	{
		EndPoint = Mesh->GetSocketTransform(StartSocket, RTS_World).InverseTransformPosition(EndPoint);
	}
	if (bEndPointIsPosition) Fx->SetVariablePosition(EndPointParameterName, EndPoint);
	else Fx->SetVariableVec3(EndPointParameterName, EndPoint);
}

void UAnimNotify_WeaponTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (!MeshComp || !NiagaraTemplate) return;

	const FTrailSockets Trail = ResolveTrail(MeshComp);
	if (!Trail.IsValid()) return;

	// Drop entries for meshes destroyed before their NotifyEnd. The notify object outlives them.
	for (auto It = ActiveTrails.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid()) It.RemoveCurrent();
	}

	// A re-trigger on the same mesh replaces the old trail.
	TWeakObjectPtr<UNiagaraComponent> Old;
	if (ActiveTrails.RemoveAndCopyValue(MeshComp, Old))
	{
		if (UNiagaraComponent* OldFx = Old.Get()) OldFx->Deactivate();
	}

	// Spawned inactive, so the end point is set before the first particle is emitted.
	UNiagaraComponent* Fx = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraTemplate, Trail.Mesh, Trail.Start, FVector::ZeroVector, FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget, true, false);
	if (!IsValid(Fx)) return;

	PushEndPoint(Fx, Trail.Mesh, Trail.Start, Trail.End);
	if (bOverrideLook)
	{
		Fx->SetVariableLinearColor(TEXT("User.HeadColor"), HeadColor);
		Fx->SetVariableLinearColor(TEXT("User.TailColor"), TailColor);
		Fx->SetVariableFloat(TEXT("User.FadePower"), FadePower);
		Fx->SetVariableFloat(TEXT("User.Lifetime"), Lifetime);
		Fx->SetVariableFloat(TEXT("User.SpawnRate"), SpawnRate);
		Fx->SetVariableFloat(TEXT("User.WidthScale"), WidthScale);
	}
	Fx->Activate(true);
	ActiveTrails.Add(MeshComp, Fx);
}

void UAnimNotify_WeaponTrail::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                         float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);
	if (!MeshComp) return;

	const TWeakObjectPtr<USkeletalMeshComponent> Key(MeshComp);
	const TWeakObjectPtr<UNiagaraComponent>* Found = ActiveTrails.Find(Key);
	UNiagaraComponent* Fx = Found ? Found->Get() : nullptr;
	if (!Fx) return;

	const FTrailSockets Trail = ResolveTrail(MeshComp);
	if (Trail.IsValid())
	{
		PushEndPoint(Fx, Trail.Mesh, Trail.Start, Trail.End);
	}
	else
	{
		Fx->Deactivate(); // the weapon was sheathed mid-swing
		ActiveTrails.Remove(Key);
	}
}

void UAnimNotify_WeaponTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (!MeshComp) return;

	TWeakObjectPtr<UNiagaraComponent> Trail;
	if (ActiveTrails.RemoveAndCopyValue(MeshComp, Trail))
	{
		if (UNiagaraComponent* Fx = Trail.Get()) Fx->Deactivate();
		// live particles finish, then bAutoDestroy removes it
	}
}
