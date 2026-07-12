// UIManagerSubsystem.cpp
#include "UIManagerSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include <Components/WidgetComponent.h>
#include <HUD_PawnStatusBarInScreen.h>
#include "SL_UserWidgetBase.h"

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
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_LevelComplete,TEXT("/Game/SoulLikeDemo/UI/BluePrint/PopLayer/WBP_PU_LevelComplete.WBP_PU_LevelComplete"));
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
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_ClassSelectScreen, TEXT("/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_ClassSelectScreen.WBP_HUD_ClassSelectScreen"));
	RegisterWidgetFromBPPath(EWidgetType::EWIDGET_ItemUseUI, TEXT("/Game/SoulLikeDemo/UI/BluePrint/HUDLayer/WBP_HUD_ItemUseUI.WBP_HUD_ItemUseUI"));

	UE_LOG(LogTemp, Log, TEXT("UIManagerSubsystem initialized"));
}

/************************************************************************/
/* 平台识别                                                                     */
/************************************************************************/

EPlatformType UUIManagerSubsystem::GetPlatform() const
{
#if PLATFORM_ANDROID || PLATFORM_IOS
	return EPlatformType::Mobile;
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

/************************************************************************/
/* 打开屏幕Widget（保留原始接口，支持自动ZOrder（ZOrder < 0 时从Layer计算））                */
/************************************************************************/
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
			CloseScreenWidget(WidgetType);
		}

		if (WidgetClass)
		{
			UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), WidgetClass);
			if (Widget)
			{
				// 设置USL_UserWidgetBase基类属性
				if (USL_UserWidgetBase* BaseWidget = Cast<USL_UserWidgetBase>(Widget))
				{
					BaseWidget->SetWidgetType(WidgetType);
					BaseWidget->SetUILayer(UILayerUtils::GetLayerForWidgetType(WidgetType));
				}

				// 计算ZOrder：负数视为自动，从Layer计算
				int32 FinalZOrder = ZOrder;
				if (FinalZOrder < 0)
				{
					FinalZOrder = CalculateZOrderForWidget(WidgetType);
				}

				// 添加到视口（ZOrder越大越靠前）
				Widget->AddToViewport(FinalZOrder);
				// 记录信息
				ActiveWidgets.Add(WidgetType, Widget);

				// 自动管理InputMode
				ApplyInputModeForWidget(Widget, WidgetType);

				// 导航栈管理
				if (ShouldPushToNavigation(WidgetType, Widget))
				{
					FUINavigationEntry Entry;
					Entry.WidgetType = WidgetType;
					Entry.Widget = Widget;
					Entry.EnteredTime = FPlatformTime::Seconds();
					NavigationStack.Push(Entry);

					// 触发导航事件
					if (USL_UserWidgetBase* BaseWidget = Cast<USL_UserWidgetBase>(Widget))
					{
						FUINavigationContext Context;
						Context.TargetType = WidgetType;
						BaseWidget->OnNavigatedTo(Context);
					}
				}
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
						if (Comp->ComponentHasTag(CreateParam.TargetBodyTag) &&
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

void UUIManagerSubsystem::CloseWidget(const FUICreateParams& CloseParam)
{
	if (CloseParam.TargetActor != nullptr)
	{
		CloseWorldWidgetWithActor(CloseParam);
	}
	else
	{
		CloseScreenWidget(CloseParam.Type);
	}
}

/************************************************************************/
/* 关闭屏幕Widget（新增：自动恢复InputMode + 导航事件）                                   */
/************************************************************************/
void UUIManagerSubsystem::CloseScreenWidget(EWidgetType WidgetType)
{
	UUserWidget* Widget = ActiveWidgets.FindRef(WidgetType);

	if (Widget)
	{
		// 通知导航离开
		if (USL_UserWidgetBase* BaseWidget = Cast<USL_UserWidgetBase>(Widget))
		{
			FUINavigationContext Context;
			Context.SourceType = WidgetType;
			BaseWidget->OnNavigatedFrom(Context);
		}

		Widget->RemoveFromParent();
	}
	else
	{
		UE_LOG(LogTemp, Verbose, TEXT("UUIManagerSubsystem::CloseScreenWidget - Widget %d not open"),
			static_cast<int32>(WidgetType));
		return;
	}

	ActiveWidgets.Remove(WidgetType);

	// 从导航栈中移除
	RemoveFromNavigationStack(WidgetType);

	// 移除该Widget的InputMode条目 + 应用栈顶
	RemoveFromInputModeStack(WidgetType);
	ApplyCurrentInputMode();

	// 触发新栈顶的恢复事件
	if (NavigationStack.Num() > 0)
	{
		FUINavigationEntry& TopEntry = NavigationStack.Top();
		if (UUserWidget* TopWidget = TopEntry.Widget.Get())
		{
			if (USL_UserWidgetBase* BaseWidget = Cast<USL_UserWidgetBase>(TopWidget))
			{
				FUINavigationContext Context;
				Context.bIsBackNavigation = true;
				Context.TargetType = TopEntry.WidgetType;
				Context.Payload = TopEntry.Payload;
				BaseWidget->OnNavigatedBack(Context);
			}
		}
	}
}


void UUIManagerSubsystem::CloseWorldWidgetWithActor(const FUICreateParams& CloseParam)
{
	FString Key = CloseParam.TargetActor->GetName() + FString::FromInt(static_cast<int32>(CloseParam.Type));
	if (ActiveWorldWidgets.Contains(Key))
	{
		if (UUserWidget* Widget = ActiveWorldWidgets.FindRef(Key))
		{
			// 获取指定组件插槽
			TArray<UWidgetComponent*> WidgetComponents;
			CloseParam.TargetActor->GetComponents<UWidgetComponent>(WidgetComponents);
			for (UWidgetComponent* Comp : WidgetComponents)
			{
				// 通过组件标签识别（推荐）
				if (Comp->ComponentHasTag(CloseParam.TargetBodyTag) &&
					Comp->IsRegistered())
				{
					Comp->SetVisibility(false);
	
					// 记录信息
					ActiveWorldWidgets.Remove(Key);
					return;
				}
			}
		}
	}
}

void UUIManagerSubsystem::CloseAllWidgets()
{
	// 先拷贝出所有Widget指针，避免迭代中容器被修改
	TArray<UUserWidget*> AllWidgets;
	ActiveWidgets.GenerateValueArray(AllWidgets);

	for (UUserWidget* Widget : AllWidgets)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
	}
	ActiveWidgets.Reset();

	TArray<UUserWidget*> AllWorldWidgets;
	ActiveWorldWidgets.GenerateValueArray(AllWorldWidgets);

	for (UUserWidget* Widget : AllWorldWidgets)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
	}
	ActiveWorldWidgets.Reset();

	// 清理栈
	InputModeStack.Reset();
	NavigationStack.Reset();
	FocusedWidgetName = EWidgetType::EWIDGET_None;
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

/************************************************************************/
/* 焦点管理                                                                     */
/************************************************************************/
void UUIManagerSubsystem::SetFocusToWidget(EWidgetType WidgetType)
{
	if (!ActiveWidgets.Contains(WidgetType))
	{
		UE_LOG(LogTemp, Verbose, TEXT("UUIManagerSubsystem::SetFocusToWidget - Widget %d not open"),
			static_cast<int32>(WidgetType));
		return;
	}

	UUserWidget* Widget = ActiveWidgets.FindRef(WidgetType);
	if (!Widget) return;

	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;

	FocusedWidgetName = WidgetType;
}

FName UUIManagerSubsystem::GetFocusedWidgetName() const
{
	return StaticEnum<EWidgetType>()->GetNameByValue(static_cast<int64>(FocusedWidgetName));
}

void UUIManagerSubsystem::UpdateHealthUI(float NewHealth)
{
	
}

/************************************************************************/
/* 导航系统                                                                     */
/************************************************************************/
void UUIManagerSubsystem::NavigateTo(EWidgetType WidgetType, const FUINavigationPayload& InPayload, EUINavigationMode InMode /*= EUINavigationMode::Push*/)
{
	switch (InMode)
	{
	case EUINavigationMode::ReplaceTop:
	{
		// 关闭当前栈顶，打开目标页面替换之（栈深度不变）
		if (NavigationStack.Num() > 0)
		{
			FUINavigationEntry CurrentTop = NavigationStack.Top();
			NavigationStack.Pop();

			if (UUserWidget* Widget = CurrentTop.Widget.Get())
			{
				if (USL_UserWidgetBase* BaseWidget = Cast<USL_UserWidgetBase>(Widget))
				{
					FUINavigationContext Ctx;
					Ctx.SourceType = CurrentTop.WidgetType;
					Ctx.bIsBackNavigation = false;
					BaseWidget->OnNavigatedFrom(Ctx);
				}
				Widget->RemoveFromParent();
				ActiveWidgets.Remove(CurrentTop.WidgetType);
			}
			RemoveFromInputModeStack(CurrentTop.WidgetType);
		}
		OpenScreenWidget(WidgetType, -1);
		break;
	}
	case EUINavigationMode::ClearStack:
	{
		// 从栈顶到栈底逐个关闭所有导航页面，清空栈
		while (NavigationStack.Num() > 0)
		{
			FUINavigationEntry Entry = NavigationStack.Top();
			NavigationStack.Pop();

			if (UUserWidget* Widget = Entry.Widget.Get())
			{
				if (USL_UserWidgetBase* BaseWidget = Cast<USL_UserWidgetBase>(Widget))
				{
					FUINavigationContext Ctx;
					Ctx.SourceType = Entry.WidgetType;
					Ctx.bIsBackNavigation = true;
					BaseWidget->OnNavigatedFrom(Ctx);
				}
				Widget->RemoveFromParent();
				ActiveWidgets.Remove(Entry.WidgetType);
			}
		}
		InputModeStack.Reset(); // 清空InputMode栈，新页面从干净状态开始
		OpenScreenWidget(WidgetType, -1); // 新页面作为根
		break;
	}
	default: // Push
	{
		OpenScreenWidget(WidgetType, -1);
		break;
	}
	}

	// 更新载荷（三种模式共用）
	if (!InPayload.IsEmpty() &&
		NavigationStack.Num() > 0)
	{
		NavigationStack.Last().Payload = InPayload;
	}
}

void UUIManagerSubsystem::NavigateBack(int32 InStep)
{
	if (NavigationStack.Num() <= 1)
	{
		UE_LOG(LogTemp, Verbose, TEXT("UUIManagerSubsystem::NavigateBack - Nothing to pop (stack size %d)"),
			NavigationStack.Num());
		return;
	}

	const int32 StepsToPop = FMath::Min(InStep, NavigationStack.Num() - 1);
	for (int32 i = 0; i < StepsToPop; ++i)
	{
		FUINavigationEntry Entry = NavigationStack.Top();
		NavigationStack.Pop();

		if (UUserWidget* Widget = Entry.Widget.Get())
		{
			// 通知导航离开
			if (USL_UserWidgetBase* BaseWidget = Cast<USL_UserWidgetBase>(Widget))
			{
				FUINavigationContext Context;
				Context.SourceType = Entry.WidgetType;
				Context.bIsBackNavigation = true;
				BaseWidget->OnNavigatedFrom(Context);
			}

			Widget->RemoveFromParent();
			ActiveWidgets.Remove(Entry.WidgetType);
		}

		// 移除该Widget的InputMode条目
		RemoveFromInputModeStack(Entry.WidgetType);
	}

	// 应用当前栈顶的InputMode
	ApplyCurrentInputMode();
	// 通知新栈顶恢复
	if (NavigationStack.Num() > 0)
	{
		FUINavigationEntry& NewTop = NavigationStack.Top();
		if (UUserWidget* Widget = NewTop.Widget.Get())
		{
			if (USL_UserWidgetBase* BaseWidget = Cast<USL_UserWidgetBase>(Widget))
			{
				FUINavigationContext Context;
				Context.bIsBackNavigation = true;
				Context.TargetType = NewTop.WidgetType;
				Context.Payload = NewTop.Payload;
				BaseWidget->OnNavigatedBack(Context);
			}
		}
	}
}

int32 UUIManagerSubsystem::GetNavigationStackSize() const
{
	return NavigationStack.Num();
}

EWidgetType UUIManagerSubsystem::GetTopNavigationWidgetType() const
{
	if (NavigationStack.Num() > 0)
	{
		return NavigationStack.Top().WidgetType;
	}
	return EWidgetType::EWIDGET_None;
}

/************************************************************************/
/* 内部辅助：获取PlayerController                                                  */
/************************************************************************/
APlayerController* UUIManagerSubsystem::GetPlayerController() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	return World->GetFirstPlayerController();
}

