// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD_StatusBar.h"
#include "HUD_StatusIcon.h"

void UHUD_StatusBar::AddStatus(FStatusEffectInfo StatusInfo)
{
	// 重复的Icon不接受,由上级管控确保传入是新状态
	if (!IsStausExist(StatusInfo.IconIndex))
	{
		CreateNewStatus(StatusInfo);
	}
}

void UHUD_StatusBar::RemoveStatus(int IconIndex)
{
	if (UUserWidget* WaitRemoveSubWidget = FindIconIndexWidget(IconIndex))
	{
		// 已经找到数据,可进行数据清理
		StatusManager.Remove(WaitRemoveSubWidget);
	}
}

void UHUD_StatusBar::UpdateStatus(FStatusEffectInfo StatusInfo)
{
	if (UUserWidget* WaitUpdateSubWidget = FindIconIndexWidget(StatusInfo.IconIndex))
	{
		if (UHUD_StatusIcon* StatusIcon = Cast<UHUD_StatusIcon>(WaitUpdateSubWidget))
		{
			// 已经找到数据,可进行数据更新
			StatusIcon->UpdateIcon(StatusInfo.RemainingTime, StatusInfo.Stacks);
		}
	
	}
}

bool UHUD_StatusBar::IsStausExist(int IconIndex)
{
	for (UUserWidget* SubWidget : StatusManager)
	{
		if (UHUD_StatusIcon* Icon = Cast<UHUD_StatusIcon>(SubWidget))
		{
			if (true == Icon->EqualIconIndex(IconIndex))
			{
				return true;
			}
		}
	}
	return false;
}

void UHUD_StatusBar::CreateNewStatus(FStatusEffectInfo StatusInfo)
{
	RETURN_IF_TRUE(StatusIconWidgetClass == nullptr || GetWorld() == nullptr);
	// 创建IconWidget
	UUserWidget* NewIcon = CreateWidget<UUserWidget>(GetWorld(),StatusIconWidgetClass);
	if (NewIcon)
	{
		UHUD_StatusIcon* StatusIcon = Cast<UHUD_StatusIcon>(NewIcon);
		if (StatusIcon)
		{
			// 初始化后纳入管理
			StatusIcon->InitializeIcon(StatusInfo);
			StatusManager.Add(NewIcon);
		}
	}
}

UUserWidget* UHUD_StatusBar::FindIconIndexWidget(int IconIndex)
{
	bool FindFlag = false;
	UUserWidget* FindSubWidget = nullptr;
	for (UUserWidget* SubWidget : StatusManager)
	{
		if (UHUD_StatusIcon* Icon = Cast<UHUD_StatusIcon>(SubWidget))
		{
			if (true == Icon->EqualIconIndex(IconIndex))
			{
				FindFlag = true;
				FindSubWidget = Icon;
			}
		}
	}
	if (FindSubWidget != nullptr && FindFlag == true)
	{
		return FindSubWidget;
	}
	else
	{
		return nullptr;
	}
}
