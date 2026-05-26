// UIManagerSubsystem.cpp
#include "UIManagerSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include <Components/WidgetComponent.h>
#include <HUD_PawnStatusBarInScreen.h>

void UUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	/**
	 * WidgetBlueprint'/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_CoinIconBar.WBP_HUD_CoinIconBar'
	 * WidgetBlueprint'/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_Dialog.WBP_HUD_Dialog'
	 * WidgetBlueprint'/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_EquipmentBar.WBP_HUD_EquipmentBar'
	 * WidgetBlueprint'/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_PC.WBP_HUD_PC'
	 * WidgetBlueprint'/Game/SoulLikeDemo/UI/BluePrint/PopLayer/WBP_PU_InterActPanel.WBP_PU_InterActPanel'
	 * WidgetBlueprint'/Game/SoulLikeDemo/UI/BluePrint/PopLayer/WBP_PU_MainMenu.WBP_PU_MainMenu'
	 * WidgetBlueprint'/Game/SoulLikeDemo/UI/BluePrint/PopLayer/WBP_PU_NotifyMessage.WBP_PU_NotifyMessage'
	 * 
	 */
	 /************************************************************************/
	 /*                             HUDLayer                                         */
	 /************************************************************************/
	//RegisterWidgetFromBPPath(EWidgetType::EWIDGET_PCGameMain, TEXT("/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_PC.WBP_HUD_PC"));
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_PlayerStatus, TEXT("/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_PlayerStatusBar.WBP_HUD_PlayerStatusBar"));
	/************************************************************************/
	/*                              PopLayer                                        */
	/************************************************************************/
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_InterActPanel,TEXT("/Game/SoulLikeDemo/UI/BluePrint/PopLayer/WBP_PU_InterActPanel.WBP_PU_InterActPanel"));
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_MainMenu,TEXT("/Game/SoulLikeDemo/UI/BluePrint/PopLayer/WBP_PU_MainMenu.WBP_PU_MainMenu"));
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_DeathScreen,TEXT("/Game/SoulLikeDemo/UI/BluePrint/PopLayer/WBP_PU_DeathScreen.WBP_PU_DeathScreen"));
	//RegisterWidgetFromBPPath(EWidgetType::EWIDGET_Inventory,TEXT("/Game/SoulLikeDemo/UI/BluePrint/PopLayer/WBP_PU_NotifyMessage.WBP_PU_NotifyMessage"));
	
	/************************************************************************/
	/*                             Screen                                         */
	/************************************************************************/
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_PawnStatusInScreen, TEXT("/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_PawnStatusInScreen.WBP_HUD_PawnStatusInScreen"));
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_LockOnIndicator, TEXT("/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_LockOnIndicator.WBP_HUD_LockOnIndicator"));

	/************************************************************************/
	/*                          全屏界面                                      */
	/************************************************************************/
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_BeginPlayScreen, TEXT("/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_BeginPlayScreen.WBP_HUD_BeginPlayScreen"));
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_LobbyScreen, TEXT("/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_LobbyScreen.WBP_HUD_LobbyScreen"));
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_PauseMenu, TEXT("/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_PauseMenuScreen.WBP_HUD_PauseMenuScreen"));

	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem initialized"));
}

/************************************************************************/
/* 平台识别                                                                     */
/************************************************************************/

EPlatformType UUIManagerSubsystem::GetPlatform() const
{
#if PLATFORM_ANDROID || PLATFORM_IOS
	return EPlatformType::Mobile;
#elif PLATFORM_XBOXONE || PLATFORM_PS4
	return EPlatformType::Console;
#else
	return EPlatformType::PC;
#endif
}

TSubclassOf<UUserWidget> UUIManagerSubsystem::GetWidgetClassForPlatform(EWidgetType WidgetType) const
{
	TSubclassOf<UUserWidget> WidgetClass;
	const EPlatformType Platform = const_cast<UUIManagerSubsystem*>(this)->GetPlatform();

	if (Platform == EPlatformType::Mobile && MobileWidgets.Contains(WidgetType))
		WidgetClass = MobileWidgets.FindRef(WidgetType);
	else if (Platform == EPlatformType::Console && ConsoleWidgets.Contains(WidgetType))
		WidgetClass = ConsoleWidgets.FindRef(WidgetType);

	if (!WidgetClass)
		WidgetClass = RegisteredWidgets.FindRef(WidgetType); // fallback to PC

	return WidgetClass;
}

