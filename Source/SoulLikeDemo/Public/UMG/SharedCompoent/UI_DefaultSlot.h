// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_BaseSlot.h"
#include "UI_DefaultSlot.generated.h"

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UUI_DefaultSlot : public UUI_BaseSlot
{
	GENERATED_BODY()
public:
	UUI_DefaultSlot();
	/************************************************************************/
	/* 继承实现																		*/
	/************************************************************************/

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION()
		void SetNewAcquireImageVisible(bool visible);

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
		class UImage* m_newTipImage;

};


