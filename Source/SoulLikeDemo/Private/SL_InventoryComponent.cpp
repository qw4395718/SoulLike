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
	// 后续完成读表的初始化
	EquipWeaponInfoList.Reset();
	for(int WeaponID : WeaponList)
	{
		FWeaponData UnitTestData;
		UnitTestData.WeaponID = WeaponID;
		UnitTestData.WeaponUniqueID = WeaponID * 100 + 1;
		UnitTestData.Mesh = TEXT("/Game/InfinityBladeWeapons/Weapons/Blade/Swords/Blade_BlackKnight/SK_Blade_BlackKnight.SK_Blade_BlackKnight");
		UnitTestData.SocketName = TEXT("ik_hand_r");
		UnitTestData.AnimClass = TEXT("");
		UnitTestData.AttackMentageName = TEXT("/Game/SoulLikeDemo/Anim/AM_Attack_Sword.AM_Attack_Sword");
		UnitTestData.ComboSkillMentageName = TEXT("/Game/SoulLikeDemo/Anim/AM_CombatSkill_Sheild.AM_CombatSkill_Sheild");
		UnitTestData.ExecuteMentageName = TEXT("/Game/SoulLikeDemo/Anim/AM_Execute_Sword.AM_Execute_Sword");
		UnitTestData.BackStabMentageName = TEXT("/Game/SoulLikeDemo/Anim/AM_BackStab_Sword.AM_BackStab_Sword");
		UnitTestData.WeaponCollisionBoxSize.X = 100.0f;
		UnitTestData.WeaponCollisionBoxSize.Y = 5.0f;
		UnitTestData.WeaponCollisionBoxSize.Z = 5.0f;
		// 武器数据信息
		FAbilityCostInfo AbilityCostInfo;
		AbilityCostInfo.HealthCost = 20.0f;
		AbilityCostInfo.StaminaCost = 20.0f;
		AbilityCostInfo.MagicCost = 20.0f;
		FComboCoordinatorInfo ComboCoordinatorInfo;
		ComboCoordinatorInfo.BaseDamage = 20.0f;
		ComboCoordinatorInfo.ComboMaxNum = 3;
		ComboCoordinatorInfo.CurrentComboNum = 0;
		ComboCoordinatorInfo.CanContinueCombo = false;
		for (int i = 0; i <= ComboCoordinatorInfo.ComboMaxNum; i++)
		{
			ComboCoordinatorInfo.ComboDamageMultiplier.Add(i+1);
			ComboCoordinatorInfo.ComboStateCostMultiplier.Add(i, AbilityCostInfo);
		}
		UnitTestData.ComboCoordinatorInfoMap.Reset();
		UnitTestData.ComboCoordinatorInfoMap.Add(EWeaponModeTyoe::WEAPONMODE_Attack, ComboCoordinatorInfo);
		UnitTestData.ComboCoordinatorInfoMap.Add(EWeaponModeTyoe::WEAPONMODE_Defence, ComboCoordinatorInfo);
		UnitTestData.ComboCoordinatorInfoMap.Add(EWeaponModeTyoe::WEAPONMODE_ComboSkill, ComboCoordinatorInfo);
		UnitTestData.ComboCoordinatorInfoMap.Add(EWeaponModeTyoe::WEAPONMODE_BackStab, ComboCoordinatorInfo);
		UnitTestData.ComboCoordinatorInfoMap.Add(EWeaponModeTyoe::WEAPONMODE_Execute, ComboCoordinatorInfo);

		UnitTestData.NeedLoadComponentInfoMap.Add(EWeaponComponentType::MeleeAttack, true);
		UnitTestData.NeedLoadComponentInfoMap.Add(EWeaponComponentType::Parry, true);
		UnitTestData.NeedLoadComponentInfoMap.Add(EWeaponComponentType::Execute, true);
		UnitTestData.NeedLoadComponentInfoMap.Add(EWeaponComponentType::BackStab, true);

		EquipWeaponInfoList.Add(UnitTestData);
	}

	
}
