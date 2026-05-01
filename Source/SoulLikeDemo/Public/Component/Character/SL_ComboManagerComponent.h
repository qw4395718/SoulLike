// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "SL_ComboManagerComponent.generated.h"

class UAbilitySystemComponent;
class USL_StaminaComponent;
class UAbilityTask_ComboMontage;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_ComboManagerComponent : public UActorComponent 
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_ComboManagerComponent();

public:
	/************************************************************************/
	/*                              接口实现                                        */
	/************************************************************************/


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

protected:
	/************************************************************************/
	/*                              内部调用                                        */
	/************************************************************************/
	UAbilitySystemComponent* GetCachedASC() const;

	USL_StaminaComponent* GetCachedStaminaComp() const;

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

};
