// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "Stamina_IF.h"
#include <GameplayEffectTypes.h>
#include "GameplayEffect.h"
#include "SL_StaminaComponent.generated.h"

class UAbilitySystemComponent;

/** 体力恢复状态 */
UENUM(BlueprintType)
enum class EStaminaRegenState : uint8
{
	Regenerating,		// 正在恢复
	Paused,				// 暂停恢复（连击中）
	Delayed				// 延迟恢复（体力清空后的间隔）
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_StaminaComponent : public UActorComponent ,public IStamina_IF
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_StaminaComponent();

public:
	/************************************************************************/
	/*                                    接口实现                                  */
	/************************************************************************/

	/************************************************************************/
	/*                                    外部调用                                  */
	/************************************************************************/

	UFUNCTION()
		void InitializeStaminaComponent();

	/** 是否有足够体力进行本次连击 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
		bool CanAffordCost(float CostAmount) const;

	/** 是否为透支攻击（体力>0但<消耗值） */
	UFUNCTION(BlueprintPure, Category = "Stamina")
		bool IsOverdrawAttack(float CostAmount) const;

	/** 获取当前体力值 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
		float GetCurrentStamina() const;

	/** 获取最大体力值 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
		float GetMaxStamina() const;

	/** 获取恢复状态 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
		EStaminaRegenState GetRegenState() const { return CurrentRegenState; }

	/** 获取体力百分比（0-1） */
	UFUNCTION(BlueprintPure, Category = "Stamina")
		float GetStaminaPercentage() const;

	UFUNCTION()
		void ConsumeStamina(float InAmount);

	// ===== 连击生命周期事件 =====
	/** 连击开始时调用 - 暂停恢复 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
		void OnComboStarted();

	/** 连击结束/中断时调用 - 恢复恢复或延迟 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
		void OnComboEnded();

	// ===== 配置接口 =====
	/** 注册恢复GE */
	UFUNCTION(BlueprintCallable, Category = "Stamina|Config")
		void SetRegenEffect(TSubclassOf<UGameplayEffect> InEffectClass);

	/** 注册延迟GE */
	UFUNCTION(BlueprintCallable, Category = "Stamina|Config")
		void SetDelayEffect(TSubclassOf<UGameplayEffect> InEffectClass);

	/** 注册延迟GE */
	UFUNCTION(BlueprintCallable, Category = "Stamina|Config")
		void SetCostEffect(TSubclassOf<UGameplayEffect> InEffectClass);

protected:

	/************************************************************************/
	/*                                    内部调用                                  */
	/************************************************************************/
	/** 激活恢复GE */
	void ActivateRegen();

	/** 移除恢复GE */
	void DeactivateRegen();

	/** 开始延迟恢复 */
	void StartDelayRecovery();

	/** 延迟GE移除回调 */
	UFUNCTION()
		void OnDelayEffectRemoved(const FActiveGameplayEffect& Effect);

	/** 设置恢复状态 */
	void SetRegenState(EStaminaRegenState NewState);

	/** 创建体力消耗GE（Instant） */
	UGameplayEffect* CreateCostEffect();

	/** 创建体力恢复GE（Infinite + Periodic） */
	UGameplayEffect* CreateRegenEffect();

	/** 创建延迟恢复GE（Duration） */
	UGameplayEffect* CreateDelayEffect();

	UAbilitySystemComponent* GetCacheASC() const;
protected:
	
	// ASC的缓存
	mutable TWeakObjectPtr<UAbilitySystemComponent> CachedASC;

	// ===== GameplayEffect句柄 =====
/** 恢复GE的ActiveHandle */
	FActiveGameplayEffectHandle RegenEffectHandle;

	/** 延迟GE的ActiveHandle */
	FActiveGameplayEffectHandle DelayEffectHandle;

	// ===== GE Class =====
	/** 立即消耗体力的GE */
	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Config")
		TSubclassOf<UGameplayEffect> CostEffectClass;

	/** 周期性恢复体力的GE（无限持续） */
	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Config")
		TSubclassOf<UGameplayEffect> RegenEffectClass;

	/** 体力清空后的延迟GE（Duration类型） */
	UPROPERTY(EditDefaultsOnly, Category = "Stamina|Config")
		TSubclassOf<UGameplayEffect> DelayEffectClass;

	// ===== 状态 =====
	/** 当前恢复状态 */
	EStaminaRegenState CurrentRegenState;

	/** 透支标记 */
	bool bWasOverdraw;

};
