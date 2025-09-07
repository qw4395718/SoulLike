#include "SL_EquipmentComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_EquipmentComponent::USL_EquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CurrentLeftHandWeapon = nullptr;
	CurrentRightHandWeapon = nullptr;
}

void USL_EquipmentComponent::AttackBehaviorResponse(AActor* OwnerActor)
{
	// 检查是否有右手武器
	if (CurrentRightHandWeapon != nullptr)
	{
		CurrentRightHandWeapon->Attack(OwnerActor);
	}
}

void USL_EquipmentComponent::DefenceBehaviorResponse(AActor* OwnerActor)
{
	// 判定是否有左手武器
	if (CurrentLeftHandWeapon != nullptr)
	{
		CurrentLeftHandWeapon->Defence(OwnerActor);
	}
}

void USL_EquipmentComponent::ComboSkillBehaviorResponse(AActor* OwnerActor)
{
	// 检查是否有右手武器
	if (CurrentRightHandWeapon != nullptr)
	{
		CurrentRightHandWeapon->ComboSkill(OwnerActor);
	}
	else if (CurrentLeftHandWeapon != nullptr)
	{
		CurrentLeftHandWeapon->ComboSkill(OwnerActor);
	}
	else
	{
		// 双手都未持有武器
	}
}

void USL_EquipmentComponent::ExecuteBehaviorResponse(AActor* OwnerActor)
{
	// 检查是否有右手武器
	if (CurrentRightHandWeapon == nullptr){return;}
	if (CurrentRightHandWeapon.IsLoadExecuteMod())
	{
		CurrentRightHandWeapon.Execute(OwnerActor);
	}
	else
	{
		CurrentRightHandWeapon.Attack(OwnerActor);
	}
}

void USL_EquipmentComponent::BackStabBehaviorResponse(AActor* OwnerActor)
{
	// 检查是否有右手武器
	if (CurrentRightHandWeapon == nullptr) { return; }
	if(CurrentRightHandWeapon.IsLoadBackStabMod())
	{
		CurrentRightHandWeapon.BackStab(OwnerActor);
	}
	else
	{
		CurrentRightHandWeapon.Attack(OwnerActor);
	}
}


void USL_EquipmentComponent::InitEquipmentComponent(const TArray<FWeaponData*> WeaponList, const TArray<int> ItemList)
{
	// 检测武器数组与Item数组是否符合要求
	check(WeaponList.Num() == EQUIPMENT_SLOT_NUM && ItemList.Num() == EQUIPMENT_SLOT_NUM)
	// 执行武器数组初始化
	for (int i = 0; i < EQUIPMENT_SLOT_NUM; i++)
	{
		ASL_WeaponBase* NewWeapon = NewObject<ASL_WeaponBase>(this);
		NewWeapon->InitWeaponInfo(*WeaponList[i]);
		NewWeapon->InActiveWeapon();
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
	if (CurrentEquipmentIndex.Find(ArrowType) == nullptr ||
		CurrentEquipmentIndex.Find(ArrowType) >= EQUIPMENT_SLOT_NUM)
		{return;}
	
	CurrentEquipmentIndex.FindRef(ArrowType) = (CurrentEquipmentIndex.Find(ArrowType)++) % EQUIPMENT_SLOT_NUM;

	switch (ArrowType)
	{
	case EArrowKeyType::ARROWKEY_Up:
	{
		CurrentUpSlotItemID = UpItemList[CurrentEquipmentIndex.Find(ArrowType)];
	}
	break;
	case EArrowKeyType::ARROWKEY_Down:
	{
		CurrentDownSlotItemID = UpItemList[CurrentEquipmentIndex.Find(ArrowType)];
	}
	break;
	case EArrowKeyType::ARROWKEY_Left:
	{
		check(CurrentLeftHandWeapon != nullptr);
		// 设置当前左手武器静默状态
		CurrentLeftHandWeapon->InActiveWeapon();
		CurrentLeftHandWeapon = LeftHandEquipmentInfoList[CurrentEquipmentIndex.Find(ArrowType)];
		check(CurrentLeftHandWeapon != nullptr);
		CurrentLeftHandWeapon->ActiveWeapon();

	}
	break;
	case EArrowKeyType::ARROWKEY_Right:
	{
		check(CurrentRightHandWeapon != nullptr);
		// 设置当前左手武器静默状态
		CurrentRightHandWeapon->InActiveWeapon();
		CurrentRightHandWeapon = RightHandEquipmentInfoList[CurrentEquipmentIndex.Find(ArrowType)];
		check(CurrentRightHandWeapon != nullptr);
		CurrentRightHandWeapon->ActiveWeapon();
	}
	break;
	
	default:break;
	}
}

void USL_EquipmentComponent::SetEquipemntInfo(EArrowKeyType ArrowType, int SlotIndex, FWeaponData* WeaponInfo)
{
	if(SlotIndex < 0 || SlotIndex >= EQUIPMENT_SLOT_NUM){return;}
	// 根据EArrowKeyType区别上下装备槽
	ASL_WeaponBase* NewWeapon = NewObject<ASL_WeaponBase>(this);
	NewWeapon->InitWeaponInfo(*WeaponInfo);
	if (ArrowType == EArrowKeyType::ARROWKEY_Left)
	{
		LeftHandEquipmentInfoList[SlotIndex] = NewWeapon;
	}
	else if (ArrowType == EArrowKeyType::ARROWKEY_Right)
	{
		RightHandEquipmentInfoList[SlotIndex] = NewWeapon;
	}
	else
	{
		// 不正确的调用
	}
}

void USL_EquipmentComponent::SetCostItemInfo(EArrowKeyType ArrowType, int SlotIndex, int ItemID)
{
	if (SlotIndex < 0 || SlotIndex >= EQUIPMENT_SLOT_NUM) { return; }
	// 根据EArrowKeyType区别上下装备槽
	if (ArrowType == EArrowKeyType::ARROWKEY_Up)
	{
		UpItemList[SlotIndex] = ItemID;
	}
	else if (ArrowType == EArrowKeyType::ARROWKEY_Down)
	{
		DownItemList[SlotIndex] = ItemID;
	}
	else
	{
		// 不正确的调用
	}
}

void USL_EquipmentComponent::CleanCostItemInfo(EArrowKeyType ArrowType, int SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= EQUIPMENT_SLOT_NUM) { return; }
	// 根据EArrowKeyType区别上下装备槽
	if (ArrowType == EArrowKeyType::ARROWKEY_Up)
	{
		UpItemList[SlotIndex] = 0;
	}
	else if (ArrowType == EArrowKeyType::ARROWKEY_Down)
	{
		DownItemList[SlotIndex] = 0;
	}
	else
	{
		// 不正确的调用
	}
}

