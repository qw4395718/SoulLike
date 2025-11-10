// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_StatusBar.h"
#include "HUD_StatusIcon.h"
#include "Components/HorizontalBox.h"

UHUD_StatusBar::UHUD_StatusBar(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
}

void UHUD_StatusBar::InitializeStatusBar(UHorizontalBox* InStatusIconsContainer)
{
	StatusIconsContainer = InStatusIconsContainer;
}

void UHUD_StatusBar::AddStatus(FStatusEffectInfo StatusInfo)
{
	// 重复的Icon不接受,由上级管控确保传入是新状态
	if (!StatusIconsContainer || ActiveStatusIcons.Contains(StatusInfo.IconIndex))
		return;

	CreateNewStatus(StatusInfo);
}

void UHUD_StatusBar::RemoveStatus(int IconIndex)
{
	if (UUserWidget** IconWidgetPtr = ActiveStatusIcons.Find(IconIndex))
	{
		UUserWidget* IconWidget = *IconWidgetPtr;

		// 调用蓝图移除动画
		OnStatusIconRemoved(IconWidget);

		// 延迟实际销毁，让动画有时间播放
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, IconWidget, IconIndex]()
			{
				if (IconWidget && StatusIconsContainer)
				{
					StatusIconsContainer->RemoveChild(IconWidget);
				}
				ActiveStatusIcons.Remove(IconIndex);
			}, 0.3f, false); // 延迟时间匹配动画长度
	}
}

void UHUD_StatusBar::UpdateStatus(FStatusEffectInfo StatusInfo)
{
	if (UUserWidget** WaitUpdateSubWidget = ActiveStatusIcons.Find(StatusInfo.IconIndex))
	{
		if (UHUD_StatusIcon* StatusIcon = Cast<UHUD_StatusIcon>(*WaitUpdateSubWidget))
		{
			// 已经找到数据,可进行数据更新
			StatusIcon->UpdateIcon(StatusInfo.RemainingTime, StatusInfo.Stacks);
		}
	}
}

void UHUD_StatusBar::CreateNewStatus(FStatusEffectInfo StatusInfo)
{
	RETURN_IF_TRUE(StatusIconWidgetClass == nullptr || GetWorld() == nullptr);
	// 创建IconWidget
	UUserWidget* NewIcon = CreateWidget<UUserWidget>(GetWorld(),StatusIconWidgetClass);
	if (!NewIcon)
	return;
	
	UHUD_StatusIcon* StatusIcon = Cast<UHUD_StatusIcon>(NewIcon);
	if (StatusIcon)
	{
		// 初始化后纳入管理
	// 添加到容器
		StatusIconsContainer->AddChild(NewIcon);

		// 存储引用
		ActiveStatusIcons.Add(StatusInfo.IconIndex, NewIcon);

		// 调用蓝图动画事件
		OnStatusIconAdded(NewIcon);
	}
	
}