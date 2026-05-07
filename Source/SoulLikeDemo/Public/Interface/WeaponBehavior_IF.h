// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "WeaponAccessory_IF.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType,meta = (CannotImplementInterfaceInBlueprint))
class UWeaponAccessory_IF : public UInterface
{
	GENERATED_BODY()
};

/**
 * 武器访问接口
 */
class SOULLIKEDEMO_API IWeaponAccessory_IF
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 提供给动画通知获取武器引用
	UFUNCTION()
		virtual TSharedPtr<AWeaponBase> GetLeftHandWeaponReference() = 0;

	UFUNCTION()
		virtual TSharedPtr<AWeaponBase> GetRightHandWeaponReference() = 0;
};
