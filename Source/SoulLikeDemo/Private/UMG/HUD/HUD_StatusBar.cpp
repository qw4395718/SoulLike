// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_StatusBar.h"
#include "HUD_StatusIcon.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

UHUD_StatusBar::UHUD_StatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
}

void UHUD_StatusBar::AddStatus_Implementation(FStatusEffectInfo statusInfo)
{
	// 
	RETURN_IF_TRUE(m_statusIconsContainer == nullptr || m_activeStatusIcons.Contains(statusInfo.IconIndex))

	CreateNewStatus(statusInfo);
}

void UHUD_StatusBar::RemoveStatus_Implementation(int iconIndex)
{
	if (UUserWidget** ppiconWidget = m_activeStatusIcons.Find(iconIndex))
	{
		UUserWidget* iconWidget = *ppiconWidget;

		// 
		OnStatusIconRemoved(iconWidget);

		// 
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, iconWidget, iconIndex]()
			{
				if (iconWidget && m_statusIconsContainer)
				{
					m_statusIconsContainer->RemoveChild(iconWidget);
				}
				m_activeStatusIcons.Remove(iconIndex);
			}, 0.3f, false); //
	}
}

void UHUD_StatusBar::UpdateStatus_Implementation(FStatusEffectInfo statusInfo)
{
	if (UUserWidget** ppwaitUpdateSubWidget = m_activeStatusIcons.Find(statusInfo.IconIndex))
	{
		if (UUI_IconSlot* statusIcon = Cast<UUI_IconSlot>(*ppwaitUpdateSubWidget))
		{
			// 
			statusIcon->SetData(statusInfo);
		}
	}
}

void UHUD_StatusBar::CreateNewStatus(FStatusEffectInfo statusInfo)
{
	RETURN_IF_TRUE(m_statusIconWidgetClass == nullptr || GetWorld() == nullptr);
	// 
	UUserWidget* newIcon = CreateWidget<UUserWidget>(GetWorld(), m_statusIconWidgetClass);
	if (!newIcon)
	return;
	
	UUI_IconSlot* statusIcon = Cast<UUI_IconSlot>(newIcon);
	if (statusIcon)
	{
		// 
		statusIcon->SetData(statusInfo);

		// 
		m_statusIconsContainer->AddChild(statusIcon);

		// 
		m_activeStatusIcons.Add(statusInfo.IconIndex, newIcon);

		// 
		OnStatusIconAdded(newIcon);
	}
	
}