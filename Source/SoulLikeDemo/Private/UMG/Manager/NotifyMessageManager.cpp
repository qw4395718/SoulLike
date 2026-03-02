// Fill out your copyright notice in the Description page of Project Settings.


#include "NotifyMessageManager.h"
#include "Pop_NotifyMessage.h"
#include "Blueprint/UserWidget.h"
#include "TimerManager.h"
#include "Engine/EngineTypes.h"


UNotifyMessageManager::UNotifyMessageManager()
{
	// 在构造时指定Widget类路径
	static ConstructorHelpers::FClassFinder<UPop_NotifyMessage>
		widgetClassFinder(TEXT("/Game/SoulLikeDemo/UI/BluePrint/PopLayer/WBP_PU_NotifyMessage"));
	if (widgetClassFinder.Succeeded())
	{
		m_notificationWidgetClass = widgetClassFinder.Class;
	}
}

void UNotifyMessageManager::FakeInit()
{

}

void UNotifyMessageManager::ShowNotification(const FText& message)
{
	if (!m_bIsInitialized)
	{
		Initialize();
	}

	// 如果超过最大数量，移除最早的一个
	if (m_activeNotifications.Num() >= m_maxDisplayCount && m_activeNotifications.Num() > 0)
	{
		UPop_NotifyMessage* oldestNotification = m_activeNotifications[0];
		RemoveNotification(oldestNotification);
	}

	// 创建新提示
	if (m_notificationWidgetClass && GEngine->GameViewport)
	{
		UPop_NotifyMessage* newNotification = CreateWidget<UPop_NotifyMessage>(
			GetWorld(),
			m_notificationWidgetClass
			);

		if (newNotification)
		{
			newNotification->SetNotificationText(message);
			newNotification->AddToViewport();

			// 设置初始位置
			FVector2D viewportSize;
			GEngine->GameViewport->GetViewportSize(viewportSize);
			float xPosition = (viewportSize.X - newNotification->GetDesiredSize().X) / 2;
			newNotification->SetPositionInViewport(FVector2D(xPosition, m_initialYPosition));

			// 添加到队列
			m_activeNotifications.Add(newNotification);

			// 更新所有提示位置
			UpdateNotificationPositions();

			// 设置自动消失定时器
			FTimerHandle TimerHandle;
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUObject(this, &UNotifyMessageManager::RemoveNotification, newNotification);

			if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
			{
				World->GetTimerManager().SetTimer(
					TimerHandle,
					TimerDelegate,
					m_displayDuration,
					false
				);
			}
		}
	}
}

void UNotifyMessageManager::ClearAllNotifications()
{
	for (UPop_NotifyMessage* notification : m_activeNotifications)
	{
		if (notification)
		{
			notification->RemoveFromParent();
		}
	}
	m_activeNotifications.Empty();
}

void UNotifyMessageManager::Initialize()
{
	m_bIsInitialized = true;
}

void UNotifyMessageManager::UpdateNotificationPositions()
{
	float curcrentY = m_initialYPosition;

	// 从最新到最旧更新位置
	for (int32 i = m_activeNotifications.Num() - 1; i >= 0; i--)
	{
		if (UPop_NotifyMessage* notification = m_activeNotifications[i])
		{
			notification->MoveToPosition(curcrentY);
			curcrentY -= m_notificationSpacing;
		}
	}
}

void UNotifyMessageManager::RemoveNotification(class UPop_NotifyMessage* notification)
{
	if (!notification) return;

	// 开始消失动画
	notification->StartFadeOut();

	// 延迟移除
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this, notification]()
		{
			if (notification && notification->IsValidLowLevel())
			{
				notification->RemoveFromParent();
				m_activeNotifications.Remove(notification);
				UpdateNotificationPositions();
			}
		});

	if (UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->GetTimerManager().SetTimer(
			TimerHandle,
			TimerDelegate,
			0.5f, // 等待动画完成
			false
		);
	}
}
