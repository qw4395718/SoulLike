// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemComponent.h"
#include "SL_AbilitySystemComponent.generated.h"

UCLASS()
class SOULLIKEDEMO_API USL_AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	USL_AbilitySystemComponent();

public:

	UFUNCTION(BlueprintCallable, Category = "GAS")
		FGameplayAbilitySpecHandle GiveAbilityForBP(TSubclassOf<UGameplayAbility> AbilityClass, int32 InLevel, int32 InInputID, UObject* SourceObject);

	UFUNCTION(BlueprintCallable, Category = "GAS")
		FGameplayAbilitySpecHandle GiveAbilityAndActivateOnceForBP(TSubclassOf<UGameplayAbility> AbilityClass, int32 InLevel, int32 InInputID, UObject* SourceObject);

	UFUNCTION(BlueprintCallable, Category = "GAS")
		TArray<FGameplayAbilitySpec>& GetActivatableAbilitiesForBP();

	UFUNCTION(BlueprintCallable, Category = "GAS")
		bool TryActivateAbilityByHandle(FGameplayAbilitySpecHandle Handle);

	UFUNCTION(BlueprintCallable, Category = "GAS")
		void ClearAbilityByHandle(FGameplayAbilitySpecHandle Handle);

	UFUNCTION()
		TArray<UGameplayTask*> GetCurrentlyActiveTasks();

		 /** 通过Tag激活能力（供BTTask使用） */
    UFUNCTION(BlueprintCallable, Category = "GAS")
        bool TryActivateAbilityByTag(const FGameplayTag& AbilityTag);

    /** 通过Tag获取能力SpecHandle（供BTTask监听委托） */
    UFUNCTION(BlueprintCallable, Category = "GAS")
        FGameplayAbilitySpecHandle FindAbilitySpecHandleByTag(const FGameplayTag& AbilityTag) const;

    /** 获取能力实例（供BTTask绑定委托） */
    UFUNCTION(BlueprintCallable, Category = "GAS")
        UGameplayAbility* GetActiveAbilityInstanceByTag(const FGameplayTag& AbilityTag) const;

	FGameplayAbilitySpec* FindAbilitySpecFromTag(const FGameplayTag& AbilityTag) const;

	// 设置状态Tag为“存活”
	UFUNCTION(BlueprintCallable, Category = "GAS")
	void SetAliveTag();
};