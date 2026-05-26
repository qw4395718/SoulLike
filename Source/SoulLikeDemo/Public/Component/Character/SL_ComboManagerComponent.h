// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "SL_ComboManagerComponent.generated.h"

class UAbilitySystemComponent;
class USL_StaminaComponent;
class UAbilityTask_ComboMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChargeLevelChanged, float, ChargeLevel);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SOULLIKEDEMO_API USL_ComboManagerComponent : public UActorComponent 
{
	GENERATED_BODY()

public:	
	USL_ComboManagerComponent();

public:
	/************************************************************************/
	/*                              接口实现                                        */
	/************************************************************************/
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	/************************************************************************/
	/*                              外部调用                                        */
	/************************************************************************/
	// 委托-响应输入
	UFUNCTION()
	void HandleInputPressed(EComboInputActionType InputType);

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageFinished();

	// 注册当前激活的Combo Task
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void RegisterActiveComboTask(UAbilityTask_ComboMontage* InTask);

	// 注销Combo Task
	UFUNCTION(BlueprintCallable, Category = "Combo")
	void UnregisterActiveComboTask();

	// 设置窗口标签
	UFUNCTION()
	void SetNeedClearTag(FGameplayTag WindowTag);

	// 能力成功激活后清理上一轮的标签
	UFUNCTION()
	void ClearTargetWindowTag();

	// 获取当前连击伤害倍率
	UFUNCTION(BlueprintCallable, Category = "Combo")
	float GetCurrentComboDamageMultiplier() const { return CurrentComboInfo.DamageMultiplier; }

	// 获取当前连击体力消耗
	UFUNCTION(BlueprintCallable, Category = "Combo")
	float GetCurrentComboStaminaCost() const { return CurrentComboInfo.StaminaCost; }

	// 获取完整的当前连击信息
	UFUNCTION(BlueprintCallable, Category = "Combo")
	const FComboInfo& GetCurrentComboInfo() const { return CurrentComboInfo; }

	/************************************************************************/
	/*                              蓄力系统                                        */
	/************************************************************************/
	// 是否正在蓄力
	UFUNCTION(BlueprintPure, Category = "Charge")
	bool IsCharging() const { return m_bIsCharging; }

	// 获取当前蓄力进度（0.0 ~ 1.0）
	UFUNCTION(BlueprintPure, Category = "Charge")
	float GetChargeLevel() const { return m_chargeLevel; }

	// 蓄力进度变化委托（UI 监听）
	UPROPERTY(BlueprintAssignable, Category = "Charge")
	FOnChargeLevelChanged OnChargeLevelChanged;

	// 外部通知蓄力释放（由 Controller 的 Released 回调调用）
	UFUNCTION()
	void ReleaseCharge();

protected:
	/************************************************************************/
	/*                              内部调用                                        */
	/************************************************************************/
	UAbilitySystemComponent* GetCachedASC() const;

	USL_StaminaComponent* GetCachedStaminaComp() const;

	void StartCharge(EComboInputActionType InChargeInputType, const FComboInfo& InComboInfo);

	void UpdateCharge(float DeltaTime);

protected:
	/************************************************************************/
	/*                              内部变量                                        */
	/************************************************************************/
	// 当前激活的ComboTask
	TWeakObjectPtr<UAbilityTask_ComboMontage> ActiveComboTask;
	// ASC的缓存
	mutable TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	// ASC的缓存
	mutable TWeakObjectPtr<USL_StaminaComponent> CachedStaminaComp;
	// 当前招式的缓存
	FComboInfo CurrentComboInfo;
	// 下一招的缓存
	FComboInfo nextComboInfo;
	// 上一招的窗口标签(如果是混合过度,则需要手动清理标签)
	FGameplayTag oldWindowTag;

	/************************************************************************/
	/*                              蓄力变量                                        */
	/************************************************************************/
	bool m_bIsCharging;

	float m_chargeStartTime;

	float m_chargeLevel;

	// 蓄力对应的连招数据
	FComboInfo m_chargeComboInfo;

	// 蓄力开始时是否已激活占位 GA
	bool m_bChargeHoldAbilityActivated;
};
