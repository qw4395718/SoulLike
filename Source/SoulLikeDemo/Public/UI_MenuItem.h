// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_BaseSlot.h"
#include "UI_MenuItem.generated.h"

class UImage;
class UButton;
class UTextBlock;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UUI_MenuItem : public UUI_BaseSlot
{
	GENERATED_BODY()
public:
	UUI_MenuItem();
	/************************************************************************/
	/* 继承实现																		*/
	/************************************************************************/

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION()
		void SetNewAcquireImageVisible(bool visible);

	UFUNCTION()
		void SetCenterTitle(FString title);

	UFUNCTION()
		FName GetButtonFlag() const {return m_menuFuncName};

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/

protected:
	/************************************************************************/
	/* 内部变量                                                                     */
	/************************************************************************/
	// 新获取提示图标
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UImage* m_newTipImage;
	
	// 交互行为按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UButton* m_actionButton;

	// 按钮文本
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UTextBlock* m_btnLabel;

	// 在容器标识
	FName m_menuFuncName;
};