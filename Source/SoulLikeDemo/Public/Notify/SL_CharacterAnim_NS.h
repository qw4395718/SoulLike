// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SoulLikeGameGlobal.h"
#include "SL_CharacterAnim_NS.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API USL_CharacterAnim_NS : public UAnimNotifyState
{
	GENERATED_BODY()

public:

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	UPROPERTY(EditAnywhere)
		EAnimNotifyType AnimStartNotify;

	UPROPERTY(EditAnywhere)
		EAnimNotifyType AnimEndNotify;
};