void UUIManagerSubsystem::Deinitialize()
{
	CloseAllWidgets();
	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem deinitialized"));
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
		// Cook / 引擎初始化 / 打包阶段没有游戏 World，这是预期行为
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

void UUIManagerSubsystem::RegisterWidgetFromBPPath(EWidgetType WidgetType,const FString& WidgetClassPathStr)
{
	// 确保路径以 "_C" 结尾
	FString ClassPath = WidgetClassPathStr;
	if (!ClassPath.EndsWith("_C"))
	{
		ClassPath += "_C";
	}

	// 使用 LoadClass 动态加载
	UClass* WidgetClass = LoadClass<UUserWidget>(nullptr, *ClassPath);
	if (WidgetClass)
	{
		RegisterWidget(WidgetType, WidgetClass);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load widget class: %s"), *WidgetClassPathStr);
	}
}

void UUIManagerSubsystem::RegisterPlatformWidgetFromBPPath(EWidgetType WidgetType, const FString& WidgetClassPathStr, EPlatformType InPlatform)
{
	FString ClassPath = WidgetClassPathStr;
	if (!ClassPath.EndsWith("_C"))
	{
		ClassPath += "_C";
	}

	UClass* WidgetClass = LoadClass<UUserWidget>(nullptr, *ClassPath);
	if (WidgetClass)
	{
		TMap<EWidgetType, TSubclassOf<UUserWidget>>* TargetMap = nullptr;
		if (InPlatform == EPlatformType::Mobile)
			TargetMap = &MobileWidgets;
		else if (InPlatform == EPlatformType::Console)
			TargetMap = &ConsoleWidgets;

		if (TargetMap)
			TargetMap->Add(WidgetType, WidgetClass);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load widget class for platform: %s"), *WidgetClassPathStr);
	}
}

void UUIManagerSubsystem::RegisterWidget(EWidgetType WidgetType,TSubclassOf<UUserWidget> WidgetClass)
{
	RegisteredWidgets.Add(WidgetType,WidgetClass);
}

void UUIManagerSubsystem::UnregisterWidget(EWidgetType WidgetType)
{
	if(RegisteredWidgets.Contains(WidgetType))
	{
		RegisteredWidgets.Remove(WidgetType);
	}
}

void UUIManagerSubsystem::OpenWidget(const FUICreateParams& CreateParam)
{
	if (CreateParam.TargetActor != nullptr)
	{
		OpenWorldWidgetWithActor(CreateParam);
	}
	else
	{
		OpenScreenWidget(CreateParam.Type, CreateParam.ZOrder);
	}
}

void UUIManagerSubsystem::OpenScreenWidget(EWidgetType WidgetType, int32 ZOrder)
{
	TSubclassOf<UUserWidget> WidgetClass = GetWidgetClassForPlatform(WidgetType);
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("UUIManagerSubsystem::OpenScreenWidget - Widget type %d not registered for any platform"),
			static_cast<int32>(WidgetType));
		return;
	}

	if (RegisteredWidgets.Contains(WidgetType) ||
		MobileWidgets.Contains(WidgetType) ||
		ConsoleWidgets.Contains(WidgetType))
	{
		if (ActiveWidgets.Contains(WidgetType))
		{
			CloseWidget(WidgetType);
		}

		if (WidgetClass)
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
			if (Widget)
			{
				// 添加到视口（ZOrder越大越靠前）
				Widget->AddToViewport(ZOrder);
				// 记录信息
				ActiveWidgets.Add(WidgetType, Widget);
			}
		}
	}
}


void UUIManagerSubsystem::OpenWorldWidgetWithActor(const FUICreateParams& CreateParam)
{
	if (RegisteredWidgets.Contains(CreateParam.Type))
	{
		if (CreateParam.TargetActor != nullptr && IsWorldWidget(CreateParam.Type))
		{
			if (TSubclassOf<UUserWidget> WidgetClass = RegisteredWidgets.FindRef(CreateParam.Type))
			{
				UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
				if (Widget)
				{
					// 获取指定组件插槽
					TArray<UWidgetComponent*> WidgetComponents;
					CreateParam.TargetActor->GetComponents<UWidgetComponent>(WidgetComponents);
					for (UWidgetComponent* Comp : WidgetComponents)
					{
						// 通过组件标签识别（推荐）
						if (Comp->ComponentHasTag(FName("HeadUI")) &&
							Comp->IsRegistered())
						{
							// 添加到指定区域
							Comp->SetWidget(Widget);
							Comp->SetVisibility(true);
							// 记录数据来源
							if (IScreenWidget_IF* ScreenWidgetTarget = Cast<IScreenWidget_IF>(Widget))
							{
								ScreenWidgetTarget->SetOwningPawn(CreateParam.TargetActor);
							}
							// 记录信息
							FString Key = CreateParam.TargetActor->GetName() + FString::FromInt(static_cast<int32>(CreateParam.Type));
							ActiveWorldWidgets.Add(Key, Widget);
							return;
						}

					}
				}
			}

		}
	}
}

void UUIManagerSubsystem::CloseWidget(EWidgetType WidgetType)
{
	if (ActiveWidgets.Contains(WidgetType))
	{
		if (UUserWidget* Widget = ActiveWidgets.FindRef(WidgetType))
		{
			Widget->RemoveFromParent();
		}
		ActiveWidgets.Remove(WidgetType);
	}
	PopWidget(WidgetType);
}

void UUIManagerSubsystem::CloseAllWidgets()
{
	for (auto& Pair : ActiveWidgets)
	{
		if (UUserWidget* Widget = Pair.Value)
		{
			Widget->RemoveFromParent();
		}
	}
	ActiveWidgets.Reset();
	WidgetStack.Reset();
}

void UUIManagerSubsystem::SetWidgetVisible(EWidgetType WidgetType)
{

}

void UUIManagerSubsystem::ToggleWidgetVisible(EWidgetType WidgetType)
{

}

UUserWidget *UUIManagerSubsystem::GetWidget(EWidgetType WidgetType) const
{
	if(ActiveWidgets.Contains(WidgetType)){ return ActiveWidgets.FindRef(WidgetType);}
    return nullptr;
}

bool UUIManagerSubsystem::IsWidgetOpen(EWidgetType WidgetType) const
{
	return ActiveWidgets.Contains(WidgetType);
}

bool UUIManagerSubsystem::IsWorldWidget(EWidgetType WidgetType) const
{
	if (static_cast<int>(WidgetType) > static_cast<int>(EWidgetType::EWIDGET_WorldWidgetStart) &&
		static_cast<int>(WidgetType) < static_cast<int>(EWidgetType::EWIDGET_WorldWidgetEnd))
	{
		return true;
	}
	else{return false;}
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

void UUIManagerSubsystem::UpdateHealthUI(float NewHealth)
{
	
}