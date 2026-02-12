// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "WeaponData.h"
#include "ItemDataObject.h"
#include "SL_InventoryComponent.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsed, UItemData*, Item);

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

	// 获取所有库存物品（用于UI绑定）
	UFUNCTION(BlueprintPure, Category = "Inventory")
		TArray<UItemData*> GetInventoryItems() const { return InventoryItems; }

	// 添加物品到库存
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool AddItem(UItemData* ItemToAdd);

	// 添加物品通过ID（从数据表加载）
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool AddItemByID(FName ItemID, int32 Count = 1);

	// 移除物品
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool RemoveItem(UItemData* ItemToRemove, int32 Count = 1);

	// 移除物品通过索引
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool RemoveItemByIndex(int32 Index, int32 Count = 1);

	// 查找物品
	UFUNCTION(BlueprintPure, Category = "Inventory")
		UItemData* FindItemByID(FName ItemID) const;

	// 查找物品索引
	UFUNCTION(BlueprintPure, Category = "Inventory")
		int32 FindItemIndex(UItemData* Item) const;

	// 获取物品数量
	UFUNCTION(BlueprintPure, Category = "Inventory")
		int32 GetItemCount(FName ItemID) const;

	// 使用物品
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool UseItem(UItemData* ItemToUse);

	// 使用物品通过索引
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool UseItemByIndex(int32 Index);

	// 交换物品位置
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool SwapItems(int32 IndexA, int32 IndexB);

	// 整理库存（按类型和稀有度排序）
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void SortInventory();

	// 获取总负重
	UFUNCTION(BlueprintPure, Category = "Inventory")
		float GetTotalWeight() const;

	// 检查是否有空位
	UFUNCTION(BlueprintPure, Category = "Inventory")
		bool HasEmptySlot() const;

	// 获取空位数量
	UFUNCTION(BlueprintPure, Category = "Inventory")
		int32 GetEmptySlotCount() const;

	// 清空库存
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void ClearInventory();

	virtual void BeginPlay() override;
protected:
	/************************************************************************/
	/*                                  内部调用                                    */
	/************************************************************************/


	//拟造数据
	void UnitTest(const TArray<int> WeaponList);

	// 内部：尝试堆叠物品
	bool TryStackItem(UItemData* ItemToAdd);

	// 内部：创建物品实例
	UItemData* CreateItemFromDataTable(FName ItemID) const;

	// 内部：整理比较函数
	static bool CompareItems(const UItemData& A, const UItemData& B);

public:
	/************************************************************************/
	/*	                                 委托                                    */
	/************************************************************************/
	// 库存更新委托
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnInventoryUpdated OnInventoryUpdated;

	// 物品使用委托
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnItemUsed OnItemUsed;


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

		// 最大库存格子数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ClampMin = "1", ClampMax = "500"))
		int32 MaxInventorySlots;

	// 当前库存物品列表
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
		TArray<UItemData*> InventoryItems;

	// 数据表引用（用于通过ID创建物品）
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		UDataTable* ItemDataTable;


};