/************************************************************************/
/* 内部辅助：计算自动ZOrder                                                         */
/************************************************************************/
int32 UUIManagerSubsystem::CalculateZOrderForWidget(EWidgetType WidgetType) const
{
	const int32 BaseZ = UILayerUtils::GetBaseZOrderForWidget(WidgetType);
	const EUILayer Layer = UILayerUtils::GetLayerForWidgetType(WidgetType);

	// 统计同一层级已打开的Widget数量
	int32 LayerCount = 0;
	for (const auto& Pair : ActiveWidgets)
	{
		if (UILayerUtils::GetLayerForWidgetType(Pair.Key) == Layer)
		{
			++LayerCount;
		}
	}
	return BaseZ + LayerCount;
}

/************************************************************************/
/* 内部辅助：判断是否应入导航栈                                                       */
/************************************************************************/
bool UUIManagerSubsystem::ShouldPushToNavigation(EWidgetType WidgetType, UUserWidget* InWidget) const
{
	if (USL_UserWidgetBase* BaseWidget = Cast<USL_UserWidgetBase>(InWidget))
	{
		return BaseWidget->IsNavigationPage();
	}
	// 未继承基类的Widget，根据层级判断（>= Popup 的页面才入栈）
	return UILayerUtils::GetLayerForWidgetType(WidgetType) >= EUILayer::Popup;
}

