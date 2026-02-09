// UIManagerSubsystem.cpp
#include "UIManagerSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UUIManagerSubsystem* UUIManagerSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManagerSubsystem::Get: WorldContextObject is null"));
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManagerSubsystem::Get: Cannot get World"));
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManagerSubsystem::Get: Cannot get GameInstance"));
		return nullptr;
	}

	// ✅ 正确方式：通过Subsystem系统获取
	return GameInstance->GetSubsystem<UUIManagerSubsystem>();
}

void UUIManagerSubsystem::RegisterWidget(EWidgetType WidgetType,TSubclassOf<UUserWidget>> WidgetClass)
{
	RegisteredWidgets.Add(WidgetType,WidgetClass);
}

void UUIManagerSubsystem::UnregisterWidget(EWidgetType WidgetType)
{
	if(RegisteredWidgets.Container(WidgetType))
	{
		RegisteredWidgets.Remove(WidgetType);
	}
}

void UUIManagerSubsystem::OpenWidget(EWidgetType WidgetType)
{
	if(RegisteredWidgets.Container(WidgetType))
	{
		if(ActiveWidgets.Container(WidgetType))
		{
			CloseWidget(WidgetType);
		}

		if (TSubclassOf<UUserWidget> WidgetClass = RegisteredWidgets.FindRef(WidgetType))
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
			if(Widget)
			{
				// 添加到视口
				Widget->AddToViewport();
				// 记录信息
				ActiveWidgets.Add(WidgetType,Widget);
				
			}
		}
	}
}

void UUIManagerSubsystem::CloseWidget(EWidgetType WidgetType)
{
	if(ActiveWidgets.Container(WidgetType))
	{
		ActiveWidgets.Remove(WidgetType);
	}
	PopWidget(WidgetType);
}

void UUIManagerSubsystem::CloseAllWidgets()
{
	ActiveWidgets.Reset();
	WidgetStack.Reset();
}

UUserWidget *UUIManagerSubsystem::GetWidget(EWidgetType WidgetType) const
{
	if(ActiveWidgets.Container(WidgetType)){ return ActiveWidgets.FindRef(WidgetType);}
    return nullptr;
}

bool UUIManagerSubsystem::IsWidgetOpen(EWidgetType WidgetType) const
{
	return ActiveWidgets.Container(WidgetType);
}

void UUIManagerSubsystem::PushWidget(EWidgetType WidgetType)
{
	if(WidgetStack.Find(WidgetType) != INDEX_NONE)
	{
		PopWidget(WidgetType);
	}
	else
	{
		WidgetStack.Add(WidgetType);
	}

}

void UUIManagerSubsystem::PopWidget(EWidgetType WidgetType)
{
	int findIndex = WidgetStack.Find(WidgetType);
	int stackNum = WidgetStack.Num();
	if( findIndex != INDEX_NONE)
	{
		WidgetStack.RemoveAt(findIndex,stackNum);
	}
}

void UUIManagerSubsystem::SetFocusToWidget(EWidgetType WidgetType)
{
}

FName UUIManagerSubsystem::GetFocusedWidgetName() const
{
    return FName();
}
