// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "Equipment_IF.h"
#include "WeaponBase.h"
#include "SL_EquipmentComponent.generated.h"


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
		void SwitchEquipmentEvent(int handtype);

	UFUNCTION()
		void AddEquipemntInfo(int handtype,int SlotIndex,FWeaponDefinition WeaponInfo);

	UFUNCTION()
		void RemoveEquipemntInfo(int WeaponUniqueID);

protected:
	/************************************************************************/
	/*                                  内部调用                                    */
	/************************************************************************/
	// 切换武器时需要将当前武器摧毁,回收资源
	UFUNCTION()
		void DestroyWeaponClass(ASL_WeaponBase* DestroyWeapon);

	// 根据武器信息进行初始化
	UFUNCTION()
		void InitCurrentWeapon(FWeaponDefinition WeaponInfo);

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
	// 左手装备信息数组
	UPROPERTY()
		TArray<FWeaponDefinition> LeftHandEquipmentInfoList; 

	// 当前左手武器
	UPROPERTY()
		ASL_WeaponBase* pCurrentLeftHandWeapon;

	// 右手装备信息数组
	UPROPERTY()
		TArray<FWeaponDefinition> RightHandEquipmentInfoList;

	// 当前左手武器
	UPROPERTY()
		ASL_WeaponBase* pCurrentRightHandWeapon;

	// 上方装备道具信息
	UPROPERTY()
		TArray<int> UpItemList;

	// 当前上方装备道具
	UPROPERTY()
		int iCurrentUpSlotItemID;

	// 下方装备道具信息
	UPROPERTY()
		TArray<int> DownItemList;
	
	// 当前下方装备道具
	UPROPERTY()
		int iCurrentDownSlotItemID;
};
