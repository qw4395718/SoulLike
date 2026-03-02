// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Pop_NotifyMessage.generated.h"

class UTextBlock;
class UWidgetAnimation;

/**
 * 
 */
UCLASS()
class SOULLIKEDEMO_API UPop_NotifyMessage : public UUserWidget
{
	GENERATED_BODY()
public:
	UPop_NotifyMessage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 单元测试相关
	UFUNCTION(BlueprintCallable, Category = "Dialog")
	void FakeInit();

	// 设置提示文本
	UFUNCTION(BlueprintCallable)
	void SetNotificationText(const FText& text);

	// 开始消失动画
	UFUNCTION(BlueprintCallable)
	void StartFadeOut();

	// 移动到目标位置
	UFUNCTION(BlueprintCallable)
	void MoveToPosition(float targetY);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	// 继承
	virtual void NativeConstruct() override;

protected:
	/************************************************************************/
	/* 内部变量                                                                  */
	/************************************************************************/
	UPROPERTY(meta = (BindWidget))
	UTextBlock* m_notificationText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* m_fadeOutAnimation;

	FVector2D m_originalPosition;

};

