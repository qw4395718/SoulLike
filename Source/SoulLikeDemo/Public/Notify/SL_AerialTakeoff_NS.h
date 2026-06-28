// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "SL_AerialTakeoff_NS.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "¸¡¿ÕÍ¨Öª"))
class SOULLIKEDEMO_API USL_AerialTakeoff_NS : public UAnimNotify
{
	GENERATED_BODY()

public:

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

public:

};
