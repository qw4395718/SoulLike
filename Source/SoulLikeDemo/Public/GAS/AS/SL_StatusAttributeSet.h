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

	// �궨��
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, Health);
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, MaxHealth);
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, Damage);
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, Stamina);
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, MaxStamina);
	ATTRIBUTE_ACCESSORS(USL_StatusAttributeSet, StaminaCost);

public:
	// Sets default values for this AttributeSet's properties
    USL_StatusAttributeSet();

	
	/************************************************************************/
	/*                               �ⲿ����                                       */
	/************************************************************************/
	// �������Գ�����
	void SetOwningActor(AActor* pOwnActor);
	// ���״̬���Գ�ʼ��

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "StatusAttributeSet")
	void InitStatusAS();
	virtual void InitStatusAS_Implementation();

	// ���״̬����-Ѫ����Ϣ��ʼ��
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "StatusAttributeSet")
		void InitHealthAS(float MinValue, float MaxValue);
	virtual void InitHealthAS_Implementation(float MinValue, float MaxValue);

	// ���״̬����-������Ϣ��ʼ��
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "StatusAttributeSet")
		void InitStaminaAS(float MinValue, float MaxValue);
	virtual void InitStaminaAS_Implementation(float MinValue, float MaxValue);

	UFUNCTION()
		void OnRep_CurrentHealth();

	UFUNCTION()
		void OnRep_CurrentStamina();
	
protected:
	/************************************************************************/
	/*                               �̳�ʵ��                                       */
	/************************************************************************/
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	void OnCharacterDeath();
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
public:
	/************************************************************************/
	/*                               �ⲿ����-״̬����(��ս���б仯������)                                       */
	/************************************************************************/
	// ״̬����
    // ��ǰ����ֵ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "Health")
    FGameplayAttributeData Health;

    // �������ֵ-������Ҫ�Ƶ��μ�������ȥ
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Health")
    FGameplayAttributeData MaxHealth;

	// Meta����: �ݴ淢���߽���Ĺ����˺�
	UPROPERTY(VisibleAnywhere, Category = "Health")
		FGameplayAttributeData Damage;

	// ��ǰ����ֵ 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentStamina, Category = "Stamina")
		FGameplayAttributeData Stamina;

	// �������ֵ 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Stamina")
		FGameplayAttributeData MaxStamina;

	// Meta����: �ݴ�����ĵ�����ֵ 
	UPROPERTY(VisibleAnywhere, Category = "Stamina")
		FGameplayAttributeData StaminaCost;



private:
	/************************************************************************/
	/*                               �ڲ�����                                      */
	/************************************************************************/
	UPROPERTY()
	AActor* OwningActor;

};