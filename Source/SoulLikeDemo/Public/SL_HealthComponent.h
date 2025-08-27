// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "SL_HealthComponent.generated.h"

class AWeaponBase;
class ASoulLikeCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_HealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_HealthComponent();

public:
	/************************************************************************/
	/*                                    外部调用                                  */
	/************************************************************************/
	UFUNCTION()
		void InitHealthInfo();

	UFUNCTION()
		bool IsAlive();

	UFUNCTION()
		float GetCurrentHealthValue();

	UFUNCTION()
		float GetMaxHealthValue();

	UFUNCTION()
		void ReduceCurrentHealth(float ReduceValue);

	UFUNCTION()
		void ReviveCurrentHealth(float ReviveValue);

protected:

	/************************************************************************/
	/*                                    内部调用                                  */
	/************************************************************************/
	UFUNCTION()
		void OnHealthEqualZero();


protected:
	
	UPROPERTY()
		float CurrentHealthValue;

	UPROPERTY()
		float MaxHealthValue;

	UPROPERTY()
		bool bIsAlive;

};
