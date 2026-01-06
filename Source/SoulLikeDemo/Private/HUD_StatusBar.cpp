// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_StatusBar.h"
#include "HUD_StatusIcon.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

UHUD_StatusBar::UHUD_StatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
}

void UHUD_StatusBar::AddStatus(FStatusEffectInfo statusInfo)
{
	// �ظ���Icon������,���ϼ��ܿ�ȷ����������״̬
	RETURN_IF_TRUE(m_statusIconsContainer == nullptr || m_activeStatusIcons.Contains(statusInfo.IconIndex))

	CreateNewStatus(statusInfo);
}

void UHUD_StatusBar::RemoveStatus(int iconIndex)
{
	if (UUserWidget** ppiconWidget = m_activeStatusIcons.Find(iconIndex))
	{
		UUserWidget* iconWidget = *ppiconWidget;

		// ������ͼ�Ƴ�����
		OnStatusIconRemoved(iconWidget);

		// �ӳ�ʵ�����٣��ö�����ʱ�䲥��
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, iconWidget, iconIndex]()
			{
				if (iconWidget && m_statusIconsContainer)
				{
					m_statusIconsContainer->RemoveChild(iconWidget);
				}
				m_activeStatusIcons.Remove(iconIndex);
			}, 0.3f, false); // �ӳ�ʱ��ƥ�䶯������
	}
}

void UHUD_StatusBar::UpdateStatus(FStatusEffectInfo statusInfo)
{
	if (UUserWidget** ppwaitUpdateSubWidget = m_activeStatusIcons.Find(statusInfo.IconIndex))
	{
		if (UUI_IconSlot* statusIcon = Cast<UUI_IconSlot>(*ppwaitUpdateSubWidget))
		{
			// �Ѿ��ҵ�����,�ɽ������ݸ���
			statusIcon->SetData(statusInfo);
		}
	}
}

void UHUD_StatusBar::CreateNewStatus(FStatusEffectInfo statusInfo)
{
	RETURN_IF_TRUE(m_statusIconWidgetClass == nullptr || GetWorld() == nullptr);
	// ����IconWidget
	UUserWidget* newIcon = CreateWidget<UUserWidget>(GetWorld(), m_statusIconWidgetClass);
	if (!newIcon)
	return;
	
	UUI_IconSlot* statusIcon = Cast<UUI_IconSlot>(newIcon);
	if (statusIcon)
	{
		// ��ʼ�����������
		statusIcon->SetData(statusInfo);

		// ��ӵ�����
		m_statusIconsContainer->AddChild(statusIcon);

		// �洢����
		m_activeStatusIcons.Add(statusInfo.IconIndex, newIcon);

		// ������ͼ�����¼�
		OnStatusIconAdded(newIcon);
	}
	
}