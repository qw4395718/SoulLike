#include "SL_EquipmentComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_EquipmentComponent::USL_EquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	pCurrentLeftHandWeapon = nullptr;
	pCurrentRightHandWeapon = nullptr;
}

void USL_EquipmentComponent::InitEquipment()
{

}

void USL_EquipmentComponent::LeftMouseEvent()
{
	// 检查是否有右手武器
	if (pCurrentRightHandWeapon != nullptr)
	{
		pCurrentRightHandWeapon->PerformAttack();
	}
}

void USL_EquipmentComponent::RightMouseEvent()
{
	// 判定是否有左手武器
	if (pCurrentLeftHandWeapon != nullptr)
	{
		pCurrentLeftHandWeapon->PerformDefence();
	}
}

void USL_EquipmentComponent::CtrlKeyEvent()
{
	// 检查是否有右手武器
	if (pCurrentRightHandWeapon != nullptr)
	{
		pCurrentRightHandWeapon->PerformComboSkill();
	}
	else if (pCurrentLeftHandWeapon != nullptr)
	{
		pCurrentLeftHandWeapon->PerformComboSkill();
	}
	else
	{
		// 双手都未持有武器
	}
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
	if (CurrentEquipmentIndex.Find(ArrowType) != nullptr)
	{
		CurrentEquipmentIndex.FindRef(ArrowType) = (CurrentEquipmentIndex.Find(ArrowType)++) % EQUIPMENT_SLOT_NUM;
	}
}

void USL_EquipmentComponent::AddEquipemntInfo(EArrowKeyType ArrowType, int SlotIndex, FWeaponData* WeaponInfo)
{
	if(SlotIndex < 0 || SlotIndex >= EQUIPMENT_SLOT_NUM){return;}
	// 根据EArrowKeyType区别上下装备槽
	if (ArrowType == EArrowKeyType::Left_ArrowKey)
	{
		LeftHandEquipmentInfoList[SlotIndex] = WeaponInfo;
	}
	else if (ArrowType == EArrowKeyType::Right_ArrowKey)
	{
		RightHandEquipmentInfoList[SlotIndex] = WeaponInfo;
	}
	else
	{
		// 不正确的调用
	}
}

void USL_EquipmentComponent::RemoveEquipemntInfo(EArrowKeyType ArrowType, int SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= EQUIPMENT_SLOT_NUM) { return; }
	// 根据EArrowKeyType区别上下装备槽
	if (ArrowType == EArrowKeyType::Left_ArrowKey)
	{
		LeftHandEquipmentInfoList[SlotIndex] = nullptr;
	}
	else if (ArrowType == EArrowKeyType::Right_ArrowKey)
	{
		RightHandEquipmentInfoList[SlotIndex] = nullptr;
	}
	else
	{
		// 不正确的调用
	}
}

void USL_EquipmentComponent::AddCostItemInfo(EArrowKeyType ArrowType, int SlotIndex, int ItemID)
{
	if (SlotIndex < 0 || SlotIndex >= EQUIPMENT_SLOT_NUM) { return; }
	// 根据EArrowKeyType区别上下装备槽
	if (ArrowType == EArrowKeyType::Up_ArrowKey)
	{
		UpItemList[SlotIndex] = ItemID;
	}
	else if (ArrowType == EArrowKeyType::Down_ArrowKey)
	{
		DownItemList[SlotIndex] = ItemID;
	}
	else
	{
		// 不正确的调用
	}
}

void USL_EquipmentComponent::RemoveCostItemInfo(EArrowKeyType ArrowType, int SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= EQUIPMENT_SLOT_NUM) { return; }
	// 根据EArrowKeyType区别上下装备槽
	if (ArrowType == EArrowKeyType::Up_ArrowKey)
	{
		UpItemList[SlotIndex] = 0;
	}
	else if (ArrowType == EArrowKeyType::Down_ArrowKey)
	{
		DownItemList[SlotIndex] = 0;
	}
	else
	{
		// 不正确的调用
	}
}

void USL_EquipmentComponent::DestroyWeaponClass(ASL_WeaponBase* DestroyWeapon)
{

}

void USL_EquipmentComponent::InitCurrentWeapon(FWeaponData WeaponInfo)
{

}

void USL_EquipmentComponent::LeftHandWeaponAttack()
{

}

void USL_EquipmentComponent::LeftHandWeaponDefence()
{

}

void USL_EquipmentComponent::LeftHandWeaponComboSkill()
{

}

void USL_EquipmentComponent::RightHandWeaponAttack()
{

}

void USL_EquipmentComponent::RightHandWeaponDefence()
{

}

void USL_EquipmentComponent::RightHandWeaponComboSkill()
{

}

