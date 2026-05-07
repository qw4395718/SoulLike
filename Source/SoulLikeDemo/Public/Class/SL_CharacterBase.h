// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SL_CombatantComponent.h"
#include "SL_EquipmentComponent.h"
#include "SL_HealthComponent.h"
#include "SL_InventoryComponent.h"
#include "SL_StaminaComponent.h"
#include "SL_StateComponent.h"
#include "SL_MovementComponent.h"
#include "CharacterComponent_IF.h"
#include "WeaponAnimNotify_IF.h"
#include "AnimNotify_IF.h"
#include "../Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemInterface.h"
#include <WeaponAccessory_IF.h>
#include "SL_CharacterBase.generated.h"

// 声明一个自定义日志类别
DECLARE_LOG_CATEGORY_EXTERN(SL_CharacterBase, Log, All);

class USL_AbilitySystemComponent;
class USL_StatusAttributeSet;
class UGameplayAbility;
class UWidgetComponent;
class USL_ComboManagerComponent;

UCLASS()
class SOULLIKEDEMO_API ASL_CharacterBase : public ACharacter ,
public ICharacterComponent_IF, 
public IAnimNotify_IF, 
public IAbilitySystemInterface,
public IWeaponAccessory_IF 
{
	GENERATED_BODY()



public:
	// Sets default values for this character's properties
	ASL_CharacterBase(const FObjectInitializer& ObjectInitializer);

protected:
	/************************************************************************/
	/*                               继承实现                                       */
	/************************************************************************/
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/************************************************************************/
	/*                               接口实现(ICharacterComponent_IF)                                       */
	/************************************************************************/

	UFUNCTION()
		UActorComponent* GetCombatantComponent() override;

	UFUNCTION(BlueprintCallable)
		UActorComponent* GetEquipmentComponent() override;

	UFUNCTION()
		UActorComponent* GetHealthComponent() override;

	UFUNCTION()
		UActorComponent* GetInventoryComponent() override;

	UFUNCTION()
		UActorComponent* GetSpecialMovementComponent() override;

	UFUNCTION()
		UActorComponent* GetStaminaComponent() override;

	UFUNCTION()
		UActorComponent* GetStateComponent() override;

	UFUNCTION()
		UActorComponent* GetComboManagerComponent() override;

	/************************************************************************/
	/*                               接口实现(IAnimNotify_IF)                                       */
	/************************************************************************/

	// 动画(状态)通知响应
	UFUNCTION()
		void AnimNotifyResponse(int NotifyType) override;

	/************************************************************************/
    /*                    IWeaponAccessory_IF 接口实现                        */
    /************************************************************************/
	
    virtual ASL_WeaponBase* GetLeftHandWeapon() const override;
    virtual ASL_WeaponBase* GetRightHandWeapon() const override;
    virtual ASL_WeaponBase* GetWeaponByHand(int32 HandIndex) const override;

	/************************************************************************/
	/*                               Unlua相关                                      */
	/************************************************************************/

protected:
	/************************************************************************/
	/*                                内部调用                                      */
	/************************************************************************/

	// 角色通知转武器通知(EAnimNotifyType -> EWeaponAnimNotifyType)
	EWeaponAnimNotifyType TranslteAnimNotifyToWeapon(EAnimNotifyType NotifyType);

	// 根据角色通知确定是哪只手(EAnimNotifyType -> Hand(L/R))
	int CheckAnimNotifyToHand(EAnimNotifyType NotifyType);

	// 武器动画通知下发
	void WeaponAnimProcess(int HandType,EWeaponAnimNotifyType NotifyType);

	/************************************************************************/
	/*                                GAS委托处理                                      */
	/************************************************************************/


public:
	/************************************************************************/
	/*                               外部调用                                       */
	/************************************************************************/
	// 白盒初始化
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void InitializeCharacter();

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void InitPartmentComponent();

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void InitEquipmentWithClass(int32 InPlayerClassID);

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		bool IsAlive();

	FString GetNetworkGUIDString(AActor* InActor);
protected:
	/************************************************************************/
	/*                                变量                                      */
	/************************************************************************/
	// GAS能力系统组件
	UPROPERTY()
		USL_AbilitySystemComponent* AbilitySystemComp;

	UPROPERTY()
		USL_StatusAttributeSet* StatusAttributeSet;

	UPROPERTY()
		USL_StatusAttributeSet* PlayerStatusSet;

	// 先不设计组件管理器，直接创建组件使用
	// 战斗组件
	UPROPERTY()
		USL_CombatantComponent* CombatCmp;

	UPROPERTY()
		USL_EquipmentComponent* EquipmentCmp;

	UPROPERTY()
		USL_HealthComponent* HealthCmp;

	UPROPERTY()
		USL_InventoryComponent* InventoryCmp;

	UPROPERTY(BlueprintReadOnly)
		USL_StaminaComponent* StaminaCmp;

	UPROPERTY()
		USL_StateComponent* StateCmp;

	UPROPERTY()
		USL_MovementComponent* MovementCmp;

	UPROPERTY()
		UWidgetComponent* ScreenWidgetCmp;

	UPROPERTY()
		USL_ComboManagerComponent* ComboManagerCmp;
};
