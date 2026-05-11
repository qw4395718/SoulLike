// Public/Component/Character/SL_InventoryComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemDataStruct.h"
#include "SL_InventoryComponent.generated.h"

class ASL_CharacterBase;
class USL_GameplayAbilityUseItem;

/**
 * 库存组件（基于GAS + DataTable 重构版）
 * 管理玩家持有的道具，提供使用、添加、移除等功能
 * 能力生命周期：整个角色生命周期中只需授予一次 GA_UseItem
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULLIKEDEMO_API USL_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USL_InventoryComponent();

	/************************************************************************/
	/*                               外部调用                               */
	/************************************************************************/
	// 初始化库存组件
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void InitializeInventory();

	// ===== 道具管理 =====
	// 添加道具（通过ID和数量）
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool AddItemByID(FName InItemID, int32 InCount = 1);

	// 移除道具（通过ID和数量）
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool RemoveItemByID(FName InItemID, int32 InCount = 1);

	// 获取道具数量
	UFUNCTION(BlueprintPure, Category = "Inventory")
		int32 GetItemCount(FName InItemID) const;

	// 获取所有道具ID列表
	UFUNCTION(BlueprintPure, Category = "Inventory")
		TArray<FName> GetAllItemIDs() const;

	// ===== 道具使用 =====
	// 使用指定ID的道具（由UI或输入调用）
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool UseItemByID(FName InItemID);

	// 使用当前选中的道具（绑定到"E"键）
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		bool UseSelectedItem();

	// 设置当前选中的道具ID
	UFUNCTION(BlueprintCallable, Category = "Inventory")
		void SetSelectedItemID(FName InNewItemID) { SelectedItemID = InNewItemID; }

	// 获取当前选中的道具ID
	UFUNCTION(BlueprintPure, Category = "Inventory")
		FName GetSelectedItemID() const { return SelectedItemID; }

	// 检查道具是否可使用
	UFUNCTION(BlueprintPure, Category = "Inventory")
		bool CanUseItem(FName InItemID) const;

protected:
	/************************************************************************/
	/*                               继承实现                               */
	/************************************************************************/
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/************************************************************************/
	/*                               内部调用                               */
	/************************************************************************/
	// 授予通用道具使用能力（角色一生只需授予一次）
	void GrantUseItemAbility();

	// 通过道具ID找到对应的GameplayTag并激活能力
	bool ActivateItemAbility(FName InItemID);

	// 监听道具使用事件，更新库存
	UFUNCTION()
		void OnItemUsedCallback(AActor* InUserActor, FName InItemID);

	// 获取归属的Character
	ASL_CharacterBase* GetOwningCharacter() const;

	// 获取归属的ASC
	class USL_AbilitySystemComponent* GetAbilitySystemComponent() const;

protected:
	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/
	// 通用道具使用能力的类
	UPROPERTY(EditDefaultsOnly, Category = "Inventory|GAS")
		TSubclassOf<USL_GameplayAbilityUseItem> UseItemAbilityClass;

	// 道具库存（道具ID -> 数量）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
		TMap<FName, int32> ItemInventory;

	// 当前选中的道具ID（用于"E"键使用）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
		FName SelectedItemID;

	// 委托句柄
	FDelegateHandle OnItemUsedHandle;

	// 标记能力是否已授予
	bool bAbilityGranted;
};