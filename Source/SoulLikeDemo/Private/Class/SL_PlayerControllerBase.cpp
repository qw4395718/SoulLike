// Fill out your copyright notice in the Description page of Project Settings.

#include "SL_PlayerControllerBase.h"
#include "SL_CharacterBase.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include <UIManagerSubsystem.h>

// 构造函数
ASL_PlayerControllerBase::ASL_PlayerControllerBase()
{
	// 设置输入模式默认值
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	UIManager = nullptr;
	bIsUIInitialized = false;
}

// 游戏开始
void ASL_PlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();

	// 设置初始输入模式为游戏模式
	SetInputModeGameOnly();

	// 获取子系统
	UIManager = UUIManagerSubsystem::Get(this);

	// 创建血量UI
	CreateHealthUI();
}

// 当控制器控制一个Pawn时调用
void ASL_PlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 检查是否为我们的角色类
	if (ASL_CharacterBase* MyCharacter = Cast<ASL_CharacterBase>(InPawn))
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerController possessed: %s"), *MyCharacter->GetName());

		// 当角色被控制时，初始化血量UI
		OnHealthUINeedsInitialization();
	}
}

// 当控制器放弃控制一个Pawn时调用
void ASL_PlayerControllerBase::OnUnPossess()
{
	// 可以在这里处理角色死亡或切换时的UI逻辑
	UE_LOG(LogTemp, Log, TEXT("PlayerController unpossessed"));

	Super::OnUnPossess();
}

// 设置纯UI输入模式（鼠标控制UI）
void ASL_PlayerControllerBase::SetInputModeUIOnly(UWidget* InWidgetToFocus)
{
	FInputModeUIOnly InputMode;
	if (InWidgetToFocus)
	{
		InputMode.SetWidgetToFocus(InWidgetToFocus->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockOnCapture);

	SetInputMode(InputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

// 设置纯游戏输入模式（隐藏鼠标）
void ASL_PlayerControllerBase::SetInputModeGameOnly()
{
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

// 设置游戏+UI混合输入模式
void ASL_PlayerControllerBase::SetInputModeGameAndUI(UWidget* InWidgetToFocus, bool bLockMouseToViewport)
{
	FInputModeGameAndUI InputMode;
	if (InWidgetToFocus)
	{
		InputMode.SetWidgetToFocus(InWidgetToFocus->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(bLockMouseToViewport ? EMouseLockMode::LockOnCapture : EMouseLockMode::DoNotLock);

	SetInputMode(InputMode);
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

// 获取当前控制的角色
ASL_CharacterBase* ASL_PlayerControllerBase::GetMyPlayerCharacter() const
{
	return Cast<ASL_CharacterBase>(GetPawn());
}

// 创建血量UI
void ASL_PlayerControllerBase::CreatePlayerStatusUI()
{
	// 检测是否持有有效的UI管理子系统,如无则获取,若获取不到则返回
	if (!UIManager && !UIManager = UUIManagerSubsystem::Get(this))
	{
		UE_LOG(LogTemp, Verbose, TEXT("CreatePlayerStatusUI :Can not Get Valid UUIManagerSubsystem Refence"));
		return;
	}

	// 创建UI实例
	UIManager->OpenWidget(EWidgetType::EWIDGET_PlayerStatus);
	
}

// 销毁血量UI
void ASL_PlayerControllerBase::DestroyPlayerStatusUI()
{
	// 检测是否持有有效的UI管理子系统,如无则获取,若获取不到则返回
	if (!UIManager && !UIManager = UUIManagerSubsystem::Get(this))
	{
		UE_LOG(LogTemp, Verbose, TEXT("DestroyPlayerStatusUI :Can not Get Valid UUIManagerSubsystem Refence"));
		return;
	}
	UIManager->CloseWidget(EWidgetType::EWIDGET_PlayerStatus);
}

// 当角色准备好时初始化UI
void ASL_PlayerControllerBase::OnHealthUINeedsInitialization()
{
	// 确保UI存在
	if (!HealthUIInstance)
	{
		return;
	}

	// 获取当前角色
	ASL_CharacterBase* MyCharacter = GetMyPlayerCharacter();
	if (!MyCharacter)
	{
		// 角色不存在，等待后续调用
		UE_LOG(LogTemp, Verbose, TEXT("Health UI initialization deferred: no character possessed"));
		return;
	}

	// 如果UI实现了自定义初始化函数，调用它
	// 这里假设你的UMG蓝图有一个名为"InitializeWithPlayer"的函数
	FName InitFunctionName = FName(TEXT("InitializeWithPlayer"));
	UFunction* InitFunction = HealthUIInstance->FindFunction(InitFunctionName);

	if (InitFunction)
	{
		// 准备参数
		struct FInitializeWithPlayerParams
		{
			ASL_CharacterBase* PlayerCharacter;
		};

		FInitializeWithPlayerParams Params;
		Params.PlayerCharacter = MyCharacter;

		// 调用初始化函数
		HealthUIInstance->ProcessEvent(InitFunction, &Params);

		UE_LOG(LogTemp, Log, TEXT("Health UI initialized with player: %s"), *MyCharacter->GetName());
		bIsUIInitialized = true;
	}
	else
	{
		// 如果UMG没有实现该函数，可以尝试其他方式
		UE_LOG(LogTemp, Warning, TEXT("Health UI doesn't have InitializeWithPlayer function"));
	}
}