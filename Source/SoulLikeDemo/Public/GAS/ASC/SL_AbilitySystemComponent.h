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
		TArray<FGameplayAbilitySpec>& GetActivatableAbilitiesForBP()
	{
		return ActivatableAbilities.Items;
	}

};