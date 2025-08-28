// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "Health_IF.h"
#include "SL_HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_HealthComponent : public UActorComponent ,public IHealth_IF
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_HealthComponent();

public:

	/************************************************************************/
	/*                                    接口实现                                  */
	/************************************************************************/
	UFUNCTION()
		bool IsAlive() override;

	UFUNCTION()
		float GetCurrentHealthValue() override;

	UFUNCTION()
		float GetMaxHealthValue() override;

	UFUNCTION()
		void ReduceCurrentHealth(float ReduceValue) override;

	UFUNCTION()
		void ReviveCurrentHealth(float ReviveValue) override;

	/************************************************************************/
	/*                                    外部调用                                  */
	/************************************************************************/
	UFUNCTION()
		void InitHealthInfo();



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
