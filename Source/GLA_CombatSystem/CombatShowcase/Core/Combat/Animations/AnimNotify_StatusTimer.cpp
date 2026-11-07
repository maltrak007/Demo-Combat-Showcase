// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_StatusTimer.h"

#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_StatusTimer::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	UWorld* World = MeshComp ? MeshComp->GetWorld() : nullptr;
	UAnimMontage* Montage = Cast<UAnimMontage>(Animation);
	if (!World || !World->IsGameWorld() || !Montage) return; // not in the animation editor's preview

	const TWeakObjectPtr<USkeletalMeshComponent> WeakMesh(MeshComp);
	const TWeakObjectPtr<UAnimMontage> WeakMontage(Montage);
	const FName Loop = LoopSection;
	const FName End = EndSection;
	const bool bFinishLoop = bFinishLoopFirst;

	FTimerHandle Handle; // fire and forget: nothing is stored on the shared notify object
	World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakMesh, WeakMontage, Loop, End, bFinishLoop]()
	{
		UAnimInstance* Anim = WeakMesh.IsValid() ? WeakMesh->GetAnimInstance() : nullptr;
		UAnimMontage* M = WeakMontage.Get();
		if (!Anim || !M || !Anim->Montage_IsPlaying(M)) return; // already interrupted: nothing to do

		if (bFinishLoop) Anim->Montage_SetNextSection(Loop, End, M); // the loop's link becomes the get-up
		else Anim->Montage_JumpToSection(End, M);
	}), Duration, false);
}
