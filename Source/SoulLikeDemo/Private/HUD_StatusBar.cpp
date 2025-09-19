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
	
}

void UHUD_StatusBar::UpdateStatus(FStatusEffectInfo StatusInfo)
{

}

bool UHUD_StatusBar::IsStausExist(int IconIndex)
{
	
}

void UHUD_StatusBar::CreateNewStatus(FStatusEffectInfo StatusInfo)
{
	RETURN_IF_TRUE(StatusIconWidgetClass == nullptr);
	// 创建IconWidget
	UUserWidget* NewIcon = CreateWidget<UUserWidget>(World,StatusIconWidgetClass);
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
