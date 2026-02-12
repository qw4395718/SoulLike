// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "WeaponData.h"
#include "SL_InventoryComponent.generated.h"




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API USL_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USL_InventoryComponent();

public:
	/************************************************************************/
	/*									外部调用                                   */
	/************************************************************************/
	// 初始化仓库组件信息
	UFUNCTION()
		void InitInventoryComponentInfo();

	// 初始化装备栏
	UFUNCTION()
		void InitEquipmentInfo(const TArray<int> WeaponList, const TArray<int> ItemList, TMap<EArrowKeyType, int> ActiveSlotIndex);

	// 初始化仓库信息
	UFUNCTION()
		void InitInventoryWeaponInfo(TArray<int> WeaponList);

	UFUNCTION()
		void InitInventoryEquippableItemInfo(TArray<int> EquippableItemList);

	UFUNCTION()
		void GetEquipmentInfoList(TArray<FWeaponData> & EquipmentInfoList);


protected:
	/************************************************************************/
	/*                                  内部调用                                    */
	/************************************************************************/
	
	//拟造数据
	void UnitTest(const TArray<int> WeaponList);

protected:
	/************************************************************************/
	/*                                   变量                                   */
	/************************************************************************/
	// 当前装备栏激活索引
	UPROPERTY()
		TMap<EArrowKeyType,int> CurrentActiveEquipmentIndex;

	// 装备武器栏信息数组
	UPROPERTY()
		TArray<FWeaponData> EquipWeaponInfoList;

	// 装备道具栏信息数组
	UPROPERTY()
		TArray<int> EquipItemInfoList;
	
	 // 目前其他装备先简单的来，建构一个简易的框架，待后续UI开始后，重新设计数据结构
	UPROPERTY()
		TArray<int> InventoryWeaponInfoList;

	UPROPERTY()
		TArray<int> InventoryEquippableItemInfoList;

	// 头盔,护甲......
};
