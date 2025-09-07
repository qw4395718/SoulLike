#include "SL_InventoryComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_InventoryComponent::USL_InventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USL_InventoryComponent::InitInventoryComponentInfo()
{
	// 初始化装备表，道具表等本地数据接口
	// 等数据表完成后再完善

}

void USL_InventoryComponent::InitEquipmentInfo(const TArray<int> WeaponList, const TArray<int> ItemList, TMap<EArrowKeyType, int> ActiveSlotIndex)
{
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
