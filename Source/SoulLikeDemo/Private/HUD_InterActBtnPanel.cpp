// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_InterActBtnPanel.h"
#include "UI_InterActButton.h"


UHUD_InterActBtnPanel::UHUD_InterActBtnPanel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	InitializeVirtualization(INTERACT_BTN_MAX);
}

void UHUD_InterActBtnPanel::UpdateBatch(const TArray<FInterActOptionInfo>& options)
{
	// 重置已有数据进行更新
	m_interActDataArr.Reset();
	m_interActDataArr = options;
}

void UHUD_InterActBtnPanel::UpdateTarget(const FInterActOptionInfo& options)
{
	// 检索是否有同一索引的数据,若有则更新,若无则添加,此处的数据索引与显示索引一致
	for  (FInterActOptionInfo dataObj : m_interActDataArr)
	{
		if (dataObj.Index == options.Index)
		{
			// 更新数据
			dataObj = options;
		}
	}
	
}

void UHUD_InterActBtnPanel::ClearAllOptions()
{

}

void UHUD_InterActBtnPanel::SetTargetOptionSelected(int32 Index)
{
	
}

void UHUD_InterActBtnPanel::OnButtonClicked(int32 Index)
{
	
}

void UHUD_InterActBtnPanel::SetVisible(bool bVisible)
{
	if (bVisible == true)
	{
		SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
	
}

void UHUD_InterActBtnPanel::UpdateVisibleSlots()
{
	// 确定可视的slot并且将对应的数据覆盖上去

}

void UHUD_InterActBtnPanel::InitializeVirtualization(int32 TotalItemCount)
{
	for (int i = 0; i < TotalItemCount; i++)
	{
		UUI_InterActButton* newInterActBnt = CreateWidget<UUI_InterActButton>(GetWorld(), m_interActBtnClass);
		if (newInterActBnt != nullptr)
		{
			m_slotPool.Push(newInterActBnt);
		}
	}
}

UUI_InterActButton* UHUD_InterActBtnPanel::GetOrCreateSlot()
{
	return m_slotPool.Pop();
}

void UHUD_InterActBtnPanel::ReturnSlotToPool(UUI_InterActButton* atcBtn)
{
	m_slotPool.Push(atcBtn);
	m_visibleSlots.Remove(atcBtn);
}

