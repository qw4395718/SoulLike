// Fill out your copyright notice in the Description page of Project Settings.


#include "Pop_NotifyMessage.h"
#include "Components/TextBlock.h"


UPop_NotifyMessage::UPop_NotifyMessage(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	:Super(ObjectInitializer)
{
	
}

void UPop_NotifyMessage::FakeInit()
{

}

void UPop_NotifyMessage::SetNotificationText(const FText& text)
{
	if (m_notificationText)
	{
		m_notificationText->SetText(text);
	}
}

void UPop_NotifyMessage::StartFadeOut()
{
	if (m_fadeOutAnimation)
	{
		PlayAnimation(m_fadeOutAnimation);
	}
}

void UPop_NotifyMessage::MoveToPosition(float targetY)
{
	FVector2D newPosition = m_originalPosition;
	newPosition.Y = targetY;
	SetPositionInViewport(newPosition);
}

void UPop_NotifyMessage::NativeConstruct()
{
	Super::NativeConstruct();
}
