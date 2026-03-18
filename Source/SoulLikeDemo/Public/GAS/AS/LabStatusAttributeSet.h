// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AttributeSet.h"
#include "AttributeSet.h"
#include "LabAbilitySystemComponent.h"
#include "LabStatusAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
        GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)



UCLASS()
class SOULLIKEDEMO_API ULabStatusAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

	// 宏定义
	ATTRIBUTE_ACCESSORS(ULabStatusAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(ULabStatusAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(ULabStatusAttributeSet, Damage);

public:
	// Sets default values for this AttributeSet's properties
    ULabStatusAttributeSet();

	
	/************************************************************************/
	/*                               外部函数                                       */
	/************************************************************************/
	// 设置属性持有人
	void SetOwningActor(AActor* pOwnActor);
	// 玩家状态属性初始化
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "StatusAttributeSet")
	void InitStatusAS();
	virtual void InitStatusAS_Implementation();
	// 玩家状态属性-血量信息初始化
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "StatusAttributeSet")
		void InitHealthAS(float MinValue, float MaxValue);
	virtual void InitHealthAS_Implementation(float MinValue, float MaxValue);;

	

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

private:
	/************************************************************************/
	/*                               内部变量                                      */
	/************************************************************************/
	UPROPERTY()
	AActor* OwningActor;

};