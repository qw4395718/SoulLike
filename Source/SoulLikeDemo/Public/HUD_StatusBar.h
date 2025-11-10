// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "StatusEffectInfo.h"
#include "HUD_StatusBar.generated.h"

class UHUD_StatusIcon;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_StatusBar : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_StatusBar(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "Status Bar")
		void InitializeStatusBar(UHorizontalBox* InStatusIconsContainer);


	UFUNCTION(BlueprintCallable, Category = "Status Bar")
		void AddStatus(FStatusEffectInfo StatusInfo);

	UFUNCTION(BlueprintCallable, Category = "Status Bar")
		void RemoveStatus(int IconIndex);

	UFUNCTION(BlueprintCallable, Category = "Status Bar")
		void UpdateStatus(FStatusEffectInfo StatusInfo);

	// 为蓝图暴露的动画事件
	UFUNCTION(BlueprintImplementableEvent, Category = "Status Bar")
		void OnStatusIconAdded(UUserWidget* StatusIconWidget);

	UFUNCTION(BlueprintImplementableEvent, Category = "Status Bar")
		void OnStatusIconRemoved(UUserWidget* StatusIconWidget);

	UFUNCTION(BlueprintImplementableEvent, Category = "Status Bar")
		void OnStatusIconUpdated(UUserWidget* StatusIconWidget);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

	UFUNCTION()
		void CreateNewStatus(FStatusEffectInfo StatusInfo);

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
		TSubclassOf<UHUD_StatusIcon> StatusIconWidgetClass;

	// 控件引用
	UPROPERTY()
		UHorizontalBox* StatusIconsContainer;

	// 图标管理器
	UPROPERTY()
		TMap<int, UUserWidget*> ActiveStatusIcons;

};
