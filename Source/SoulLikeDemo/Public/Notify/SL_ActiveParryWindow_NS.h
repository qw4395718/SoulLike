// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SL_ActiveParryWindow_NS.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API USL_ActiveParryWindow_NS : public UAnimNotifyState
{
	GENERATED_BODY()

public:

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
protected:
	// 弹反成功时的额外帧数（用于容错）
	UPROPERTY(EditAnywhere, Category = "Parry Settings")
		float SuccessWindowExtension = 0.05f; // 额外5帧@60FPS

public:
	UPROPERTY(EditAnywhere)
		bool bIsLHActive;
	UPROPERTY(EditAnywhere)
		bool bIsRHActive;
};
