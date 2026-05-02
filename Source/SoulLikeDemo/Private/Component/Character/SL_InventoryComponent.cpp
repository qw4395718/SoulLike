#include "SL_InventoryComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "ItemDataObject.h"
#include "ItemDataStruct.h"
#include "Engine/DataTable.h"

USL_InventoryComponent::USL_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxInventorySlots = 30;
}

void USL_InventoryComponent::InitInventoryComponentInfo()
{
	// 初始化装备表，道具表等本地数据接口
	// 等数据表完成后再完善

}

void USL_InventoryComponent::InitEquipmentInfo(const TArray<int> WeaponList, const TArray<int> ItemList, TMap<EArrowKeyType, int> ActiveSlotIndex)
{
	//根据查表将 WeapList->EquipmentInfoList,暂时用拟造数据代替
	UnitTest(WeaponList);
	InventoryEquippableItemInfoList = ItemList;
	CurrentActiveEquipmentIndex = ActiveSlotIndex;
}

void USL_InventoryComponent::InitInventoryWeaponInfo(TArray<int> WeaponList)
{
	InventoryWeaponInfoList = WeaponList;
}

void USL_InventoryComponent::InitInventoryEquippableItemInfo(TArray<int> EquippableItemList)
{
	InventoryEquippableItemInfoList = EquippableItemList;
}

void USL_InventoryComponent::GetEquipmentInfoList(TArray<FWeaponData>& EquipmentInfoList)
{
	EquipmentInfoList = EquipWeaponInfoList;
}

void USL_InventoryComponent::UnitTest(const TArray<int> WeaponList)
{
	//// 后续完成读表的初始化
	//EquipWeaponInfoList.Reset();
	//for(int WeaponID : WeaponList)
	//{
	//	FWeaponData UnitTestData;
	//	UnitTestData.WeaponID = WeaponID;
	//	UnitTestData.WeaponUniqueID = WeaponID * 100 + 1;
	//	UnitTestData.Mesh = TEXT("/Game/InfinityBladeWeapons/Weapons/Blade/Swords/Blade_BlackKnight/SK_Blade_BlackKnight.SK_Blade_BlackKnight");
	//	UnitTestData.SocketName = TEXT("ik_hand_r");
	//	UnitTestData.AnimClass = TEXT("");
	//	UnitTestData.AttackMentageName = TEXT("/Game/SoulLikeDemo/Anim/AM_Attack_Sword.AM_Attack_Sword");
	//	UnitTestData.ComboSkillMentageName = TEXT("/Game/SoulLikeDemo/Anim/AM_CombatSkill_Sheild.AM_CombatSkill_Sheild");
	//	UnitTestData.ExecuteMentageName = TEXT("/Game/SoulLikeDemo/Anim/AM_Execute_Sword.AM_Execute_Sword");
	//	UnitTestData.BackStabMentageName = TEXT("/Game/SoulLikeDemo/Anim/AM_BackStab_Sword.AM_BackStab_Sword");
	//	UnitTestData.WeaponCollisionBoxSize.X = 100.0f;
	//	UnitTestData.WeaponCollisionBoxSize.Y = 5.0f;
	//	UnitTestData.WeaponCollisionBoxSize.Z = 5.0f;
	//	// 武器数据信息
	//	FAbilityCostInfo AbilityCostInfo;
	//	AbilityCostInfo.HealthCost = 20.0f;
	//	AbilityCostInfo.StaminaCost = 20.0f;
	//	AbilityCostInfo.MagicCost = 20.0f;
	//	FComboCoordinatorInfo ComboCoordinatorInfo;
	//	ComboCoordinatorInfo.BaseDamage = 20.0f;
	//	ComboCoordinatorInfo.ComboMaxNum = 3;
	//	ComboCoordinatorInfo.CurrentComboNum = 0;
	//	ComboCoordinatorInfo.CanContinueCombo = false;
	//	for (int i = 0; i <= ComboCoordinatorInfo.ComboMaxNum; i++)
	//	{
	//		ComboCoordinatorInfo.ComboDamageMultiplier.Add(i+1);
	//		ComboCoordinatorInfo.ComboStateCostMultiplier.Add(i, AbilityCostInfo);
	//	}
	//	UnitTestData.ComboCoordinatorInfoMap.Reset();
	//	UnitTestData.ComboCoordinatorInfoMap.Add(EWeaponModeTyoe::WEAPONMODE_Attack, ComboCoordinatorInfo);
	//	UnitTestData.ComboCoordinatorInfoMap.Add(EWeaponModeTyoe::WEAPONMODE_Defence, ComboCoordinatorInfo);
	//	UnitTestData.ComboCoordinatorInfoMap.Add(EWeaponModeTyoe::WEAPONMODE_ComboSkill, ComboCoordinatorInfo);
	//	UnitTestData.ComboCoordinatorInfoMap.Add(EWeaponModeTyoe::WEAPONMODE_BackStab, ComboCoordinatorInfo);
	//	UnitTestData.ComboCoordinatorInfoMap.Add(EWeaponModeTyoe::WEAPONMODE_Execute, ComboCoordinatorInfo);

	//	UnitTestData.NeedLoadComponentInfoMap.Add(EWeaponComponentType::MeleeAttack, true);
	//	UnitTestData.NeedLoadComponentInfoMap.Add(EWeaponComponentType::Parry, true);
	//	UnitTestData.NeedLoadComponentInfoMap.Add(EWeaponComponentType::Execute, true);
	//	UnitTestData.NeedLoadComponentInfoMap.Add(EWeaponComponentType::BackStab, true);

	//	EquipWeaponInfoList.Add(UnitTestData);
	//}

	
}

