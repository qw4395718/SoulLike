#pragma once
#include "Engine/Texture2D.h"
#include "StatusEffectInfo.generated.h"

USTRUCT(BlueprintType)
struct FStatusEffectInfo
{
	GENERATED_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int IconIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString IconPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Duration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RemainingTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 Stacks;
};