/************************************************************************/
/* 内部辅助：从导航栈中移除                                                           */
/************************************************************************/
void UUIManagerSubsystem::RemoveFromNavigationStack(EWidgetType WidgetType)
{
	for (int32 i = NavigationStack.Num() - 1; i >= 0; --i)
	{
		if (NavigationStack[i].WidgetType == WidgetType)
		{
			NavigationStack.RemoveAt(i);
			break;
		}
	}
}

/************************************************************************/
/* 内部辅助：应用InputMode                                                            */
/************************************************************************/
void UUIManagerSubsystem::ApplyInputModeForWidget(UUserWidget* InWidget, EWidgetType InWidgetType)
{
	APlayerController* PC = GetPlayerController();
	if (!PC || !InWidget) return;

	// 获取该Widget的InputMode需求
	EUIInputModeRequirement Req;
	if (USL_UserWidgetBase* BaseWidget = Cast<USL_UserWidgetBase>(InWidget))
	{
		Req = BaseWidget->GetResolvedInputModeRequirement();
	}
	else
	{
		Req = UILayerUtils::GetLayerConfig(
			UILayerUtils::GetLayerForWidgetType(InWidgetType)).DefaultInputMode;
	}

	// 保存当前状态
	FUISavedInputState SavedState;
	SavedState.bShowMouseCursor = Req != EUIInputModeRequirement::GameOnly ? 1 : 0;
	SavedState.bEnableClickEvents = Req != EUIInputModeRequirement::GameOnly ? 1 : 0;
	SavedState.bEnableMouseOverEvents = Req != EUIInputModeRequirement::GameOnly ? 1 : 0;
	if (Req == EUIInputModeRequirement::GameOnly)
		SavedState.ModeType = 0;
	else if (Req == EUIInputModeRequirement::GameAndUI)
		SavedState.ModeType = 1;
	else if (Req == EUIInputModeRequirement::UIOnly)
		SavedState.ModeType = 2;
	else
		SavedState.ModeType = 2;

	SavedState.SourceWidgetType = InWidgetType;
	InputModeStack.Push(SavedState);

	// 设置新InputMode
	switch (Req)
	{
	case EUIInputModeRequirement::GameAndUI:
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(InWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		break;
	}
	case EUIInputModeRequirement::UIOnly:
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(InWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;
		break;
	}
	default:
		break;
	}
}

