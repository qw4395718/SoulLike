// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_EquipmentBar.h"
#include "HUD_ItemIcon.h"


UHUD_EquipmentBar::UHUD_EquipmentBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{

}


void UHUD_EquipmentBar::InitializeEquipmentBar()
{
	RETURN_IF_TRUE(m_upSlot == nullptr || m_downSlot == nullptr || m_leftSlot == nullptr || m_rightSlot == nullptr);
	EquipmentSotMap.Empty();
	EquipmentSotMap.Add(EHUDEquipmentSlotType::EHUDEquipmentSlotType_Up, m_upSlot);
	EquipmentSotMap.Add(EHUDEquipmentSlotType::EHUDEquipmentSlotType_Down, m_downSlot);
	EquipmentSotMap.Add(EHUDEquipmentSlotType::EHUDEquipmentSlotType_Left, m_leftSlot);
	EquipmentSotMap.Add(EHUDEquipmentSlotType::EHUDEquipmentSlotType_Right, m_rightSlot);
}

void UHUD_EquipmentBar::UpdateTargetSlot(EHUDEquipmentSlotType type, FStatusEffectInfo status)
{
	if (EquipmentSotMap.Contains(type))
	{
		UHUD_ItemIcon* targetSlot = EquipmentSotMap.FindRef(type);
		RETURN_IF_TRUE(targetSlot == nullptr);
		targetSlot->SetData(status);
	}
}

void UHUD_EquipmentBar::StartChangeEquipment(UWidgetAnimation* Anim)
{
	if (Anim)
	{
		PlayAnimation(Anim);
	}
}

FReply UHUD_EquipmentBar::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 
	float WheelDelta = InMouseEvent.GetWheelDelta();

	// 
	HandleScroll(WheelDelta);

	// 
	return FReply::Handled();
}



void UHUD_EquipmentBar::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 检测是否按下了Ctrl键（左Ctrl或右Ctrl）

	if (APlayerController* PC = GetOwningPlayer())
	{
		m_bIsCtrlHeldInternal = PC->IsInputKeyDown(EKeys::LeftControl) ||
			PC->IsInputKeyDown(EKeys::RightControl);
	}
}

void UHUD_EquipmentBar::HandleScroll(float wheelDelta)
{
	// 
	float newScrollOffset = (wheelDelta * scrollSensitivity * (-1));

	// change -> index 
	int changeNum = FMath::CeilToInt( newScrollOffset/ scrollHeightLimit);
	if (changeNum != 0)
	{
		// 多播委托到管理类中
		UE_LOG(LogTemp,Display,TEXT("UHUD_EquipmentBar::HandleScroll changeNum:%d isActiveLeftCtrl:%d"), changeNum, m_bIsCtrlHeldInternal);

	}
}
