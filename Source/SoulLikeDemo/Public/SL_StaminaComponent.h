// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "SL_StaminaComponent.generated.h"

class AWeaponBase;
class ASoulLikeCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_StaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_StaminaComponent();

public:
	/************************************************************************/
	/*                                    外部调用                                  */
	/************************************************************************/
	UFUNCTION()
		void InitStaminaInfo();

	UFUNCTION()
		bool GetIsStaminaZero();

	UFUNCTION()
		float GetCurrentStaminaValue();

	UFUNCTION()
		float GetMaxStaminaValue();

protected:

	/************************************************************************/
	/*                                    内部调用                                  */
	/************************************************************************/
	UFUNCTION()
		void ReduceStaminaValue(float ReduceValue);

	UFUNCTION()
		void OnStaminaValueZero();

	UFUNCTION()
		void ReviveStaminaValue();

protected:
	
	UPROPERTY()
		float MaxStaminaValue;

	UPROPERTY()
		float CurrentStaminaValue;

	UPROPERTY()
		bool IsStaminaZero;

	// 从损失精力->精力恢复的状态切换时间
	UPROPERTY()
		float StaminaReviveBlock;

	// 从力竭状态->精力恢复的状态切换时间
	UPROPERTY()
		float StaminaZeroReviveBlock;

	// 精力恢复定时器tick间隔
	UPROPERTY()
		float StaminaReviveTimerBlock;

	UPROPERTY()
		float StaminaReviveSingleValue;

	UPROPERTY()
		FTimerHandle StaminaReviveTimerHandle;

};
