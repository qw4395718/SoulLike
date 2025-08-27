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
		bool InitMovemenetInfo();

	UFUNCTION()
		void ExeRoll();


protected:

/************************************************************************/
/*                                    内部调用                                  */
/************************************************************************/
	UFUNCTION()
		bool CanExeRoll();

	UFUNCTION()
		void LoadMovementMentageAsync(const FString MentagePath);

protected:
	// 特殊蒙太奇动画异步加载ptr
	UPROPERTY()
		TSoftClassPtr<UAnimMontage> SoftMentageRefrence;

};
