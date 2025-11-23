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
	// 重复的Icon不接受,由上级管控确保传入是新状态
	RETURN_IF_TRUE(m_statusIconsContainer == nullptr || m_activeStatusIcons.Contains(statusInfo.IconIndex))

	CreateNewStatus(statusInfo);
}

void UHUD_StatusBar::RemoveStatus(int iconIndex)
{
	if (UUserWidget** ppiconWidget = m_activeStatusIcons.Find(iconIndex))
	{
		UUserWidget* iconWidget = *ppiconWidget;

		// 调用蓝图移除动画
		OnStatusIconRemoved(iconWidget);

		// 延迟实际销毁，让动画有时间播放
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, iconWidget, iconIndex]()
			{
				if (iconWidget && m_statusIconsContainer)
				{
					m_statusIconsContainer->RemoveChild(iconWidget);
				}
				m_activeStatusIcons.Remove(iconIndex);
			}, 0.3f, false); // 延迟时间匹配动画长度
	}
}

void UHUD_StatusBar::UpdateStatus(FStatusEffectInfo statusInfo)
{
	if (UUserWidget** ppwaitUpdateSubWidget = m_activeStatusIcons.Find(statusInfo.IconIndex))
	{
		if (UUI_IconSlot* statusIcon = Cast<UUI_IconSlot>(*ppwaitUpdateSubWidget))
		{
			// 已经找到数据,可进行数据更新
			statusIcon->SetData(statusInfo);
		}
	}
}

void UHUD_StatusBar::CreateNewStatus(FStatusEffectInfo statusInfo)
{
	RETURN_IF_TRUE(m_statusIconWidgetClass == nullptr || GetWorld() == nullptr);
	// 创建IconWidget
	UUserWidget* newIcon = CreateWidget<UUserWidget>(GetWorld(), m_statusIconWidgetClass);
	if (!newIcon)
	return;
	
	UUI_IconSlot* statusIcon = Cast<UUI_IconSlot>(newIcon);
	if (statusIcon)
	{
		// 初始化后纳入管理
		statusIcon->SetData(statusInfo);

		// 添加到容器
		m_statusIconsContainer->AddChild(statusIcon);

		// 存储引用
		m_activeStatusIcons.Add(statusInfo.IconIndex, newIcon);

		// 调用蓝图动画事件
		OnStatusIconAdded(newIcon);
	}
	
}