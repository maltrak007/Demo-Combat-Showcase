// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotify_WeaponTrail.generated.h"

class UPDT_Weapon;
class UNiagaraSystem;
class UNiagaraComponent;

/**
 * 
 */
UCLASS()
class GLA_COMBATSYSTEM_API UAnimNotify_WeaponTrail : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Trail")
	TObjectPtr<UNiagaraSystem> NiagaraTemplate;
	
	UPROPERTY(EditAnywhere, Category = "Trail")
	FName StartPointParameterName = NAME_None;
	// The user parameter that receives the trail's far end, with its namespace as it appears in Niagara.
	UPROPERTY(EditAnywhere, Category = "Trail")
	FName EndPointParameterName = "User.EndPoint";
	// Match the parameter's type in the system: Position, or Vector3 when this is off.
	UPROPERTY(EditAnywhere, Category = "Trail")
	bool bEndPointIsPosition = false;
	// Off: the end point is a world position. On: relative to the start socket, for a local-space emitter.
	UPROPERTY(EditAnywhere, Category = "Trail")
	bool bEndPointRelativeToStart = false;
	
	UPROPERTY(EditAnywhere, Category = "Trail|Look")
	bool bOverrideLook = false;
	UPROPERTY(EditAnywhere, Category = "Trail|Look", meta = (EditCondition = "bOverrideLook"))
	FLinearColor HeadColor = FLinearColor(0.f, 1.f, 1.f, 1.f);
	UPROPERTY(EditAnywhere, Category = "Trail|Look", meta = (EditCondition = "bOverrideLook"))
	FLinearColor TailColor = FLinearColor(1.f, 0.f, 0.6f, 0.f);
	UPROPERTY(EditAnywhere, Category = "Trail|Look", meta = (EditCondition = "bOverrideLook"))
	float FadePower = 1.f;
	UPROPERTY(EditAnywhere, Category = "Trail|Look", meta = (EditCondition = "bOverrideLook"))
	float Lifetime = 0.3f;
	UPROPERTY(EditAnywhere, Category = "Trail|Look", meta = (EditCondition = "bOverrideLook"))
	float SpawnRate = 20.f;
	UPROPERTY(EditAnywhere, Category = "Trail|Look", meta = (EditCondition = "bOverrideLook"))
	float WidthScale = 1.f;
	
protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                         const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	                        const FAnimNotifyEventReference& EventReference) override;
	
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	                       const FAnimNotifyEventReference& EventReference) override;

private:
	void PushEndPoint(UNiagaraComponent* Fx, UMeshComponent* Mesh, FName StartSocket, FName EndSocket) const;

	TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>> ActiveTrails;
};
