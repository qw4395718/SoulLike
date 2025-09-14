// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "WeaponCoordinatorControl_IF.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType,meta = (CannotImplementInterfaceInBlueprint))
class UWeaponCoordinatorControl_IF : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOULLIKEDEMO_API IWeaponCoordinatorControl_IF
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 武器伤害获取(int -> enum)
	UFUNCTION()
		virtual float GetAttackComboDamage(int type) = 0;

	// 武器行为消耗获取(int -> enum)
	UFUNCTION()
		virtual float GetAttackStateCost(int type,int CostType) = 0;

};
