// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "WeaponBase.h"
#include "SL_EquipmentComponent.generated.h"

const EQUIPMENT_SLOT_NUM 5;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_EquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_EquipmentComponent();

public:
	/************************************************************************/
	/*                                接口继承                                      */
	/************************************************************************/


public:
	/************************************************************************/
	/*                                 外部调用                                     */
	/************************************************************************/
	UFUNCTION()
		void InitEquipment();

	UFUNCTION()
		void LeftMouseEvent();

	UFUNCTION()
		void RightMouseEvent();

	UFUNCTION()
		void CtrlKeyEvent();

	UFUNCTION()
		void UseUpSlotItemEvent();

	UFUNCTION()
		void UseDownSlotItemEvent();

	UFUNCTION()
		void SwitchEquipmentEvent(EArrowKeyType ArrowType);

	UFUNCTION()
		void AddEquipemntInfo(EArrowKeyType ArrowType,int SlotIndex, FWeaponData* WeaponInfo);

	UFUNCTION()
		void RemoveEquipemntInfo(EArrowKeyType ArrowType, int SlotIndex);

	UFUNCTION()
		void AddCostItemInfo(EArrowKeyType ArrowType, int SlotIndex, int ItemID);

	UFUNCTION()
		void RemoveCostItemInfo(EArrowKeyType ArrowType, int SlotIndex);

protected:
	/************************************************************************/
	/*                                  内部调用                                    */
	/************************************************************************/
	// 切换武器时需要将当前武器摧毁,回收资源
	UFUNCTION()
		void DestroyWeaponClass(ASL_WeaponBase* DestroyWeapon);

	// 根据武器信息进行初始化
	UFUNCTION()
		void InitCurrentWeapon(FWeaponData WeaponInfo);

	UFUNCTION()
		void LeftHandWeaponAttack();

	UFUNCTION()
		void LeftHandWeaponDefence();

	UFUNCTION()
		void LeftHandWeaponComboSkill();

	UFUNCTION()
		void RightHandWeaponAttack();

	UFUNCTION()
		void RightHandWeaponDefence();

	UFUNCTION()
		void RightHandWeaponComboSkill();

protected:
	/************************************************************************/
	/*                                  变量                                    */
	/************************************************************************/
	// 当前四个装备槽的当前Index
	UPROPERTY()
	TMap<EArrowKeyType,int> CurrentEquipmentIndex;

	// 左手装备信息数组
	UPROPERTY()
		FWeaponData* LeftHandEquipmentInfoList[EQUIPMENT_SLOT_NUM];

	// 当前左手武器
	UPROPERTY()
		ASL_WeaponBase* pCurrentLeftHandWeapon;

	// 右手装备信息数组
	UPROPERTY()
		FWeaponData* RightHandEquipmentInfoList[EQUIPMENT_SLOT_NUM];

	// 当前左手武器
	UPROPERTY()
		ASL_WeaponBase* pCurrentRightHandWeapon;

	// 上方装备道具信息
	UPROPERTY()
		int UpItemList[EQUIPMENT_SLOT_NUM];

	// 当前上方装备道具
	UPROPERTY()
		int iCurrentUpSlotItemID;

	// 下方装备道具信息
	UPROPERTY()
		int DownItemList[EQUIPMENT_SLOT_NUM];
	
	// 当前下方装备道具
	UPROPERTY()
		int iCurrentDownSlotItemID;
};
