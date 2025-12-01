// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_InterActBtnPanel.h"

UHUD_InterActBtnPanel::UHUD_InterActBtnPanel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
}

void UHUD_InterActBtnPanel::UpdateBatch(const TArray<FInterActOptionInfo>& options)
{
	// ¼ì²â
}

void UHUD_InterActBtnPanel::UpdateTarget(const FInterActOptionInfo& options)
{

}

void UHUD_InterActBtnPanel::ClearAllOptions()
{
	if (m_interActBtnArr.Num() != 0)
	{
		m_interActBtnArr.Reset();
	}
}

void UHUD_InterActBtnPanel::SetTargetOptionSelected(int32 Index)
{
	
}

void UHUD_InterActBtnPanel::OnButtonClicked(int32 Index)
{

}

void UHUD_InterActBtnPanel::SetVisible(bool bVisible)
{

}

