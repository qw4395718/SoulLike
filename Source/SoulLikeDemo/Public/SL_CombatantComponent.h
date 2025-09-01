// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "Combat_IF.h"
#include "SL_CombatantComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_CombatantComponent : public UActorComponent ,public ICombat_IF
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_CombatantComponent();

public:
	/************************************************************************/
	/*                              接口实现                                        */
	/************************************************************************/
	UFUNCTION()
		void TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
		int GetCurrentCombatState() override;

	UFUNCTION()
		int GetTeamID() override;

	UFUNCTION()
		void OnAttackEventCall() override;

	UFUNCTION()
		bool CanExecute() override;

	UFUNCTION()
		bool CanBackStabs() override;

	UFUNCTION()
		bool PerformExecuted(FName WeaponName) override;

	UFUNCTION()
		bool PerformBackStabbed(FName WeaponName) override;

public:
	/************************************************************************/
	/*                              外部调用                                        */
	/************************************************************************/
	// 外部初始化
	UFUNCTION()
		void InitCombatComponentInfo();

protected:
	/************************************************************************/
	/*                              内部调用                                        */
	/************************************************************************/
	// 异步加载蒙太奇动画
	void LoadActorMentageAsync(const FString MentagePath);

protected:
	/************************************************************************/
	/*                              内部变量                                        */
	/************************************************************************/
	// 所属阵营
	UPROPERTY()
		int TeamID;

	// 是否待处决
	UPROPERTY()
		bool bWaitingForExecuted;

	// 是否可背刺
	UPROPERTY()
		bool bAllowedBackStabsed;

	// 处决蒙太奇动画资产(后续可以归并到动画组件中，统一管理)
	UPROPERTY()
		TSoftObjectPtr<UAnimMontage> SoftExecuteMentageRefrence;
	
	// 背刺蒙太奇动画资产(后续可以归并到动画组件中，统一管理)
	UPROPERTY()
		TSoftObjectPtr<UAnimMontage> SoftBackStabsMentageRefrence;

	
};
