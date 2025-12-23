// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_InterActBtnPanel.h"
#include "UI_InterActButton.h"
#include "Components/VerticalBox.h"


UHUD_InterActBtnPanel::UHUD_InterActBtnPanel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	TArray<FInterActOptionInfo> test;
	test.Add(FInterActOptionInfo{ 1,nullptr,L"1" });
	test.Add(FInterActOptionInfo{ 2,nullptr,L"2" });
	test.Add(FInterActOptionInfo{ 3,nullptr,L"3" });
	test.Add(FInterActOptionInfo{ 4,nullptr,L"4" });
	test.Add(FInterActOptionInfo{ 5,nullptr,L"5" });
	test.Add(FInterActOptionInfo{ 6,nullptr,L"6" });
	UpdateBatch(test);
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
	int NeedOpertatorSlotNum = m_visibleSlots.Num();
	for (int i = 0; i < NeedOpertatorSlotNum; i++)
	{
		ReturnSlotToPool(m_visibleSlots[0]);
		m_interActBtnsContainer->RemoveChild(m_visibleSlots[0]);
	}
}

void UHUD_InterActBtnPanel::SetTargetOptionSelected(int32 Index)
{
	RETURN_IF_TRUE(Index >= INTERACT_BTN_MAX || Index > m_visibleSlots.Num());
	for (int i = 0; i < m_visibleSlots.Num(); i++)
	{
		m_visibleSlots[i]->SetSelected(false);
	}
	m_visibleSlots[Index]->SetSelected(true);
}

void UHUD_InterActBtnPanel::OnButtonClicked(int32 Index)
{
	SetTargetOptionSelected(Index);
	// 执行选中的逻辑

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
	// 更新显示范围
	if (m_interActDataArr.Num() <= INTERACT_BTN_MAX)
	{
		// 情况1：数据量小于等于最大显示数量
		FirstVisibleIndex = 0;
		LastVisibleIndex = FMath::Max(m_interActDataArr.Num() - 1, 0);
	}
	else if (FirstVisibleIndex + INTERACT_BTN_MAX >= m_interActDataArr.Num())
	{
		// 情况2：当前起始索引+最大显示数超过数组边界
		LastVisibleIndex = m_interActDataArr.Num() - 1;
		FirstVisibleIndex = FMath::Max(LastVisibleIndex - INTERACT_BTN_MAX + 1, 0);
	}
	else
	{
		// 情况3：正常情况，在数组范围内
		LastVisibleIndex = FirstVisibleIndex + INTERACT_BTN_MAX - 1;
		LastVisibleIndex = FMath::Min(LastVisibleIndex, m_interActDataArr.Num() - 1);
	}

	// 检测是否需要从池中获取控件
	if (LastVisibleIndex - FirstVisibleIndex > m_visibleSlots.Num())
	{
		for (int i = 0; i < LastVisibleIndex - FirstVisibleIndex; i++)
		{
			m_visibleSlots.Push(GetOrCreateSlot());
		}
	}

	// 将数据进行更新
	for (int i = 0; i < m_visibleSlots.Num(); i++)
	{
		m_visibleSlots[i]->UpdateInterActBtnInfo(
			m_interActDataArr[FirstVisibleIndex + i].OptionIcon,
			m_interActDataArr[FirstVisibleIndex + i].OptionText
			);
		m_interActBtnsContainer->AddChild(m_visibleSlots[i]);
	}

	
}

void UHUD_InterActBtnPanel::InitializeVirtualization(int32 TotalItemCount)
{
	RETURN_IF_TRUE(m_interActBtnClass == nullptr || GetWorld() == nullptr);
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

