// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SoulLike_Parry_NS.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API USoulLike_Parry_NS : public UAnimNotifyState
{
	GENERATED_BODY()

public:

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	
protected:
	// 弹反成功时的额外帧数（用于容错）
	UPROPERTY(EditAnywhere, Category = "Parry Settings")
		float SuccessWindowExtension = 0.05f; // 额外5帧@60FPS
};
