// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SL_CombatantComponent.h"
#include "SL_EquipmentComponent.h"
#include "SL_HealthComponent.h"
#include "SL_InventoryComponent.h"
#include "SL_StaminaComponent.h"
#include "SL_LockOnComponent.h"
#include "SL_StateComponent.h"
#include "SL_MovementComponent.h"
#include "CharacterComponent_IF.h"
#include "WeaponAnimNotify_IF.h"
#include "AnimNotify_IF.h"
#include "../Plugins/Runtime/GameplayAbilities/Source/GameplayAbilities/Public/AbilitySystemInterface.h"
#include <WeaponAccessory_IF.h>
#include <ActorState_IF.h>
#include <OnlineSummonStructs.h>
#include <CombatEventDisplay_IF.h>
#include "SL_CharacterBase.generated.h"

// 声明一个自定义日志类别
DECLARE_LOG_CATEGORY_EXTERN(SL_CharacterBase, Log, All);

class USL_AbilitySystemComponent;
class USL_StatusAttributeSet;
class UGameplayAbility;
class UWidgetComponent;
class USL_ComboManagerComponent;
UENUM(BlueprintType)
enum class EPlayerState : uint8
{
    Alive   UMETA(DisplayName = "存活"),
    Dead    UMETA(DisplayName = "死亡")
};


UCLASS()
class SOULLIKEDEMO_API ASL_CharacterBase : public ACharacter ,
public ICharacterComponent_IF, 
public IAnimNotify_IF, 
public IAbilitySystemInterface,
public IWeaponAccessory_IF,
public IActorState_IF,
public ICombatEventDisplay_IF
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
	/*                    IActorState_IF 接口实现                        */
	/************************************************************************/
	virtual int32 GetTeamID() const override;
	virtual void SetTeamID(int32 InTeamID) override;

	virtual bool IsAlive() const override;
	virtual bool IsDie() const override;
	virtual void Destroyed() override;

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

	void Die();

	void RagDollStart();

	void Revive();

	void RagDollEnd();

	void ApplyEnemyConfig(const FClassConfigInfo& InConfig);

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

	// ===== 身份管理 =====
	UFUNCTION(BlueprintCallable, Category = "CharacterIdentity")
	void SetIdentity(ECharacterIdentity InIdentity);

	UFUNCTION(BlueprintPure, Category = "CharacterIdentity")
	ECharacterIdentity GetIdentity() const { return CurrentIdentity; }

	// 用 PhantomData 初始化灵体外观
	UFUNCTION(BlueprintCallable, Category = "Phantom")
	void ApplyPhantomData(const FPhantomData& InData);

	// 获取灵体数据
	UFUNCTION(BlueprintPure, Category = "Phantom")
	const FPhantomData& GetPhantomData() const { return PhantomData; }

	// 设置能否与世界交互
	UFUNCTION(BlueprintCallable, Category = "Phantom")
	void SetInteractionEnabled(bool bEnabled);

	// 遣返灵体
	UFUNCTION(BlueprintCallable, Category = "Phantom")
	void Repatriate(EReturnReason InReason);

protected:
	// 重建灵体外观
	void RebuildAppearance();

	// 应用半透明视觉效果
	void ApplyTranslucentEffect();

	// 应用灵体交互限制
	void ApplyPhantomRestrictions();

	// 属性复制回调
	UFUNCTION()
	void OnRep_PhantomData();

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void InitPartmentComponent();

public:

	// 获取背包组件引用
	UFUNCTION(BlueprintPure, Category = "CharacterOperation")
		USL_InventoryComponent* GetInventoryComponentRef() const { return InventoryCmp; }

	// 获取锁定组件引用
	UFUNCTION(BlueprintPure, Category = "CharacterOperation")
		USL_LockOnComponent* GetLockOnComponentRef() const { return LockOnCmp; }

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void BindGASDeathEvent();

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void BindGASReviveEvent();

	// GAS角色死亡回调
	UFUNCTION()
		void OnGASCharacterDied(AActor* DiedActor, AActor* KillerActor);

	// GAS角色复活回调
	UFUNCTION()
		void OnGASCharacterRevive(AActor* ReviveActor);

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// ===== 属性值访问器 =====
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetCurrentHealth() const;
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetMaxHealth() const;
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetCurrentStamina() const;
	UFUNCTION(BlueprintPure, Category = "Attributes")
	float GetMaxStamina() const;

	// ICombatEventDisplay_IF 接口实现
	virtual void BroadcastDamageFloatingText(const struct FDamageFloatingTextData& InData) override;
	virtual void BroadcastCharacterDeath(AActor* InDeadActor, AActor* InInstigator) override;

	// 广播伤害飘字（服务器→所有客户端）
	UFUNCTION(NetMulticast, Reliable, Category = "Combat")
	void Multicast_OnDamageFloatingText(const struct FDamageFloatingTextData& InData);

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void InitCharacterWithClassID(int32 InPlayerClassID);

	UFUNCTION(BlueprintPure, Category = "CharacterOperation")
		int32 GetClassID() const { return PlayerClassID; }

	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void SetClassID(int32 InPlayerClassID);

	/** 根据职业配置授予GAS能力（替代蓝图中手动GiveAbility） */
	UFUNCTION(BlueprintCallable, Category = "CharacterOperation")
		void GrantAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InAbilities);

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
		USL_LockOnComponent* LockOnCmp;

	UPROPERTY()
		USL_StateComponent* StateCmp;

	UPROPERTY()
		USL_MovementComponent* MovementCmp;

	UPROPERTY()
		UWidgetComponent* ScreenWidgetCmp;

	UPROPERTY()
		USL_ComboManagerComponent* ComboManagerCmp;

	// ===== 身份相关 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Identity")
	ECharacterIdentity CurrentIdentity;

	// 灵体数据
	UPROPERTY(ReplicatedUsing = OnRep_PhantomData)
	FPhantomData PhantomData;

	// 能否与世界交互
	UPROPERTY(Replicated)
	bool bCanInteractWithWorld;

	// 能否受到伤害
	UPROPERTY(Replicated)
	bool bCanBeDamagedByWorld;

	// 半透明材质实例
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> PhantomMaterials;

	// 半透明材质覆盖
	UPROPERTY(EditDefaultsOnly, Category = "Identity|Visual")
	TSoftObjectPtr<UMaterialInterface> PhantomMaterialOverride;

	// 这个状态仅用于确定是否需要触发死亡布娃娃系统
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		EPlayerState CurrentState;

	// 角色配置
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
		FClassConfigInfo ClassConfig;

	int32 PlayerClassID;

	// 所属队伍
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character")
		int32 TeamID;

	// 死亡委托的句柄
	FDelegateHandle OnCharacterDiedHandle;

	// 复活委托的句柄
	FDelegateHandle OnCharacterLivedHandle;
};