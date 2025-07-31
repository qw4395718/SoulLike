// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "Damageable.h"
#include "CombatComponent.generated.h"

class AWeaponBase;
class ASoulLikeCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API UCombatComponent : public UActorComponent, public IDamageable
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

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
		void InitWeaponInventory(TArray<AWeaponBase*> arrWeaponInventory);
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_WeaponInventoryManage")
		void WeaponInventoryChange(int32 Weaponindex, AWeaponBase* NewWeapon);

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
	// 实现IDamageable接口
	// 外部通过调用接口来触发伤害事件,再由伤害事件分发器发送到各个执行函数处
	virtual void ReceiveDamage_Implementation(const FDamageEventData& DamageEvent) override;
	virtual bool CanReceiveDamage_Implementation() const override;

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


protected:

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetupPlayerInput(UInputComponent* PlayerInputComponent);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartAttack();

	// 抗性数据（C++计算）
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
		TMap<EDamageType, float> DamageResistances;

protected:
	//伤害事件分发器
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UDamageEventDispatcher* DamageDispatcher;

public:

	// 当前装备武器
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
		AWeaponBase* EquippedWeapon;

	// 武器库存
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		TArray<AWeaponBase*> WeaponInventory;

	// 组件引用
	UPROPERTY()
		ASoulLikeCharacter* CharacterOwner;

	// 角色属性-HP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterAttribute")
		float HealthPoint;

	// 角色属性-AP
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterAttribute")
		float ActionPoint;
		
};
