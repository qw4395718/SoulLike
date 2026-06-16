// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "DamageEventDispatcher.h"
#include "CombatComponent.generated.h"

class ASL_WeaponBase;
class ASL_CharacterBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

protected:
	/************************************************************************/
	/*                              内部重载                                        */
	/************************************************************************/
	virtual void PostInitProperties() override;

	virtual void OnRegister() override;

	virtual void OnUnregister() override;

public:
	/************************************************************************/
	/*外部初始化                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_Init")
		virtual void InitializeComponent() override;
	// 调试初始化
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_Init")
		virtual void Initialize();
	//提供给外部初始化武器背包
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_WeaponInventoryManage")
		void InitWeaponInventory(TArray<ASL_WeaponBase*> arrWeaponInventory);
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_WeaponInventoryManage")
		void WeaponInventoryChange(int32 Weaponindex, ASL_WeaponBase* NewWeapon);

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 拔出当前装备武器
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_WeaponOperation")
		void DrawWeapon();
	// 收起武器
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_WeaponOperation")
		void SheathWeapon();
	// 武器切换
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_WeaponOperation")
		void SwitchToWeapon(int32 Index);
	// 执行攻击
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_WeaponOperation")
		void PerformAttack();
	// 执行战技
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_WeaponOperation")
		void PerformCombatSkill();


	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

	// 精准盾反判定（仅C++可用）
	bool CheckPerfectParry(float PlayerInputTime, float EnemyAttackTime);

	// 暴露给蓝图的伤害应用接口
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ProcessAttackHit(AActor* HitActor, const FHitResult& HitResult);

	UFUNCTION(BlueprintCallable, Category = "Combat")
		void HandleDamage(const FDamageEventData& DamageEvent);

	// 弹反行为应用(内部判定是否有效及后续行为)
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void HandleParry();

	// 当前体力是否可以执行动作
	UFUNCTION(BlueprintCallable, Category = "Combat")
		bool CanAction();

	// 消耗体力
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void ChangeAP(float CostNum);

	// 开启体力恢复
	UFUNCTION(BlueprintCallable, Category = "Combat")
		void ReviveAP();

protected:

	// 抗性数据（C++计算）
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		TMap<EDamageType, float> DamageResistances;

	// 角色属性-HP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterAttribute")
		float HealthPoint;

	// 角色属性-AP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterAttribute")
		float ActionPoint;

	// 角色属性-HP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterAttribute")
		float HealthPointMaxValue;

	// 角色属性-AP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterAttribute")
		float ActionPointMaxValue;

	// 体力恢复定时器handle
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterAttribute")
		FTimerHandle ReviveActionPointHandle;

	// 体力恢复定时器间隔
	UPROPERTY()
		float TriggerReviveAPTimerInterval = 0.1f;

	// 体力恢复定时器开启间隔
	UPROPERTY()
	float EnableReviveAPTimerInterval = 1.0f;

	// 单次触发恢复量
	UPROPERTY()
		float APReviveValue = 1.0f;

protected:
	//伤害事件分发器
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UDamageEventDispatcher* DamageDispatcher;

public:
	// 当前右手装备武器
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		ASL_WeaponBase* LH_EquippedWeapon;

	// 当前右手装备武器
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		ASL_WeaponBase* RH_EquippedWeapon;

	// 武器库存
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		TArray<ASL_WeaponBase*> WeaponInventory;

	// 组件引用
	UPROPERTY()
		ASL_CharacterBase* CharacterOwner;

};
