// Public/Component/Character/SL_EquipmentComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include <WeaponAnimNotify_IF.h>
#include <SL_WeaponBase.h>
#include "SL_EquipmentComponent.generated.h"

class ASL_CharacterBase;

/** 武器变更事件委托 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponEquippedDelegate, ASL_WeaponBase*, Weapon, EArrowKeyType, HandType);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULLIKEDEMO_API USL_EquipmentComponent : public UActorComponent, public IWeaponAnimNotify_IF
{
	GENERATED_BODY()

public:
	USL_EquipmentComponent();


	/************************************************************************/
	/*                               接口实现                               */
	/************************************************************************/
	virtual void WeaponAnimNotifyResponse(int NotifyType) override;

	/************************************************************************/
	/*                               外部调用                               */
	/************************************************************************/
	// 设置组件的持有者
	void SetOwner(AActor* NewOwner);

	// ===== 初始化 =====
	/** 根据职业索引初始化装备组件 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
		void InitializeWithClassID(int32 ClassID);

	/** 直接传入配置数据初始化（用于测试/动态配置） */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
		void InitializeWithConfig(const FClassConfigInfo& ClassConfig);

	// ===== 武器访问接口 =====
	/** 获取当前左手武器 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
		ASL_WeaponBase* GetCurrentLeftHandWeapon() const { return CurrentLeftHandWeapon; }

	/** 获取当前右手武器 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
		ASL_WeaponBase* GetCurrentRightHandWeapon() const { return CurrentRightHandWeapon; }

	/** 获取指定手的武器 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
		ASL_WeaponBase* GetWeaponByHand(int32 bIsRightHand) const;

	// ===== 道具操作接口 =====
	/** 使用上方道具 */
	UFUNCTION(BlueprintCallable, Category = "Equipment")
		void UseSelectedSlotItem();

	/** 获取上方道具ID */
	UFUNCTION(BlueprintPure, Category = "Equipment")
		int32 GetSelectSlotItemID() const;

	// ===== 查询接口 =====
	/** 获取当前职业配置 */
	UFUNCTION(BlueprintPure, Category = "Equipment")
		const FClassConfigInfo& GetClassConfig() const { return CurrentClassConfig; }

	/** 获取当前职业ID */
	UFUNCTION(BlueprintPure, Category = "Equipment")
		int32 GetClassID() const { return CurrentClassConfig.ClassID; }

public:
	/************************************************************************/
	/*                               外部访问                               */
	/************************************************************************/
	// ===== 委托 =====
	/** 武器装备变更委托 */
	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
		FOnWeaponEquippedDelegate OnLeftHandWeaponEquipped;

	/** 右手武器装备变更委托 */
	UPROPERTY(BlueprintAssignable, Category = "Equipment|Events")
		FOnWeaponEquippedDelegate OnRightHandWeaponEquipped;

protected:
	/************************************************************************/
	/*                               内部调用                               */
	/************************************************************************/
	/** 根据武器ID派生武器实例 */
	ASL_WeaponBase* SpawnWeaponByID(int32 WeaponID);

	/** 销毁武器实例 */
	void DestroyWeapon(ASL_WeaponBase* Weapon);

	/** 装备左手武器 */
	void EquipLeftHandWeapon(int32 WeaponID);

	/** 装备右手武器 */
	void EquipRightHandWeapon(int32 WeaponID);

	/** 解除当前武器 */
	void UnequipCurrentWeapon(ASL_WeaponBase*& CurrentWeapon);

protected:
	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/
	// ===== 职业配置 =====
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Config")
		FClassConfigInfo CurrentClassConfig;

	// ===== 武器实例 =====
	/** 当前左手武器实例 */
	UPROPERTY()
		ASL_WeaponBase* CurrentLeftHandWeapon;

	/** 当前右手武器实例 */
	UPROPERTY()
		ASL_WeaponBase* CurrentRightHandWeapon;

	// ===== 道具数据 =====
	/** 道具集 */
	UPROPERTY()
		TArray<int32> SlotItemIDList;

	UPROPERTY()
		int32 CurrentSelectSlotIndex;

	// ===== 缓存 =====
	UPROPERTY()
		ASL_CharacterBase* OwningCharacter;

	/** 武器派生模板类 */
	UPROPERTY(EditDefaultsOnly, Category = "Equipment|Config")
		TSubclassOf<ASL_WeaponBase> WeaponBaseClass;
};