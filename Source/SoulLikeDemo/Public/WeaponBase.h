// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SoulLikeGameGlobal.h"
#include "SoulLikeCharacter.h"
#include "Components/BoxComponent.h"
#include "SL_Attack_JumpSection_NS.h"
#include "WeaponBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SOULLIKEDEMO_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:

	AWeaponBase();

protected:
	/************************************************************************/
	/*                               继承实现                                       */
	/************************************************************************/
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// 调试初始化,简易流程
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual void Initialize();

	// 获取伤害数据（含类魂特性计算）
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
		FDamageData GetDamageData() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
		void PerformAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
		void PerformCombatSkill();

	UFUNCTION(BlueprintCallable, Category = "Combat")
		void PerformBackstab();

	UFUNCTION(BlueprintCallable, Category = "Combat")
		void PerformExecute();

	// 碰撞检测回调
	UFUNCTION()
		virtual void OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, FVector NormalImpulse,
			const FHitResult& Hit);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
		virtual float GetStaminaCost(EAttackType AttackType);

	// 外部调用-用以开启攻击碰撞盒检测
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void EnableAttackCollisonCheck();

	// 外部调用-用以关闭攻击碰撞盒检测
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void DisableAttackCollisonCheck();

	// 外部调用-用以开启弹反碰撞盒检测
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void EnableParryCollisonCheck();

	// 外部调用-用以关闭攻击碰撞盒检测
	UFUNCTION(BlueprintCallable, Category = "Weapon")
		void DisableParryCollisonCheck();

	// 外部调用-用以激活弹反窗口
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_Parry")
		void ActivateParryWindow(float Duration);

	// 外部调用-用以关闭弹反窗口
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_Parry")
		void DeactivateParryWindow();

	// 外部调用-获取当前是否处于弹反窗口激活状态
	UFUNCTION(BlueprintCallable, Category = "CombatComponent_Parry")
		bool IsParryWindowActive();

	void SetComboContinueState(bool Enable);

	void SetJumpSection_NS(USL_Attack_JumpSection_NS* NS);

protected:
	// 武器动作接口
	UFUNCTION(BlueprintCallable, Category = "Animation")
		void PlayAttackMontage(FName MontageSectionName);

	// 武器战技动作接口
	UFUNCTION(BlueprintCallable, Category = "Animation")
		void PlayCombatSkillMontage(FName MontageSectionName);

	// 攻击重叠开始
	UFUNCTION()
	void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);
	
	// 攻击重叠结束
	UFUNCTION()
	void OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 弹反重叠开始
	UFUNCTION()
		void OnParryOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);

	// 弹反重叠结束
	UFUNCTION()
		void OnParryOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	// 处理攻击重叠actor
	UFUNCTION()
	void ApplyDamageToOverlappingActors();

	// 处理弹反重叠actor
	UFUNCTION()
		void ApplyParryToOverlappingActors();


public:
	//状态
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
		bool IsStaticMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
		bool bEnableCapsuleCheck;
	// 组件
	UPROPERTY(EditDefaultsOnly, Category = "Components")
		USkeletalMeshComponent* SkeletalWeaponMesh;

	// 组件
	UPROPERTY(EditDefaultsOnly, Category = "Components")
		UStaticMeshComponent* StaticWeaponMesh;

	// 胶囊体-椭球形状
	UPROPERTY(VisibleAnywhere, Category = "Components")
		UCapsuleComponent* CapsuleComp;

	// 碰撞盒-规则多边形
	UPROPERTY(VisibleAnywhere, Category = "Components")
		UBoxComponent* CollisonBox;

	// 武器属性
	UPROPERTY(EditDefaultsOnly, Category = "Stats")
		FWeaponStats WeaponData;

	// 动画资源-攻击
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* AttackMontage;

	// 动画资源-战技
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* CombatSkillMontage;

	// 动画资源-处决
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* ExecutionMontage;

	// 动画资源-被刺
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
		UAnimMontage* BackStabbMontage;

	// 动画状态通知引用
	UPROPERTY(VisibleAnywhere, Category = "Animation")
		USL_Attack_JumpSection_NS* AttackSection_NS;

	// 是否可以连击
	UPROPERTY(VisibleAnywhere, Category = "Animation")
		bool EnableComboContinue;

	// 持有者引用
	UPROPERTY(Transient)
		ASoulLikeCharacter* OwningCharacter;
	
protected:
	// 伤害定时器handle
	UPROPERTY()
	FTimerHandle DamageTimerHandle;

	// 定时器间隔
	UPROPERTY()
	float DamageInterval = 0.1f;

	// 伤害碰撞命中actor数组
	UPROPERTY()
	TArray<AActor*> AttackOverlappingActors;

	// 弹反碰撞命中actor数组,仅需找到第一个成功即可清空
	UPROPERTY()
	TArray<AActor*> ParryOverlappingActors;

	// 已应用伤害actor集
	UPROPERTY()
	TSet<AActor*> AlreadyHitActors;

	// 已应用伤害actor集
	UPROPERTY()
		TSet<AActor*> AlreadyParryActors;

	// 弹反窗口是否激活
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterAttribute")
		bool bIsParryWindowActive;

	// 弹反定时器handle
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CharacterAttribute")
		FTimerHandle ParryWindowTimer;

	// 定时器间隔
	UPROPERTY()
		float ParryInterval = 0.1f;

	// AP消耗参数
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponSettings")
		TMap<EAttackType,float> APCostMap;
};
