// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_EquipmentBar.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_ProgressBar.h"
#include "HUD_StatusBar.h"
#include "HUD_ItemIcon.h"


UHUD_EquipmentBar::UHUD_EquipmentBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}

void UHUD_EquipmentBar::InitializeEquipmentBar(UHUD_ItemIcon* upEquipment, UHUD_ItemIcon* downEquipment, UHUD_ItemIcon* leftEquipment, UHUD_ItemIcon* rightEquipment, UHUD_ItemIcon* upSecondEquipment, UHUD_ItemIcon* upThirdEquipment, UHUD_ItemIcon* downSecondEquipment, UHUD_ItemIcon* downThirdEquipment)
{
	RETURN_IF_TRUE(upEquipment == nullptr || downEquipment == nullptr || leftEquipment == nullptr || rightEquipment == nullptr || upSecondEquipment == nullptr || upThirdEquipment == nullptr || downSecondEquipment == nullptr || downThirdEquipment == nullptr);
	EquipmentSotMap.Emplace(EHUDEquipmentSlotType::EHUDEquipmentSlotType_Up, upEquipment);
	EquipmentSotMap.Emplace(EHUDEquipmentSlotType::EHUDEquipmentSlotType_UpSecond, upSecondEquipment);
	EquipmentSotMap.Emplace(EHUDEquipmentSlotType::EHUDEquipmentSlotType_UpThird, upThirdEquipment);
	EquipmentSotMap.Emplace(EHUDEquipmentSlotType::EHUDEquipmentSlotType_Down, downEquipment);
	EquipmentSotMap.Emplace(EHUDEquipmentSlotType::EHUDEquipmentSlotType_DownSecond, downSecondEquipment);
	EquipmentSotMap.Emplace(EHUDEquipmentSlotType::EHUDEquipmentSlotType_DownThird, downThirdEquipment);
	EquipmentSotMap.Emplace(EHUDEquipmentSlotType::EHUDEquipmentSlotType_Left, leftEquipment);
	EquipmentSotMap.Emplace(EHUDEquipmentSlotType::EHUDEquipmentSlotType_Right, rightEquipment);

}

void UHUD_EquipmentBar::UpdateTargetSlot(int type, FStatusEffectInfo status)
{
	RETURN_IF_TRUE(type >= int(EHUDEquipmentSlotType::EHUDEquipmentSlotType_Max) || type <= int(EHUDEquipmentSlotType::EHUDEquipmentSlotType_None));
	UHUD_ItemIcon* targetSlot = EquipmentSotMap.FindRef(EHUDEquipmentSlotType(type));
	if (targetSlot)
	{
		targetSlot->SetData(status);
	}
	
}

