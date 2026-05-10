// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "../Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AttributeSet.h"
#include "AttributeSet.h"
#include "SL_AbilitySystemComponent.h"
#include "SL_StatusAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
        GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
        GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)



UCLASS()
class SOULLIKEDEMO_API USL_StatusAttributeSet : public UAttributeSet
{
	GENERATED_BODY()


public:
	// Sets default values for this AttributeSet's properties
    USL_StatusAttributeSet();

	/** 血量属性 */
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, Damage);

	/** 耐力属性 */
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, Stamina);
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, MaxStamina);
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, StaminaCost);       // Meta: 暂存消耗量
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, StaminaRegen);      // Meta: 暂存恢复量

	/************************************************************************/
	/*                               外部调用                                      */
	/************************************************************************/
	void SetOwningActor(AActor* pOwnActor);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "StatusAttributeSet")
		void InitHealthAS(float MinValue, float MaxValue);
	virtual void InitHealthAS_Implementation(float MinValue, float MaxValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "StatusAttributeSet")
		void InitStaminaAS(float MinValue, float MaxValue);
	virtual void InitStaminaAS_Implementation(float MinValue, float MaxValue);

	UFUNCTION()
		void OnRep_CurrentHealth();

	UFUNCTION()
		void OnRep_CurrentStamina();
	
protected:
	/************************************************************************/
	/*                               内部调用                                       */
	/************************************************************************/
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	void OnCharacterDeath(AActor* DeathActor);
	void OnCharacterReLive(AActor* ReviveActor);
public:
	/************************************************************************/
	/*                              外部可访问                                       */
	/************************************************************************/
	/** 血量属性 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "Health")
    FGameplayAttributeData Health;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Health")
    FGameplayAttributeData MaxHealth;
	
	// Meta属性: 暂存伤害值 - 通过GE设置此值来触发消耗逻辑
	UPROPERTY(VisibleAnywhere, Category = "Health")
		FGameplayAttributeData Damage;
	
	/** 耐力属性 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentStamina, Category = "Stamina")
		FGameplayAttributeData Stamina;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Stamina")
		FGameplayAttributeData MaxStamina;

	// Meta属性: 暂存消耗值 - 通过GE设置此值来触发消耗逻辑
	UPROPERTY(VisibleAnywhere, Category = "Stamina")
		FGameplayAttributeData StaminaCost;

	// Meta属性: 暂存恢复值 - 通过GE设置此值来触发恢复逻辑（周期性恢复GE使用
	UPROPERTY(VisibleAnywhere, Category = "Stamina")
		FGameplayAttributeData StaminaRegen;



private:
	/************************************************************************/
	/*                              私有                                      */
	/************************************************************************/
	UPROPERTY()
	AActor* OwningActor;

};