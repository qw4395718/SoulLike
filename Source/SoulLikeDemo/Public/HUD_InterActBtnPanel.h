// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulLikeGameGlobal.h"
#include "StatusEffectInfo.h"
#include "HUD_InterActBtnPanel.generated.h"

class UUI_IconSlot;
class UUI_InterActButton;
class UTexture2D;

// 交互组件相关数据结构
struct FInterActOptionInfo
{
	int32				Index;
	UTexture2D*		OptionIcon;
	FString			OptionText;
};


/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UHUD_InterActBtnPanel : public UUserWidget
{
	GENERATED_BODY()
public:
	UHUD_InterActBtnPanel(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable)
		void UpdateBatch(const TArray<FInterActOptionInfo>& options);

	UFUNCTION(BlueprintCallable)
		void UpdateTarget(const FInterActOptionInfo& options);

	UFUNCTION(BlueprintCallable)
		void ClearAllOptions();

	UFUNCTION(BlueprintCallable)
		void SetTargetOptionSelected(int32 Index);

	// 按钮点击事件
	UFUNCTION(BlueprintCallable)
		void OnButtonClicked(int32 Index);

	UFUNCTION(BlueprintCallable)
		void SetVisible(bool bVisible);


protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/


protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "interActPanel")
		TSubclassOf<UUI_InterActButton> m_interActBtnClass;

	// 控件引用
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		class UVerticalBox* m_interActBtnsContainer;

	// 交互控件管理器
	UPROPERTY()
		TArray<UUI_InterActButton*> m_interActBtnArr;

	// 当前选中索引
	UPROPERTY(BlueprintReadOnly, Category = "State")
		int32 SelectedIndex = -1;

};
