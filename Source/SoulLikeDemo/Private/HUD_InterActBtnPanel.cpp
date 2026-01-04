// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_InterActBtnPanel.h"
#include "UI_InterActButton.h"
#include "Components/VerticalBox.h"


UHUD_InterActBtnPanel::UHUD_InterActBtnPanel(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	FakeInit();
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

void UHUD_InterActBtnPanel::FakeInit()
{
	TArray<FInterActOptionInfo> test;
	test.Add(FInterActOptionInfo{ 1,nullptr,L"1" });
	test.Add(FInterActOptionInfo{ 2,nullptr,L"2" });
	test.Add(FInterActOptionInfo{ 3,nullptr,L"3" });
	test.Add(FInterActOptionInfo{ 4,nullptr,L"4" });
	test.Add(FInterActOptionInfo{ 5,nullptr,L"5" });
	test.Add(FInterActOptionInfo{ 6,nullptr,L"6" });
	test.Add(FInterActOptionInfo{ 7,nullptr,L"7" });
	test.Add(FInterActOptionInfo{ 8,nullptr,L"8" });
	test.Add(FInterActOptionInfo{ 9,nullptr,L"9" });
	test.Add(FInterActOptionInfo{ 10,nullptr,L"10" });
	test.Add(FInterActOptionInfo{ 11,nullptr,L"11" });
	test.Add(FInterActOptionInfo{ 12,nullptr,L"12" });
	UpdateBatch(test);
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
	float maxScrollOffset = (m_interActDataArr.Num() - INTERACT_BTN_MAX) * INTERACT_BTN_HEIGHT;
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
	//FGeometry scrollGeometry = GetCachedGeometry();

	//// 获取可视区域的大小
	//float viewHeight = scrollGeometry.GetLocalSize().Y;

	// 获取每个槽位的标准高度
	float slotHeight = GetSlotHeight();

	float viewHeight = GetSlotHeight() * INTERACT_BTN_MAX;

	// 计算可见范围
	// 向下取整确保完全可见的槽位
	int32 newFirstVisibleIndex = FMath::FloorToInt(currentScrollOffset/slotHeight);

	// 向上取整确保覆盖所有部分可见的槽位
	int32 newLastVisibleIndex = FMath::CeilToInt((currentScrollOffset + viewHeight)/slotHeight) - 1;

	// 限制范围在有效数量内
	newFirstVisibleIndex = FMath::Clamp(newFirstVisibleIndex,0, m_interActDataArr.Num()-1);
	newLastVisibleIndex = FMath::Clamp(newLastVisibleIndex,0, m_interActDataArr.Num()-1);
	if (newLastVisibleIndex - newFirstVisibleIndex + 1 < INTERACT_BTN_MAX)
	{// 显示不完全
		if (m_interActDataArr.Num() <= INTERACT_BTN_MAX)
		{
			newFirstVisibleIndex = 0;
			newLastVisibleIndex = m_interActDataArr.Num() -1;
		}
		else if(newLastVisibleIndex >= m_interActDataArr.Num() - 1)
		{
			newFirstVisibleIndex = newLastVisibleIndex - INTERACT_BTN_MAX + 1;
		}
		else
		{
			newFirstVisibleIndex = 0;
			newLastVisibleIndex = INTERACT_BTN_MAX -1;
		}
	}
	else if (newLastVisibleIndex - newFirstVisibleIndex + 1 > INTERACT_BTN_MAX)
	{// 显示数据超过限制
		newLastVisibleIndex = newFirstVisibleIndex + INTERACT_BTN_MAX -1;
	}
	else{/*正常情况*/}

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

	UE_LOG(LogTemp, Log, TEXT("ZYF_UHUD_InterActBtnPanel::UpdateVisibleRange() oldFirst:%d oldLast%d newFirst%d newLast%d"),
		oldFirstVisibleIndex, oldLastVisibleIndex, FirstVisibleIndex, LastVisibleIndex);

	// 根据新旧范围差异更新可见槽位
	UpdateVisibleSlots(oldFirstVisibleIndex, oldLastVisibleIndex);
}

void UHUD_InterActBtnPanel::UpdateSlotPositions()
{
	float SlotHeight = GetSlotHeight();

	for (UUI_InterActButton* interActSlot : m_visibleSlots)
	{
		int32 ItemIndex = interActSlot->GetAssignedIndex();

		// 计算槽位的Y位置（相对位置）
		float SlotY = (ItemIndex * SlotHeight) - currentScrollOffset;

		// 设置槽位位置
		interActSlot->SetRenderTranslation(FVector2D(0.0f, SlotY));
	}
}

void UHUD_InterActBtnPanel::AddSlotForIndex(int32 Index)
{
	UUI_InterActButton* interActSlot = GetOrCreateSlot();

	// 配置槽位数据
	const FInterActOptionInfo* pItem = GetItemAtIndex(Index);
	if(pItem == nullptr){return;}
	interActSlot->UpdateInterActBtnInfo(Index, pItem->OptionIcon, pItem->OptionText);

	// 添加到可见列表
	m_visibleSlots.Add(interActSlot);

	// 添加到ScrollBox作为子组件
	//AddChild(interActSlot);
}

void UHUD_InterActBtnPanel::AddSlotAtIndexZero(int32 Index)
{
	UUI_InterActButton* interActSlot = GetOrCreateSlot();

	// 配置槽位数据
	const FInterActOptionInfo* pItem = GetItemAtIndex(Index);
	if (pItem == nullptr) { return; }
	interActSlot->UpdateInterActBtnInfo(Index, pItem->OptionIcon, pItem->OptionText);

	// 添加到可见列表
	m_visibleSlots.Insert(interActSlot,0);

	// 添加到ScrollBox作为子组件
	//AddChild(interActSlot);
}

const FInterActOptionInfo* UHUD_InterActBtnPanel::GetItemAtIndex(int32 Index)
{
	RETURN_VALUE_IF_TRUE(Index >= m_interActDataArr.Num(),nullptr);
	return &m_interActDataArr[Index];
}

float UHUD_InterActBtnPanel::GetSlotHeight()
{
	return INTERACT_BTN_HEIGHT;
}

void UHUD_InterActBtnPanel::UpdateVisibleSlots(int32 oldFirstIndex, int32 oldLastIndex)
{
	// 情况2: 完全的新范围(如跳转或者快速滚动)
	if (oldLastIndex < FirstVisibleIndex || 
	oldFirstIndex > LastVisibleIndex || 
	(oldFirstIndex == 0 && oldLastIndex == 0))
	{
		// 回收所有的旧槽位
		for (int32 i = 0;i < m_visibleSlots.Num();i++)
		{
			ReturnSlotToPool(m_visibleSlots[i]);
		}
		m_visibleSlots.Empty();

		// 创建新范围内的所有槽位
		for (int32 i = FirstVisibleIndex; i <= LastVisibleIndex; i++)
		{
			AddSlotForIndex(i);
		}
	}

	// 情况3: 滚动方向向下(显示更靠下的内容)
	else if (FirstVisibleIndex > oldFirstIndex)
	{
		for (int32 i = 0;i < m_visibleSlots.Num();)
		{
			UUI_InterActButton* slot = m_visibleSlots[i];
			int32 itemIndex = slot->GetAssignedIndex();

			if (itemIndex < FirstVisibleIndex)
			{
				// 回收
				ReturnSlotToPool(slot);
				//m_visibleSlots.RemoveAt(i);
			}
			else
			{
				i++;
			}
		}

		// 添加新加入视野的下部槽位
		for (int32 i = oldLastIndex + 1;i <= LastVisibleIndex; i++)
		{
			AddSlotForIndex(i);
		}
	}

	// 情况4 :滚动方向向上(显示更靠上的内容)
	else if (LastVisibleIndex < oldLastIndex)
	{
		for (int32 i = 0; i < m_visibleSlots.Num();)
		{
			UUI_InterActButton* slot = m_visibleSlots[i];
			int32 itemIndex = slot->GetAssignedIndex();

			if (itemIndex > LastVisibleIndex)
			{
				// 回收
				ReturnSlotToPool(slot);
				//m_visibleSlots.RemoveAt(i);
			}
			else
			{
				i++;
			}
		}

		// 添加新加入视野的上部槽位
		for (int32 i = oldFirstIndex - 1; i >= FirstVisibleIndex; i--)
		{
			AddSlotAtIndexZero(i);
		}
	}

	// 清空现有
	for (int32 i= 0;i < m_interActBtnsContainer->GetChildrenCount();i++)
	{
		m_interActBtnsContainer->RemoveChildAt(i);
	}

	// 填入可视区域缓存槽位
	for (UUI_InterActButton* interActSlot : m_visibleSlots)
	{
		m_interActBtnsContainer->AddChild(interActSlot);
	}

	// 更新所有可见槽位的位置
	//UpdateSlotPositions();
}

