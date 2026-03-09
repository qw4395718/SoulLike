// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "StatusEffectInfo.h"
#include "HUD_StatusBar.generated.h"

class UUI_IconSlot;

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
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Status Bar")
		void AddStatus(FStatusEffectInfo statusInfo);
		virtual void AddStatus_Implementation(FStatusEffectInfo statusInfo);


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Status Bar")
		void RemoveStatus(int iconIndex);
		virtual void RemoveStatus_Implementation(int iconIndex);


	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Status Bar")
		void UpdateStatus(FStatusEffectInfo statusInfo);
		virtual void UpdateStatus_Implementation(FStatusEffectInfo statusInfo);


	// 为蓝图暴露的动画事件
	UFUNCTION(BlueprintImplementableEvent, Category = "Status Bar")
		void OnStatusIconAdded(UUserWidget* statusIconWidget);

	UFUNCTION(BlueprintImplementableEvent, Category = "Status Bar")
		void OnStatusIconRemoved(UUserWidget* statusIconWidget);

	UFUNCTION(BlueprintImplementableEvent, Category = "Status Bar")
		void OnStatusIconUpdated(UUserWidget* statusIconWidget);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

	UFUNCTION()
		void CreateNewStatus(FStatusEffectInfo statusInfo);

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StatusBar")
		TSubclassOf<UUI_IconSlot> m_statusIconWidgetClass;

	// 控件引用
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		class UHorizontalBox* m_statusIconsContainer;

	// 图标管理器
	UPROPERTY()
		TMap<int, UUserWidget*> m_activeStatusIcons;

};
