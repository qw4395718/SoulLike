// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AttributeSet.h"
#include "AttributeSet.h"
#include "LabAbilitySystemComponent.h"
#include "LabHealthAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
        GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttributeChangedEvent, UAttributeSet*, AttributeSet, float, NewValue);


UCLASS()
class SOULLIKEDEMO_API ULabHealthAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

	// 宏定义
	ATTRIBUTE_ACCESSORS(ULabHealthAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(ULabHealthAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(ULabHealthAttributeSet, Damage);

public:
	// Sets default values for this AttributeSet's properties
    ULabHealthAttributeSet();

protected:
	/************************************************************************/
	/*                               继承实现                                       */
	/************************************************************************/
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;


public:
	/************************************************************************/
	/*                               外部变量                                       */
	/************************************************************************/
    // Current health
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, meta = (HideFromModifiers))
    FGameplayAttributeData Health;


    // Upper limit for health value
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    FGameplayAttributeData MaxHealth;

	// Damage value calculated during a GE. Meta attribute.
	UPROPERTY(VisibleAnywhere)
		FGameplayAttributeData Damage;


	UPROPERTY(BlueprintAssignable)
	FAttributeChangedEvent OnHealthChanged;
};