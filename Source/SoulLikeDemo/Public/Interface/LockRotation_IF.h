// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LockRotation_IF.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType,meta = (CannotImplementInterfaceInBlueprint))
class ULockRotation_IF : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOULLIKEDEMO_API ILockRotation_IF
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	// 是否可以被锁视角
	UFUNCTION()
		virtual bool IsCanLockRotation() = 0;

	// 提供用于锁定的目标点位置和旋转
	UFUNCTION()
		virtual void GetLockRotationLocation(FVector& vLocationInWorld) = 0;
};
