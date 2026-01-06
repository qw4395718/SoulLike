// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "SL_MovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_MovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_MovementComponent();

public:
/************************************************************************/
/*                                    外部调用                                  */
/************************************************************************/
	UFUNCTION()
		void InitMovemenetInfo(bool hasRollAbility,FString RollMentagePath);

	UFUNCTION()
		void ExeRoll();


protected:

/************************************************************************/
/*                                    内部调用                                  */
/************************************************************************/
	UFUNCTION()
		bool CanExeRoll();

	UFUNCTION()
		void LoadMovementMentage(const FString MentagePath);

protected:
	// 翻滚蒙太奇
	UPROPERTY()
		UAnimMontage* DodgeMontage;

	// 是否可以跳跃
	UPROPERTY()
		bool CanRoll;
};
