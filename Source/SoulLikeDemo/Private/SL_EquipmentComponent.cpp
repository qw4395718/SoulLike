#include "SL_EquipmentComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "SL_WeaponBase.h"

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
	RETURN_IF_FALSE(OwnerActor);
	RETURN_IF_FALSE(CurrentRightHandWeapon);
	//检查是否有右手武器
	CurrentRightHandWeapon->PerformWeaponAction(EWeaponModeTyoe::WEAPONMODE_Attack, OwnerActor);
	
}

void USL_EquipmentComponent::DefenceBehaviorResponse(AActor* OwnerActor)
{
	 //判定是否有左手武器
	RETURN_IF_FALSE(OwnerActor);
	RETURN_IF_FALSE(CurrentLeftHandWeapon);
	CurrentLeftHandWeapon->PerformWeaponAction(EWeaponModeTyoe::WEAPONMODE_Defence,OwnerActor);

}

void USL_EquipmentComponent::ComboSkillBehaviorResponse(AActor* OwnerActor)
{
	RETURN_IF_FALSE(OwnerActor);
	if (CurrentRightHandWeapon != nullptr)
	{
		CurrentRightHandWeapon->PerformWeaponAction(EWeaponModeTyoe::WEAPONMODE_ComboSkill, OwnerActor);
	}
	else if (CurrentLeftHandWeapon != nullptr)
	{
		CurrentLeftHandWeapon->PerformWeaponAction(EWeaponModeTyoe::WEAPONMODE_ComboSkill, OwnerActor);
	}
	else
	{ }

}

void USL_EquipmentComponent::ExecuteBehaviorResponse(AActor* OwnerActor)
{
	RETURN_IF_FALSE(OwnerActor);
	RETURN_IF_FALSE(CurrentRightHandWeapon);
	// 检查是否有右手武器
	if (CurrentRightHandWeapon->IsLoadExecuteMod())
	{
		CurrentRightHandWeapon->PerformWeaponAction(EWeaponModeTyoe::WEAPONMODE_Execute,OwnerActor);
	}
	else
	{
		CurrentRightHandWeapon->PerformWeaponAction(EWeaponModeTyoe::WEAPONMODE_Attack,OwnerActor);
	}
}

void USL_EquipmentComponent::BackStabBehaviorResponse(AActor* OwnerActor)
{
	RETURN_IF_FALSE(OwnerActor);
	RETURN_IF_FALSE(CurrentRightHandWeapon);
	// 检查是否有右手武器
	if(CurrentRightHandWeapon->IsLoadBackStabMod())
	{
		CurrentRightHandWeapon->PerformWeaponAction(EWeaponModeTyoe::WEAPONMODE_BackStab,OwnerActor);
	}
	else
	{
		CurrentRightHandWeapon->PerformWeaponAction(EWeaponModeTyoe::WEAPONMODE_Attack,OwnerActor);
	}
}


