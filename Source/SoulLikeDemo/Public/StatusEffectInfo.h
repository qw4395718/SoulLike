#pragma once
#include "Engine/Texture2D.h"
#include "GameplayTagContainer.h"
#include "StatusEffectInfo.generated.h"

USTRUCT(BlueprintType)
struct FStatusEffectInfo
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FGameplayTag EffectTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RemainingTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Stacks;
};