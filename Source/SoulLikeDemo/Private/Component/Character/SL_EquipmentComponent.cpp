#include "SL_EquipmentComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "SL_WeaponBase.h"
#include "UnLua.h"

USL_EquipmentComponent::USL_EquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 默认初始化
	CurrentLeftHandWeapon = nullptr;
	CurrentRightHandWeapon = nullptr;
	CurrentUpSlotItemID = 0;
	CurrentDownSlotItemID = 0;
	CurrentEquipmentIndex.Add(EArrowKeyType::ARROWKEY_Up, 0);
	CurrentEquipmentIndex.Add(EArrowKeyType::ARROWKEY_Down, 0);
	CurrentEquipmentIndex.Add(EArrowKeyType::ARROWKEY_Left, 0);
	CurrentEquipmentIndex.Add(EArrowKeyType::ARROWKEY_Right, 0);
	for (int i = 0; i < EQUIPMENT_SLOT_NUM; i++)
	{
		LeftHandEquipmentInfoList[i] = nullptr;
		RightHandEquipmentInfoList[i] = nullptr;
		UpItemList[i] = 0;
		DownItemList[i] = 0;
	}
}

void USL_EquipmentComponent::AttackBehaviorResponse(AActor* OwnerActor)
{
}

void USL_EquipmentComponent::DefenceBehaviorResponse(AActor* OwnerActor)
{

}

void USL_EquipmentComponent::ComboSkillBehaviorResponse(AActor* OwnerActor)
{
}

void USL_EquipmentComponent::ExecuteBehaviorResponse(AActor* OwnerActor)
{
}

void USL_EquipmentComponent::BackStabBehaviorResponse(AActor* OwnerActor)
{
}


void USL_EquipmentComponent::InitEquipmentComponent(const TArray<FWeaponData> WeaponList, const TArray<int> ItemList, TMap<EArrowKeyType, int> ActiveSlotIndex,AActor* OwnerActor)
{
}

void USL_EquipmentComponent::UseUpSlotItemEvent()
{
	// 后续添加道具使用的接口，把所属槽位和索引发射出去
}

void USL_EquipmentComponent::UseDownSlotItemEvent()
{
	// 后续添加道具使用的接口，把所属槽位和索引发射出去
}

void USL_EquipmentComponent::SwitchEquipmentEvent(EArrowKeyType ArrowType)
{
}

void USL_EquipmentComponent::SetEquipemntInfo(EArrowKeyType ArrowType, int SlotIndex, FWeaponData& WeaponInfo)
{
}

void USL_EquipmentComponent::SetCostItemInfo(EArrowKeyType ArrowType, int SlotIndex, int ItemID)
{
}

void USL_EquipmentComponent::CleanCostItemInfo(EArrowKeyType ArrowType, int SlotIndex)
{
}

ASL_WeaponBase* USL_EquipmentComponent::GetCurrentLHWeapon()
{
	return CurrentLeftHandWeapon;
}

ASL_WeaponBase* USL_EquipmentComponent::GetCurrentRHWeapon()
{
	return CurrentRightHandWeapon;
}

void USL_EquipmentComponent::CallLuaByFLuaTable()
{
}