void USL_EquipmentComponent::InitEquipmentComponent(const TArray<FWeaponData> WeaponList, const TArray<int> ItemList, TMap<EArrowKeyType, int> ActiveSlotIndex,AActor* OwnerActor)
{
	// 检测武器数组与Item数组是否符合要求
	check(WeaponList.Num() == EQUIPMENT_SLOT_NUM*2 && ItemList.Num() == EQUIPMENT_SLOT_NUM*2 && OwnerActor != nullptr)
	// 持有者信息初始化
	/*Owning = OwnerActor;*/
	// 执行武器数组初始化
	for (int i = 0; i < EQUIPMENT_SLOT_NUM*2; i++)
	{
		ASL_WeaponBase* NewWeapon = GetWorld()->SpawnActor<ASL_WeaponBase>(ASL_WeaponBase::StaticClass(),FTransform());
		NewWeapon->InitWeaponInfo(WeaponList[i], OwnerActor);
		NewWeapon->InActiveWeapon();
		if (i < EQUIPMENT_SLOT_NUM)
		{
			LeftHandEquipmentInfoList[i] = NewWeapon;
		}
		else
		{
			RightHandEquipmentInfoList[i- EQUIPMENT_SLOT_NUM] = NewWeapon;
		}
		
	}
	// 执行道具数组初始化
	for (int i = 0; i < EQUIPMENT_SLOT_NUM *2; i++)
	{
		if (i < EQUIPMENT_SLOT_NUM)
		{
			UpItemList[i] = ItemList[i];
		}
		else
		{
			DownItemList[i] = ItemList[i];
		}
	}
	// 记录当前激活槽位索引(深拷贝)
	CurrentEquipmentIndex = ActiveSlotIndex;
	// 根据记录所用激活武器
	if (int* LeftHandIndex = CurrentEquipmentIndex.Find(EArrowKeyType::ARROWKEY_Left))
	{
		if (*LeftHandIndex >= 0 && *LeftHandIndex < EQUIPMENT_SLOT_NUM)
		{
			CurrentLeftHandWeapon = LeftHandEquipmentInfoList[*LeftHandIndex];
			CurrentLeftHandWeapon->ActiveWeapon();
		}
	}
	if (int* RightHandIndex = CurrentEquipmentIndex.Find(EArrowKeyType::ARROWKEY_Right))
	{
		if (*RightHandIndex >= 0 && *RightHandIndex < EQUIPMENT_SLOT_NUM)
		{
			CurrentRightHandWeapon = RightHandEquipmentInfoList[*RightHandIndex];
			CurrentRightHandWeapon->ActiveWeapon();
		}
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
		*(CurrentEquipmentIndex.Find(ArrowType)) >= EQUIPMENT_SLOT_NUM)
		{return;}
	
	*CurrentEquipmentIndex.Find(ArrowType) = ((*CurrentEquipmentIndex.Find(ArrowType))++) % EQUIPMENT_SLOT_NUM;

	switch (ArrowType)
	{
	case EArrowKeyType::ARROWKEY_Up:
	{
		CurrentUpSlotItemID = UpItemList[*CurrentEquipmentIndex.Find(ArrowType)];
	}
	break;
	case EArrowKeyType::ARROWKEY_Down:
	{
		CurrentDownSlotItemID = UpItemList[*CurrentEquipmentIndex.Find(ArrowType)];
	}
	break;
	case EArrowKeyType::ARROWKEY_Left:
	{
		// 设置当前左手武器静默状态
		CurrentLeftHandWeapon->InActiveWeapon();
		CurrentLeftHandWeapon = LeftHandEquipmentInfoList[*CurrentEquipmentIndex.Find(ArrowType)];
		CurrentLeftHandWeapon->ActiveWeapon();

	}
	break;
	case EArrowKeyType::ARROWKEY_Right:
	{
		// 设置当前左手武器静默状态
		CurrentRightHandWeapon->InActiveWeapon();
		CurrentRightHandWeapon = RightHandEquipmentInfoList[*CurrentEquipmentIndex.Find(ArrowType)];
		CurrentRightHandWeapon->ActiveWeapon();
	}
	break;
	
	default:break;
	}
}

void USL_EquipmentComponent::SetEquipemntInfo(EArrowKeyType ArrowType, int SlotIndex, FWeaponData& WeaponInfo)
{
	if(SlotIndex < 0 || SlotIndex >= EQUIPMENT_SLOT_NUM){return;}
	// 根据EArrowKeyType区别上下装备槽
	ASL_WeaponBase* NewWeapon = NewObject<ASL_WeaponBase>(this);
	NewWeapon->InitWeaponInfo(WeaponInfo,GetOwner());
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

ASL_WeaponBase* USL_EquipmentComponent::GetCurrentLHWeapon()
{
	return CurrentLeftHandWeapon;
}

ASL_WeaponBase* USL_EquipmentComponent::GetCurrentRHWeapon()
{
	return CurrentRightHandWeapon;
}

