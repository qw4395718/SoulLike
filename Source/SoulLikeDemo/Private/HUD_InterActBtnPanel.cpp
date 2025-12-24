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

//void UHUD_InterActBtnPanel::UpdateVisibleSlots(int FirstVisibleIndex,int LastVisibleIndex)
//{
//	// 更新显示范围
//	if (m_interActDataArr.Num() <= INTERACT_BTN_MAX)
//	{
//		// 情况1：数据量小于等于最大显示数量
//		FirstVisibleIndex = 0;
//		LastVisibleIndex = FMath::Max(m_interActDataArr.Num() - 1, 0);
//	}
//	else if (FirstVisibleIndex + INTERACT_BTN_MAX >= m_interActDataArr.Num())
//	{
//		// 情况2：当前起始索引+最大显示数超过数组边界
//		LastVisibleIndex = m_interActDataArr.Num() - 1;
//		FirstVisibleIndex = FMath::Max(LastVisibleIndex - INTERACT_BTN_MAX + 1, 0);
//	}
//	else
//	{
//		// 情况3：正常情况，在数组范围内
//		LastVisibleIndex = FirstVisibleIndex + INTERACT_BTN_MAX - 1;
//		LastVisibleIndex = FMath::Min(LastVisibleIndex, m_interActDataArr.Num() - 1);
//	}
//
//	// 检测是否需要从池中获取控件
//	if (LastVisibleIndex - FirstVisibleIndex > m_visibleSlots.Num())
//	{
//		for (int i = 0; i <= LastVisibleIndex - FirstVisibleIndex; i++)
//		{
//			m_visibleSlots.Push(GetOrCreateSlot());
//		}
//	}
//
//	// 将数据进行更新
//	for (int i = 0; i < m_visibleSlots.Num(); i++)
//	{
//		m_visibleSlots[i]->UpdateInterActBtnInfo(
//			m_interActDataArr[FirstVisibleIndex + i].OptionIcon,
//			m_interActDataArr[FirstVisibleIndex + i].OptionText
//			);
//		m_interActBtnsContainer->AddChild(m_visibleSlots[i]);
//	}
//
//	
//}

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

FReply UHUD_InterActBtnPanel::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 获取滚轮增量（正数向上滚，负数向下滚）
	float WheelDelta = InMouseEvent.GetWheelDelta();

	// 处理滚动逻辑
	HandleScroll(WheelDelta);

	// 标记事件已处理
	return FReply::Handled();
}

void UHUD_InterActBtnPanel::HandleScroll(float wheelDelta)
{
	// 计算新的滚动便宜,需要反转方向
	float newScrollOffset = currentScrollOffset + (wheelDelta * scrollSensitivity *(-1));

	// 限制滚动范围
	float maxScrollOffset = INTERACT_BTN_MAX * INTERACT_BTN_HEIGHT;
	newScrollOffset = FMath::Clamp(newScrollOffset,0.0f, maxScrollOffset);

	// 更新滚动
	if (newScrollOffset != currentScrollOffset)
	{
		currentScrollOffset = newScrollOffset;
		UpdateVisibleRange();
	}

}

void UHUD_InterActBtnPanel::UpdateVisibleRange()
{
	// 获取ScrollBox的几何信息
	FGeometry scrollGeometry = GetCachedGeometry();

	// 获取可视区域的大小
	float viewHeight = scrollGeometry.GetLocalSize().Y;

	// 获取每个槽位的标准高度
	float slotHeight = INTERACT_BTN_HEIGHT;

	// 计算可见范围
	// 向下取整确保完全可见的槽位
	int32 newFirstVisibleIndex = FMath::FloorToInt(currentScrollOffset/slotHeight);

	// 向上取整确保覆盖所有部分可见的槽位
	int32 newLastVisibleIndex = FMath::CeilToInt((currentScrollOffset + viewHeight)/slotHeight);

	// 限制范围在有效数量内
	newFirstVisibleIndex = FMath::Clamp(newFirstVisibleIndex,0, m_interActDataArr.Num()-1);
	newLastVisibleIndex = FMath::Clamp(newLastVisibleIndex,0, m_interActDataArr.Num()-1);

	// 如果范围没有发生变化,无需更新
	if (newFirstVisibleIndex == FirstVisibleIndex &&
		newLastVisibleIndex == LastVisibleIndex)
	{
		return;
	}

	// 更新可见范围
	int32 oldFirstVisibleIndex = FirstVisibleIndex;
	int32 oldLastVisibleIndex = LastVisibleIndex;
	FirstVisibleIndex = newFirstVisibleIndex;
	LastVisibleIndex = newLastVisibleIndex;

	// 根据新旧范围差异更新可见槽位
	UpdateVisibleSlots(oldFirstVisibleIndex, oldLastVisibleIndex);
}

void UHUD_InterActBtnPanel::UpdateVisibleSlots(int32 oldFirstIndex, int32 oldLastIndex)
{
	// 情况1: 完全的新范围(如跳转或者快速滚动)
	if (oldLastIndex < FirstVisibleIndex || oldFirstIndex > LastVisibleIndex)
	{
		// 回收所有的旧槽位
		for (int32 i = 0;i < m_visibleSlots.Num();i++)
		{
			ReturnSlotToPool(m_visibleSlots[i]);
		}
		m_visibleSlots.Empty();

		// 创建新范围内的所有槽位

	}

	// 情况2
}

