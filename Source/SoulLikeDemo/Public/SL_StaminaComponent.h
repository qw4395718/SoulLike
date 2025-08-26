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
		void ReviveStaminaValue(float ReviveValue);

protected:
	
	UPROPERTY()
		float MaxStaminaValue;

	UPROPERTY()
		float CurrentStaminaValue;

	UPROPERTY()
		bool IsStaminaZero;

	UPROPERTY()
		float StaminaReviveTimeBlock;

	UPROPERTY()
		float StaminaReviveSingleValue;

	UPROPERTY()
		FTimerHandle StaminaReviveTimerHandle;

	UPROPERTY()
		FTimerHandle StaminaReviveEnableTimerHandle;
};
