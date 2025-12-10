// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_InterActBtnPanel.h"

UHUD_InterActBtnPanel::UHUD_InterActBtnPanel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
}

void UHUD_InterActBtnPanel::UpdateBatch(const TArray<FInterActOptionInfo>& options)
{
	// 重置已有数据进行更新
	m_interActDataArr.Reset();
	m_interActDataArr = options;
}

void UHUD_InterActBtnPanel::UpdateTarget(const FInterActOptionInfo& options)
{
	// 检索是否有同一索引的数据,若有则更新,若无则添加
	if()
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