void USL_InventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 初始化库存数组大小
	InventoryItems.SetNum(MaxInventorySlots);
}

bool USL_InventoryComponent::AddItem(UItemData* ItemToAdd)
{
	if (!ItemToAdd || ItemToAdd->ItemCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid item to add"));
		return false;
	}

	// 首先尝试堆叠到已有物品
	if (ItemToAdd->MaxStackCount > 1)
	{
		if (TryStackItem(ItemToAdd))
		{
			OnInventoryUpdated.Broadcast();
			return true;
		}
	}

	// 寻找空位放置新物品
	for (int32 i = 0; i < MaxInventorySlots; i++)
	{
		if (!InventoryItems[i])
		{
			InventoryItems[i] = ItemToAdd;
			OnInventoryUpdated.Broadcast();
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Inventory is full!"));
	return false;
}

bool USL_InventoryComponent::AddItemByID(FName ItemID, int32 Count)
{
	if (Count <= 0) return false;

	UItemData* NewItem = CreateItemFromDataTable(ItemID);
	if (!NewItem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item ID not found: %s"), *ItemID.ToString());
		return false;
	}

	NewItem->ItemCount = Count;
	return AddItem(NewItem);
}

bool USL_InventoryComponent::RemoveItem(UItemData* ItemToRemove, int32 Count)
{
	if (!ItemToRemove) return false;

	int32 Index = FindItemIndex(ItemToRemove);
	if (Index == INDEX_NONE) return false;

	return RemoveItemByIndex(Index, Count);
}

bool USL_InventoryComponent::RemoveItemByIndex(int32 Index, int32 Count)
{
	if (Index < 0 || Index >= InventoryItems.Num() || !InventoryItems[Index])
	{
		return false;
	}

	UItemData* Item = InventoryItems[Index];

	if (Count >= Item->ItemCount || Count == 0)
	{
		// 移除整个物品
		InventoryItems[Index] = nullptr;
	}
	else
	{
		// 只减少数量
		Item->ItemCount -= Count;
	}

	OnInventoryUpdated.Broadcast();
	return true;
}

UItemData* USL_InventoryComponent::FindItemByID(FName ItemID) const
{
	for (UItemData* Item : InventoryItems)
	{
		if (Item && Item->ItemID == ItemID)
		{
			return Item;
		}
	}
	return nullptr;
}

int32 USL_InventoryComponent::FindItemIndex(UItemData* Item) const
{
	if (!Item) return INDEX_NONE;

	return InventoryItems.Find(Item);
}

int32 USL_InventoryComponent::GetItemCount(FName ItemID) const
{
	int32 TotalCount = 0;

	for (UItemData* Item : InventoryItems)
	{
		if (Item && Item->ItemID == ItemID)
		{
			TotalCount += Item->ItemCount;
		}
	}

	return TotalCount;
}

bool USL_InventoryComponent::UseItem(UItemData* ItemToUse)
{
	if (!ItemToUse || !ItemToUse->bCanUse) return false;

	// 调用物品的使用方法
	AActor* Owner = GetOwner();
	ItemToUse->UseItem(Owner);

	// 广播使用事件
	OnItemUsed.Broadcast(ItemToUse);

	// 如果是消耗品，减少数量
	if (ItemToUse->ItemType == EItemType::Consumable)
	{
		RemoveItem(ItemToUse, 1);
	}

	return true;
}

bool USL_InventoryComponent::UseItemByIndex(int32 Index)
{
	if (Index < 0 || Index >= InventoryItems.Num() || !InventoryItems[Index])
	{
		return false;
	}

	return UseItem(InventoryItems[Index]);
}

bool USL_InventoryComponent::SwapItems(int32 IndexA, int32 IndexB)
{
	if (IndexA < 0 || IndexA >= InventoryItems.Num() ||
		IndexB < 0 || IndexB >= InventoryItems.Num() ||
		IndexA == IndexB)
	{
		return false;
	}

	// 交换物品
	Swap(InventoryItems[IndexA], InventoryItems[IndexB]);

	OnInventoryUpdated.Broadcast();
	return true;
}

void USL_InventoryComponent::SortInventory()
{
	// 先按位置分组
	TArray<UItemData*> SortedItems;
	TArray<UItemData*> EmptySlots;

	for (UItemData* Item : InventoryItems)
	{
		if (Item)
		{
			SortedItems.Add(Item);
		}
		else
		{
			EmptySlots.Add(nullptr);
		}
	}

	// 排序非空物品
	SortedItems.Sort([](const UItemData& A, const UItemData& B) {
		// 先按类型排序
		if (A.ItemType != B.ItemType)
		{
			return A.ItemType < B.ItemType;
		}
		// 然后按稀有度排序
		if (A.Rarity != B.Rarity)
		{
			return A.Rarity < B.Rarity;
		}
		// 最后按名称排序
		return A.ItemName.CompareTo(B.ItemName) < 0;
		});

	// 合并回原数组
	SortedItems.Append(EmptySlots);

	// 确保大小一致
	if (SortedItems.Num() > MaxInventorySlots)
	{
		SortedItems.SetNum(MaxInventorySlots);
	}
	else if (SortedItems.Num() < MaxInventorySlots)
	{
		SortedItems.SetNum(MaxInventorySlots);
	}

	InventoryItems = SortedItems;
	OnInventoryUpdated.Broadcast();
}

float USL_InventoryComponent::GetTotalWeight() const
{
	float TotalWeight = 0.0f;

	for (UItemData* Item : InventoryItems)
	{
		if (Item)
		{
			TotalWeight += Item->ItemWeight * Item->ItemCount;
		}
	}

	return TotalWeight;
}

bool USL_InventoryComponent::HasEmptySlot() const
{
	for (UItemData* Item : InventoryItems)
	{
		if (!Item) return true;
	}
	return false;
}

int32 USL_InventoryComponent::GetEmptySlotCount() const
{
	int32 EmptyCount = 0;

	for (UItemData* Item : InventoryItems)
	{
		if (!Item) EmptyCount++;
	}

	return EmptyCount;
}

void USL_InventoryComponent::ClearInventory()
{
	for (int32 i = 0; i < InventoryItems.Num(); i++)
	{
		InventoryItems[i] = nullptr;
	}

	OnInventoryUpdated.Broadcast();
}

// 私有方法实现
bool USL_InventoryComponent::TryStackItem(UItemData* ItemToAdd)
{
	for (UItemData* ExistingItem : InventoryItems)
	{
		if (ExistingItem && ExistingItem->CanStackWith(ItemToAdd))
		{
			ExistingItem->MergeStack(ItemToAdd);

			// 如果添加的物品数量变为0，说明完全堆叠了
			if (ItemToAdd->ItemCount <= 0)
			{
				return true;
			}
		}
	}

	return false;
}

UItemData* USL_InventoryComponent::CreateItemFromDataTable(FName ItemID) const
{
	if (!ItemDataTable) return nullptr;

	FItemDataRow* RowData = ItemDataTable->FindRow<FItemDataRow>(ItemID, TEXT("CreateItemFromDataTable"));
	if (!RowData) return nullptr;

	// 创建物品实例
	UItemData* NewItem = NewObject<UItemData>();
	NewItem->ItemID = RowData->ItemID;
	NewItem->ItemName = RowData->ItemName;
	NewItem->ItemDescription = RowData->ItemDescription;
	NewItem->ItemIcon = RowData->ItemIcon;
	NewItem->MaxStackCount = RowData->MaxStackCount;
	NewItem->Rarity = RowData->Rarity;
	NewItem->ItemType = RowData->ItemType;
	NewItem->ItemWeight = RowData->ItemWeight;
	NewItem->ItemValue = RowData->ItemValue;
	NewItem->bCanDrop = RowData->bCanDrop;
	NewItem->bCanUse = RowData->bCanUse;

	return NewItem;
}
