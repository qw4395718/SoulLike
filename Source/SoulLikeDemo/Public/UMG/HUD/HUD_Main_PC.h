// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_Main_PC.generated.h"

class UHUD_PlayerStatusBar;
class UHUD_EquipmentBar;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_Main_PC : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_Main_PC(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/



protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/


};

