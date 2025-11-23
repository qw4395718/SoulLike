// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_IconSlot.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "StatusEffectInfo.h"

UUI_IconSlot::UUI_IconSlot()
{

}

void UUI_IconSlot::SetData(FStatusEffectInfo effectInfo)
{
	SetImageBrush(effectInfo.Icon);
	SetStacksNum(effectInfo.Stacks);
}

