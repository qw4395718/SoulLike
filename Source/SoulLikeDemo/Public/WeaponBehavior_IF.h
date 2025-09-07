// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameFramework/Actor.h"
#include "WeaponBehavior_IF.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType,meta = (CannotImplementInterfaceInBlueprint))
class UWeaponBehavior_IF : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOULLIKEDEMO_API IWeaponBehavior_IF
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 攻击行为响应
	UFUNCTION()
		virtual float AttackBehaviorResponse(AActor* OwnerActor) = 0;

	// 防御行为响应
	UFUNCTION()
		virtual float DefenceBehaviorResponse(AActor* OwnerActor) = 0;

	// 技能行为响应
	UFUNCTION()
		virtual float ComboSkillBehaviorResponse(AActor* OwnerActor) = 0;

	// 处决行为响应
	UFUNCTION()
		virtual float ExecuteBehaviorResponse(AActor* OwnerActor) = 0;

	// 背刺行为响应
	UFUNCTION()
		virtual float BackStabBehaviorResponse(AActor* OwnerActor) = 0;

};
