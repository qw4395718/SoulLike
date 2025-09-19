// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "HUD_StatusBar.generated.h"

class UHUD_StatusIcon;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_StatusBar : public UUserWidget
{
	GENERATED_BODY()
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION()
	void AddStatus(FStatusEffectInfo StatusInfo);

	UFUNCTION()
	void RemoveStatus(int IconIndex);

	UFUNCTION()
	void UpdateStatus(FStatusEffectInfo StatusInfo);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	UFUNCTION()
	bool IsStausExist(int IconIndex);

	UFUNCTION()
	void CreateNewStatus(FStatusEffectInfo StatusInfo);

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
		TSubclassOf<UHUD_StatusIcon> StatusIconWidgetClass;

	// 图标管理器
	UPROPERTY()
		TArray<UUserWidget*> StatusManager;

};
