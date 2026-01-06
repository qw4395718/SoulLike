// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NotifyMessageManager.generated.h"

class FText;
class UPop_NotifyMessage;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SOULLIKEDEMO_API UNotifyMessageManager : public UObject
{
	GENERATED_BODY()
public:
	UNotifyMessageManager();
	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 单元测试相关
	UFUNCTION(BlueprintCallable, Category = "NotifyManager")
	void FakeInit();

	// 

	 // 显示新提示
	UFUNCTION(BlueprintCallable)
	void ShowNotification(const FText& message);

	// 清理所有提示
	UFUNCTION(BlueprintCallable)
	void ClearAllNotifications();

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	 // 初始化
	void Initialize();

	// 更新提示位置
	void UpdateNotificationPositions();

	// 移除提示
	void RemoveNotification(class UPop_NotifyMessage* notification);

protected:
	/************************************************************************/
	/* 内部变量                                                                  */
	/************************************************************************/
	  // 提示队列
	UPROPERTY()
		TArray<UPop_NotifyMessage*> m_activeNotifications;

	// 提示类引用
	UPROPERTY()
		TSubclassOf<UPop_NotifyMessage> m_notificationWidgetClass;

	// 最大显示数量
	UPROPERTY(EditDefaultsOnly, Category = "Notification Settings")
		int32 m_maxDisplayCount = 5;

	// 提示间距
	UPROPERTY(EditDefaultsOnly, Category = "Notification Settings")
		float m_notificationSpacing = 50.0f;

	// 提示显示时间
	UPROPERTY(EditDefaultsOnly, Category = "Notification Settings")
		float m_displayDuration = 3.0f;

	// 初始Y位置
	UPROPERTY(EditDefaultsOnly, Category = "Notification Settings")
		float m_initialYPosition = 200.0f;

	bool m_bIsInitialized = false;
};

