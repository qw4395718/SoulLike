// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SoulLike_ActiveParry_CB_NS.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API USoulLike_ActiveParry_CB_NS : public UAnimNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

public:
	UPROPERTY(EditAnywhere)
		bool bIsLHActive;
	UPROPERTY(EditAnywhere)
		bool bIsRHActive;
};
