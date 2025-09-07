// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "CharacterInfo_IF.h"
#include "StateCalculate_IF.h"
#include "SL_StateComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_StateComponent : public UActorComponent, public IStateCalculate_IF
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_StateComponent();

public:
/************************************************************************/
/*                            接口实现                                          */
/************************************************************************/
	// 造成伤害量结算
	UFUNCTION()
		float DamageDealtCalculate(float Damage)override;

	// 受成伤害量结算
	UFUNCTION()
		float DamageReceivedCalculate(float Damage)override;

	// 消耗体力量结算
	UFUNCTION()
		float StaminaCostCalculate(float StaminaCost)override;

public:
/************************************************************************/
/*                              外部调用                                        */
/************************************************************************/



protected:
/************************************************************************/
/*                             内部变量                                        */
/************************************************************************/
	// 伤害增加百分比
	UPROPERTY()
		float DamageIncreasePercentage;

	// 减伤百分比
	UPROPERTY()
		float DamageReducePercentage;

	// 角色防御力
	UPROPERTY()
		int DefenceValue;

	// 体力消耗系数
	UPROPERTY()
		float StaminaCostReducePercentage;
};