/************************************************************************/
/* 内部辅助：按WidgetType移除InputMode条目                                              */
/************************************************************************/
void UUIManagerSubsystem::RemoveFromInputModeStack(EWidgetType InWidgetType)
{
	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	for (int32 i = InputModeStack.Num() - 1; i >= 0; --i)
	{
		if (InputModeStack[i].SourceWidgetType == InWidgetType)
		{
			InputModeStack.RemoveAt(i);
			return;
		}
	}
}

/************************************************************************/
/* 内部辅助：应用当前栈顶的InputMode或默认值                                            */
/************************************************************************/
void UUIManagerSubsystem::ApplyCurrentInputMode()
{
	APlayerController* PC = GetPlayerController();
	if (!PC) return;

	if (InputModeStack.Num() == 0)
	{
		// 栈空 → 恢复默认GameOnly
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
		PC->bEnableClickEvents = false;
		PC->bEnableMouseOverEvents = false;
		return;
	}

	FUISavedInputState SavedState = InputModeStack.Top();
	PC->bShowMouseCursor = SavedState.bShowMouseCursor != 0;
	PC->bEnableClickEvents = SavedState.bEnableClickEvents != 0;
	PC->bEnableMouseOverEvents = SavedState.bEnableMouseOverEvents != 0;

	switch (SavedState.ModeType)
	{
	case 0: // GameOnly
		PC->SetInputMode(FInputModeGameOnly());
		break;
	case 1: // GameAndUI
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		break;
	}
	case 2: // UIOnly
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		break;
	}
	default:
		PC->SetInputMode(FInputModeGameOnly());
		break;
	}
}
