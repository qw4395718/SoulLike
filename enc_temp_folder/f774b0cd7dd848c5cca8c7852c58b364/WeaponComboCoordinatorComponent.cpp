// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComboCoordinatorComponent.h"

UWeaponComboCoordinatorComponent::UWeaponComboCoordinatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

float UWeaponComboCoordinatorComponent::GetAttackComboDamage(int type)
{
	// 检测type是否合法
	RETURN_VALUE_IF_FALSE(type>=int(EWeaponModeTyoe::WEAPONMODE_Max) || type<= int(EWeaponModeTyoe::WEAPONMODE_Min),0.0f);
	if (ComboCoordinatorInfoMap.Find(EWeaponModeTyoe(type)))
	{
		FComboCoordinatorInfo Info = ComboCoordinatorInfoMap.FindRef(EWeaponModeTyoe(type));
		if(Info.CurrentComboNum >= Info.ComboDamageMultiplier.Num()){return 0.0f;}
		float Damage = Info.BaseDamage * Info.ComboDamageMultiplier[Info.CurrentComboNum];
		return Damage;
	}
	
	return 0.0f;
}

float UWeaponComboCoordinatorComponent::GetAttackStateCost(int type,int CostType)
{
	// 检测type,costtype是否合法
	RETURN_VALUE_IF_FALSE(type >= int(EWeaponModeTyoe::WEAPONMODE_Max) || type <= int(EWeaponModeTyoe::WEAPONMODE_Min), 0.0f);
	RETURN_VALUE_IF_FALSE(CostType >= int(EWeaponActionCostType::EWeaponAction_Max) || CostType <= int(EWeaponActionCostType::EWeaponAction_Min), 0.0f);

	if (ComboCoordinatorInfoMap.Find(EWeaponModeTyoe(type)))
	{
		FComboCoordinatorInfo Info = ComboCoordinatorInfoMap.FindRef(EWeaponModeTyoe(type));
		if (Info.ComboStateCostMultiplier.Find(Info.CurrentComboNum))
		{
			FAbilityCostInfo AbilityCostInfo = Info.ComboStateCostMultiplier.FindRef(Info.CurrentComboNum);
			float CostValue = 0.0f;
			switch (CostType)
			{
			case int(EWeaponActionCostType::EWeaponAction_Health) :{CostValue = AbilityCostInfo.HealthCost;};break;
			case int(EWeaponActionCostType::EWeaponAction_Stamina) :{CostValue = AbilityCostInfo.StaminaCost;};break;
			case int(EWeaponActionCostType::EWeaponAction_Magic) : {CostValue = AbilityCostInfo.MagicCost;}; break;
			default:break;
			}
			return CostValue;
		}
		
	}

	return 0.0f;
}

void UWeaponComboCoordinatorComponent::InitComboCoordinatorComponet(const TMap<EWeaponModeTyoe, FComboCoordinatorInfo> Map)
{
	ComboCoordinatorInfoMap = Map;
}

void UWeaponComboCoordinatorComponent::ActiveComboWindowInputState(EWeaponModeTyoe type)
{
	// 检测type是否合法
	RETURN_IF_TRUE(type <= EWeaponModeTyoe::WEAPONMODE_Min || type >= EWeaponModeTyoe::WEAPONMODE_Max);
	if (ComboCoordinatorInfoMap.Find(EWeaponModeTyoe(type)))
	{
		FComboCoordinatorInfo Info = ComboCoordinatorInfoMap.FindRef(EWeaponModeTyoe(type));
		if (Info.CurrentComboNum >= Info.ComboDamageMultiplier.Num()) { return; }
		Info.CanContinueCombo = true;
	}
}

void UWeaponComboCoordinatorComponent::InActiveComboWindowInputState(EWeaponModeTyoe type)
{
	// 检测type是否合法
	RETURN_IF_TRUE(type <= EWeaponModeTyoe::WEAPONMODE_Min || type >= EWeaponModeTyoe::WEAPONMODE_Max);
	if (ComboCoordinatorInfoMap.Find(EWeaponModeTyoe(type)))
	{
		FComboCoordinatorInfo Info = ComboCoordinatorInfoMap.FindRef(EWeaponModeTyoe(type));
		if (Info.CurrentComboNum >= Info.ComboDamageMultiplier.Num()) { return; }
		Info.CanContinueCombo = false;
	}
}

int UWeaponComboCoordinatorComponent::GetNextComboNum(EWeaponModeTyoe type)
{
	// 检测type是否合法
	RETURN_VALUE_IF_TRUE(type >= EWeaponModeTyoe::WEAPONMODE_Max || type <= EWeaponModeTyoe::WEAPONMODE_Min, 0.0f);
	if (ComboCoordinatorInfoMap.Find(EWeaponModeTyoe(type)))
	{
		FComboCoordinatorInfo Info = ComboCoordinatorInfoMap.FindRef(EWeaponModeTyoe(type));
		if (Info.CurrentComboNum >= Info.ComboDamageMultiplier.Num()) { return 0; }
		return ((Info.CurrentComboNum+1)% (Info.ComboMaxNum+1));
	}
	return 0;
}

