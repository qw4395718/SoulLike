// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "SL_ComboManagerComponent.generated.h"

class UAbilitySystemComponent;
class USL_StaminaComponent;
class UAbilityTask_ComboMontage;
class UComboInfoTable;

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
	UFUNCTION()
	void HandleInputPressed(EComboInputActionType InputType);

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageFinished();

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void RegisterActiveComboTask(UAbilityTask_ComboMontage* InTask);

	UFUNCTION(BlueprintCallable, Category = "Combo")
	void UnregisterActiveComboTask();

	UFUNCTION()
	void SetNeedClearTag(FGameplayTag WindowTag);

	UFUNCTION()
	void ClearTargetWindowTag();

	UFUNCTION(BlueprintCallable, Category = "Combo")
	float GetCurrentComboDamageMultiplier() const { return CurrentComboInfo.DamageMultiplier; }

	UFUNCTION(BlueprintCallable, Category = "Combo")
	float GetCurrentComboStaminaCost() const { return CurrentComboInfo.StaminaCost; }

	UFUNCTION(BlueprintCallable, Category = "Combo")
	const FComboInfo& GetCurrentComboInfo() const { return CurrentComboInfo; }

	class UComboInfoTable* GetCurrentWeaponComboTable() const;

	UFUNCTION(BlueprintCallable, Category = "Combo")
	UAnimMontage* ResolveCurrentMontage() const;

	/************************************************************************/
	/*                              蓄力系统                                        */
	/************************************************************************/
	UFUNCTION(BlueprintPure, Category = "Charge")
	bool IsCharging() const { return m_bIsCharging; }

	UFUNCTION(BlueprintPure, Category = "Charge")
	float GetChargeLevel() const { return m_chargeLevel; }

	UPROPERTY(BlueprintAssignable, Category = "Charge")
	FOnChargeLevelChanged OnChargeLevelChanged;

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
	TWeakObjectPtr<UAbilityTask_ComboMontage> ActiveComboTask;
	mutable TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
	mutable TWeakObjectPtr<USL_StaminaComponent> CachedStaminaComp;
	FComboInfo CurrentComboInfo;
	FComboInfo nextComboInfo;
	FGameplayTag oldWindowTag;

	/************************************************************************/
	/*                              蓄力变量                                        */
	/************************************************************************/
	bool m_bIsCharging;
	float m_chargeStartTime;
	float m_chargeLevel;
	FComboInfo m_chargeComboInfo;
	bool m_bChargeHoldAbilityActivated;
};
