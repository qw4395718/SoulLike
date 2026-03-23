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
#include "SL_CharacterBase.generated.h"

// 声明一个自定义日志类别
DECLARE_LOG_CATEGORY_EXTERN(SL_CharacterBase, Log, All);

class USL_AbilitySystemComponent;
class USL_StatusAttributeSet;
class UGameplayAbility;
class UWidgetComponent;

UCLASS()
class SOULLIKEDEMO_API ASL_CharacterBase : public ACharacter ,public ICharacterComponent_IF, public IAnimNotify_IF, public IAbilitySystemInterface
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

	UFUNCTION()
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

	/************************************************************************/
	/*                               接口实现(IAnimNotify_IF)                                       */
	/************************************************************************/

	// 动画(状态)通知响应
	UFUNCTION()
		void AnimNotifyResponse(int NotifyType) override;


protected:
	/************************************************************************/
	/*                                内部调用                                      */
	/************************************************************************/
	// 左键响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformAttack();

	// 右键响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformDefence();

	// 左Ctrl响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformComboSkill();

	// 锁定响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformLockRotation();

	// 翻滚响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformRoll();

	// 切换上方道具栏响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformSwitchEquipmentUp();

	// 切换上方道具栏响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformSwitchEquipmentDown();

	// 切换左方武器栏响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformSwitchEquipmentLeft();

	// 切换右方武器栏响应
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void PerformSwitchEquipmentRight();

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


	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 在蓝图中可调用的赋予能力函数
	UFUNCTION(BlueprintCallable, Category = "GAS")
		void GiveAbilityToSelf(TSubclassOf<UGameplayAbility> AbilityClass);

	UFUNCTION(BlueprintCallable, Category = "GAS")
		void GiveAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 InLevel, int32 InInputID);
	
	UFUNCTION(BlueprintCallable, Category = "GAS")
		FGameplayAbilitySpecHandle GiveAbilityAndActivateOnce(TSubclassOf<UGameplayAbility> AbilityClass, int32 InLevel, int32 InInputID);
	
	UFUNCTION(BlueprintCallable, Category = "GAS")
		void GiveAbilitiesToSelf(const TArray<TSubclassOf<UGameplayAbility>>& AbilityClasses);

protected:
	/************************************************************************/
	/*                                变量                                      */
	/************************************************************************/
	// GAS能力系统组件
	UPROPERTY()
		USL_AbilitySystemComponent* LabAbilitySystemComp;

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

	UPROPERTY()
		USL_StaminaComponent* StaminaCmp;

	UPROPERTY()
		USL_StateComponent* StateCmp;

	UPROPERTY()
		USL_MovementComponent* MovementCmp;

	UPROPERTY()
		UWidgetComponent* ScreenWidgetCmp;
};
