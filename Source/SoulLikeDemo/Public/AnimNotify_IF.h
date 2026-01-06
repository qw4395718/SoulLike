// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "AnimNotify_IF.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType,meta = (CannotImplementInterfaceInBlueprint))
class UAnimNotify_IF : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOULLIKEDEMO_API IAnimNotify_IF
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 动画(状态)通知响应
	UFUNCTION()
		virtual void AnimNotifyResponse(int NotifyType) = 0;

};
