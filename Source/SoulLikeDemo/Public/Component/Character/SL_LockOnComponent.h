// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "SL_LockOnComponent.generated.h"

class ASL_EnemyBase;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULLIKEDEMO_API USL_LockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USL_LockOnComponent();

	/************************************************************************/
	/*                                   外部调用                                    */
	/************************************************************************/

	/** 初始化锁定组件 */
	UFUNCTION()
		void InitializeLockOnComponent();

	/** 尝试索敌 —— 已锁定则释放，未锁定则寻找最近敌人 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
		void TryLockOnTarget();

	/** 主动释放锁定 */
	UFUNCTION(BlueprintCallable, Category = "LockOn")
		void ReleaseLockOn();

	/** 是否正在锁定中 */
	UFUNCTION(BlueprintPure, Category = "LockOn")
		bool IsLocked() const { return bIsLocked && LockedTarget.IsValid(); }

	/** 获取当前锁定目标 */
	UFUNCTION(BlueprintPure, Category = "LockOn")
		AActor* GetLockedTarget() const { return LockedTarget.Get(); }

	/** 获取锁定目标相对于本体的水平方向（用于旋转控制） */
	UFUNCTION(BlueprintPure, Category = "LockOn")
		FVector GetLockDirection() const;

protected:
	/************************************************************************/
	/*                                   内部调用                                    */
	/************************************************************************/

	/** 在扇形区域内搜索最近的存活敌人 */
	ASL_EnemyBase* FindNearestEnemyInRange() const;

	/** 校验锁定条件是否仍然成立 */
	bool ValidateLockCondition() const;

	/** 显示/隐藏锁定UI指示器 */
	void UpdateLockOnIndicator(bool bShow);

	/** 锁定目标死亡回调 */
	UFUNCTION()
		void OnLockedTargetDied();

	/** 绑定/解绑目标死亡事件 */
	void BindTargetDeathEvent(ASL_EnemyBase* InTarget);
	void UnbindTargetDeathEvent();

	/** 启动/停止锁定状态定时校验 */
	void StartValidationTimer();
	void StopValidationTimer();

	/** 定时器回调：校验锁定条件 */
	UFUNCTION()
		void OnValidationTimerTick();

protected:
	/************************************************************************/
	/*                                   内部访问                                    */
	/************************************************************************/

	// ===== 可配置参数 =====

	/** 索敌最大距离 */
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Config", meta = (ClampMin = "100.0"))
		float LockOnRange = 1500.0f;

	/** 索敌扇形半角（度），0=正前方，90=180度范围 */
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Config", meta = (ClampMin = "0.0", ClampMax = "180.0"))
		float LockOnAngle = 45.0f;

	/** 释放锁定的逃逸距离系数（锁定距离 * 此系数 = 释放距离，防止边缘抖动） */
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Config", meta = (ClampMin = "1.0", ClampMax = "3.0"))
		float EscapeRangeMultiplier = 1.5f;

	/** 锁定条件校验间隔（秒） */
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Config", meta = (ClampMin = "0.05", ClampMax = "1.0"))
		float ValidationInterval = 0.25f;

	/** 锁定指示器UI的Widget类型（需先在UIManager中注册） */
	UPROPERTY(EditDefaultsOnly, Category = "LockOn|Config")
		EWidgetType LockOnIndicatorWidgetType = EWidgetType::EWIDGET_LockOnIndicator;

	// ===== 运行时状态 =====

	/** 当前锁定的目标 */
	UPROPERTY()
		TWeakObjectPtr<AActor> LockedTarget;

	/** 是否锁定中 */
	UPROPERTY()
		uint8 bIsLocked : 1;

	/** 定时器句柄 */
	FTimerHandle ValidationTimerHandle;

	/** 目标死亡委托绑定句柄 */
	FDelegateHandle LockedTargetDiedHandle;
};
