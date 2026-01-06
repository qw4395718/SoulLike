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
	// �����������ݽ��и���
	m_interActDataArr.Reset();
	m_interActDataArr = options;
}

void UHUD_InterActBtnPanel::UpdateTarget(const FInterActOptionInfo& options)
{
	// �����Ƿ���ͬһ����������,���������,���������,�˴���������������ʾ����һ��
	for  (FInterActOptionInfo dataObj : m_interActDataArr)
	{
		if (dataObj.Index == options.Index)
		{
			// ��������
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
	// ִ��ѡ�е��߼�

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
	// ��ȡ�����������������Ϲ����������¹���
	float WheelDelta = InMouseEvent.GetWheelDelta();

	// ��������߼�
	HandleScroll(WheelDelta);

	// ����¼��Ѵ���
	return FReply::Handled();
}

void UHUD_InterActBtnPanel::HandleScroll(float wheelDelta)
{
	// �����µĹ�������,��Ҫ��ת����
	float newScrollOffset = currentScrollOffset + (wheelDelta * scrollSensitivity *(-1));

	// ���ƹ�����Χ
	float maxScrollOffset = (m_interActDataArr.Num() - INTERACT_BTN_MAX) * INTERACT_BTN_HEIGHT;
	newScrollOffset = FMath::Clamp(newScrollOffset,0.0f, maxScrollOffset);

	// ���¹���
	if (newScrollOffset != currentScrollOffset)
	{
		currentScrollOffset = newScrollOffset;
		UpdateVisibleRange();
	}

}

void UHUD_InterActBtnPanel::UpdateVisibleRange()
{
	// ��ȡScrollBox�ļ�����Ϣ
	//FGeometry scrollGeometry = GetCachedGeometry();

	//// ��ȡ��������Ĵ�С
	//float viewHeight = scrollGeometry.GetLocalSize().Y;

	// ��ȡÿ����λ�ı�׼�߶�
	float slotHeight = GetSlotHeight();

	float viewHeight = GetSlotHeight() * INTERACT_BTN_MAX;

	// ����ɼ���Χ
	// ����ȡ��ȷ����ȫ�ɼ��Ĳ�λ
	int32 newFirstVisibleIndex = FMath::FloorToInt(currentScrollOffset/slotHeight);

	// ����ȡ��ȷ���������в��ֿɼ��Ĳ�λ
	int32 newLastVisibleIndex = FMath::CeilToInt((currentScrollOffset + viewHeight)/slotHeight) - 1;

	// ���Ʒ�Χ����Ч������
	newFirstVisibleIndex = FMath::Clamp(newFirstVisibleIndex,0, m_interActDataArr.Num()-1);
	newLastVisibleIndex = FMath::Clamp(newLastVisibleIndex,0, m_interActDataArr.Num()-1);
	if (newLastVisibleIndex - newFirstVisibleIndex + 1 < INTERACT_BTN_MAX)
	{// ��ʾ����ȫ
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
	{// ��ʾ���ݳ�������
		newLastVisibleIndex = newFirstVisibleIndex + INTERACT_BTN_MAX -1;
	}
	else{/*�������*/}

	// �����Χû�з����仯,�������
	if (newFirstVisibleIndex == FirstVisibleIndex &&
		newLastVisibleIndex == LastVisibleIndex)
	{
		return;
	}

	// ���¿ɼ���Χ
	int32 oldFirstVisibleIndex = FirstVisibleIndex;
	int32 oldLastVisibleIndex = LastVisibleIndex;
	FirstVisibleIndex = newFirstVisibleIndex;
	LastVisibleIndex = newLastVisibleIndex;

	UE_LOG(LogTemp, Log, TEXT("ZYF_UHUD_InterActBtnPanel::UpdateVisibleRange() oldFirst:%d oldLast%d newFirst%d newLast%d"),
		oldFirstVisibleIndex, oldLastVisibleIndex, FirstVisibleIndex, LastVisibleIndex);

	// �����¾ɷ�Χ������¿ɼ���λ
	UpdateVisibleSlots(oldFirstVisibleIndex, oldLastVisibleIndex);
}

void UHUD_InterActBtnPanel::UpdateSlotPositions()
{
	float SlotHeight = GetSlotHeight();

	for (UUI_InterActButton* interActSlot : m_visibleSlots)
	{
		int32 ItemIndex = interActSlot->GetAssignedIndex();

		// �����λ��Yλ�ã����λ�ã�
		float SlotY = (ItemIndex * SlotHeight) - currentScrollOffset;

		// ���ò�λλ��
		interActSlot->SetRenderTranslation(FVector2D(0.0f, SlotY));
	}
}

void UHUD_InterActBtnPanel::AddSlotForIndex(int32 Index)
{
	UUI_InterActButton* interActSlot = GetOrCreateSlot();

	// ���ò�λ����
	const FInterActOptionInfo* pItem = GetItemAtIndex(Index);
	if(pItem == nullptr){return;}
	interActSlot->UpdateInterActBtnInfo(Index, pItem->OptionIcon, pItem->OptionText);

	// ��ӵ��ɼ��б�
	m_visibleSlots.Add(interActSlot);

	// ��ӵ�ScrollBox��Ϊ�����
	//AddChild(interActSlot);
}

void UHUD_InterActBtnPanel::AddSlotAtIndexZero(int32 Index)
{
	UUI_InterActButton* interActSlot = GetOrCreateSlot();

	// ���ò�λ����
	const FInterActOptionInfo* pItem = GetItemAtIndex(Index);
	if (pItem == nullptr) { return; }
	interActSlot->UpdateInterActBtnInfo(Index, pItem->OptionIcon, pItem->OptionText);

	// ��ӵ��ɼ��б�
	m_visibleSlots.Insert(interActSlot,0);

	// ��ӵ�ScrollBox��Ϊ�����
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
	// ���2: ��ȫ���·�Χ(����ת���߿��ٹ���)
	if (oldLastIndex < FirstVisibleIndex || 
	oldFirstIndex > LastVisibleIndex || 
	(oldFirstIndex == 0 && oldLastIndex == 0))
	{
		// �������еľɲ�λ
		for (int32 i = 0;i < m_visibleSlots.Num();i++)
		{
			ReturnSlotToPool(m_visibleSlots[i]);
		}
		m_visibleSlots.Empty();

		// �����·�Χ�ڵ����в�λ
		for (int32 i = FirstVisibleIndex; i <= LastVisibleIndex; i++)
		{
			AddSlotForIndex(i);
		}
	}

	// ���3: ������������(��ʾ�����µ�����)
	else if (FirstVisibleIndex > oldFirstIndex)
	{
		for (int32 i = 0;i < m_visibleSlots.Num();)
		{
			UUI_InterActButton* slot = m_visibleSlots[i];
			int32 itemIndex = slot->GetAssignedIndex();

			if (itemIndex < FirstVisibleIndex)
			{
				// ����
				ReturnSlotToPool(slot);
				//m_visibleSlots.RemoveAt(i);
			}
			else
			{
				i++;
			}
		}

		// ����¼�����Ұ���²���λ
		for (int32 i = oldLastIndex + 1;i <= LastVisibleIndex; i++)
		{
			AddSlotForIndex(i);
		}
	}

	// ���4 :������������(��ʾ�����ϵ�����)
	else if (LastVisibleIndex < oldLastIndex)
	{
		for (int32 i = 0; i < m_visibleSlots.Num();)
		{
			UUI_InterActButton* slot = m_visibleSlots[i];
			int32 itemIndex = slot->GetAssignedIndex();

			if (itemIndex > LastVisibleIndex)
			{
				// ����
				ReturnSlotToPool(slot);
				//m_visibleSlots.RemoveAt(i);
			}
			else
			{
				i++;
			}
		}

		// ����¼�����Ұ���ϲ���λ
		for (int32 i = oldFirstIndex - 1; i >= FirstVisibleIndex; i--)
		{
			AddSlotAtIndexZero(i);
		}
	}

	// �������
	for (int32 i= 0;i < m_interActBtnsContainer->GetChildrenCount();i++)
	{
		m_interActBtnsContainer->RemoveChildAt(i);
	}

	// ����������򻺴��λ
	for (UUI_InterActButton* interActSlot : m_visibleSlots)
	{
		m_interActBtnsContainer->AddChild(interActSlot);
	}

	// �������пɼ���λ��λ��
	//UpdateSlotPositions();
}

