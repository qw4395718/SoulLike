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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttributeChangedEvent, float, OldValue, float, NewValue);


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
	/*                               外部变量-状态属性(在战斗中变化的属性)                                       */
	/************************************************************************/
	// 状态属性
    // 当前生命值
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, meta = (HideFromModifiers))
    FGameplayAttributeData Health;

    // 最大生命值-后面需要移到次级属性中去
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    FGameplayAttributeData MaxHealth;

	// 发起者结算的攻击伤害
	UPROPERTY(VisibleAnywhere)
	FGameplayAttributeData Damage;

	/************************************************************************/
	/*	                              属性变更委托                                       */
	/************************************************************************/
	UPROPERTY(BlueprintAssignable)
	FAttributeChangedEvent OnHealthChanged;
};