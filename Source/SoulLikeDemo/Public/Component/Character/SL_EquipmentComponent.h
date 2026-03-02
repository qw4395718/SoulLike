// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "WeaponBase.h"
#include "WeaponData.h"
#include "WeaponBehavior_IF.h"
#include "Containers/Map.h"
#include "SL_WeaponBase.h"
#include "SL_EquipmentComponent.generated.h"

const INT EQUIPMENT_SLOT_NUM  = 5;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_EquipmentComponent : public UActorComponent,public IWeaponBehavior_IF
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_EquipmentComponent();

public:
	/************************************************************************/
	/*                                接口实现                                      */
	/************************************************************************/
	// 攻击行为响应
	UFUNCTION()
		void AttackBehaviorResponse(AActor* OwnerActor) override;

	// 防御行为响应
	UFUNCTION()
		void DefenceBehaviorResponse(AActor* OwnerActor) override;

	// 技能行为响应
	UFUNCTION()
		void ComboSkillBehaviorResponse(AActor* OwnerActor) override;

	// 处决行为响应
	UFUNCTION()
		void ExecuteBehaviorResponse(AActor* OwnerActor) override;

	// 背刺行为响应
	UFUNCTION()
		void BackStabBehaviorResponse(AActor* OwnerActor) override;

public:
	/************************************************************************/
	/*                                 外部调用                                     */
	/************************************************************************/
	UFUNCTION()
		void InitEquipmentComponent(const TArray<FWeaponData> WeaponList,const TArray<int> ItemList, TMap<EArrowKeyType, int> ActiveSlotIndex ,AActor* OwnerActor);

	UFUNCTION()
		void UseUpSlotItemEvent();

	UFUNCTION()
		void UseDownSlotItemEvent();

	UFUNCTION()
		void SwitchEquipmentEvent(EArrowKeyType ArrowType);

	UFUNCTION()
		void SetEquipemntInfo(EArrowKeyType ArrowType,int SlotIndex, FWeaponData& WeaponInfo);

	UFUNCTION()
		void SetCostItemInfo(EArrowKeyType ArrowType, int SlotIndex, int ItemID);

	UFUNCTION()
		void CleanCostItemInfo(EArrowKeyType ArrowType, int SlotIndex);

	UFUNCTION()
		ASL_WeaponBase* GetCurrentLHWeapon();

	UFUNCTION()
		ASL_WeaponBase* GetCurrentRHWeapon();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "CallLuaByFLuaTable", Category = "UnLua Tutorial"))
		static void CallLuaByFLuaTable();

protected:
	/************************************************************************/
	/*                                  内部调用                                    */
	/************************************************************************/


protected:
	/************************************************************************/
	/*                                  变量                                    */
	/************************************************************************/
	// 当前四个装备槽的当前Index
	UPROPERTY()
		TMap<EArrowKeyType,int> CurrentEquipmentIndex;

	// 左手装备信息数组
	UPROPERTY()
		ASL_WeaponBase* LeftHandEquipmentInfoList[EQUIPMENT_SLOT_NUM];

	// 当前左手武器
	UPROPERTY()
		ASL_WeaponBase* CurrentLeftHandWeapon;

	// 右手装备信息数组
	UPROPERTY()
		ASL_WeaponBase* RightHandEquipmentInfoList[EQUIPMENT_SLOT_NUM];

	// 当前左手武器
	UPROPERTY()
		ASL_WeaponBase* CurrentRightHandWeapon;

	// 上方装备道具信息
	UPROPERTY()
		int UpItemList[EQUIPMENT_SLOT_NUM];

	// 当前上方装备道具
	UPROPERTY()
		int CurrentUpSlotItemID;

	// 下方装备道具信息
	UPROPERTY()
		int DownItemList[EQUIPMENT_SLOT_NUM];
	
	// 当前下方装备道具
	UPROPERTY()
		int CurrentDownSlotItemID;

	//// 持有者信息
	//UPROPERTY()
	//	AActor* Owning;
};
