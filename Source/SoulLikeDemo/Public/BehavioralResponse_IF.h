// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BehavioralResponse_IF.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType,meta = (CannotImplementInterfaceInBlueprint))
class UBehavioralResponse_IF : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOULLIKEDEMO_API IBehavioralResponse_IF
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	// 攻击行为响应
	UFUNCTION()
		virtual void AttackResponse() = 0;

	// 防御行为响应
	UFUNCTION()
		virtual void DefenceResponse() = 0;

	// 技能行为响应
	UFUNCTION()
		virtual void ComboSkillResponse() = 0;

	// 翻滚响应
	UFUNCTION()
		virtual void RollResponse() = 0;

	// 锁定响应
	UFUNCTION()
		virtual void LockRotationResponse() = 0;

	// 切换上方道具栏响应
	UFUNCTION()
		virtual void SwitchEquipemntUpBarResponse() = 0;

	// 切换下方道具栏响应
	UFUNCTION()
		virtual void SwitchEquipemntDownBarResponse() = 0;

	// 切换左手装备栏响应
	UFUNCTION()
		virtual void SwitchEquipemntLeftBarResponse() = 0;

	// 切换右手装备栏响应
	UFUNCTION()
		virtual void SwitchEquipemntRightBarResponse() = 0;

	// 使用道具响应
	UFUNCTION()
		virtual void UseItem() = 0;

};
