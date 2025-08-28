// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "Stamina_IF.h"
#include "SL_StaminaComponent.generated.h"

class AWeaponBase;
class ASoulLikeCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_StaminaComponent : public UActorComponent ,public IStamina_IF
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_StaminaComponent();

public:

	/************************************************************************/
	/*                                    接口实现                                  */
	/************************************************************************/

	UFUNCTION()
		void ReduceStamina(float ReduceValue) override;

	UFUNCTION()
		void ReviveStamina(float ReviveValue) override;

	UFUNCTION()
		bool GetIsStaminaZero() override;

	UFUNCTION()
		float GetCurrentStaminaValue() override;

	UFUNCTION()
		float GetMaxStaminaValue() override;

	/************************************************************************/
	/*                                    外部调用                                  */
	/************************************************************************/

	UFUNCTION()
		void InitStaminaInfo();

protected:

	/************************************************************************/
	/*                                    内部调用                                  */
	/************************************************************************/

	UFUNCTION()
		void OnStaminaValueZero();

	UFUNCTION()
		void ReviveStaminaValueByTime();

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
