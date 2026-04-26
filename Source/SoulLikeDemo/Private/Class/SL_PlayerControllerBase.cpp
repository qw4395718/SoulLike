// Fill out your copyright notice in the Description page of Project Settings.

#include "SL_PlayerControllerBase.h"
#include "SL_CharacterBase.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include <UIManagerSubsystem.h>
#include <SL_ComboManagerComponent.h>

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
	
	// 创建通用UI(玩家状态栏,地图等)
	CreatePlayerStatusUI();

}

// 当控制器控制一个Pawn时调用
void ASL_PlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 检查是否为我们的角色类
	if (ASL_CharacterBase* MyCharacter = Cast<ASL_CharacterBase>(InPawn))
	{
		UE_LOG(LogTemp, Log, TEXT("PlayerController possessed: %s"), *MyCharacter->GetName());

	}
}

// 当控制器放弃控制一个Pawn时调用
void ASL_PlayerControllerBase::OnUnPossess()
{
	// 可以在这里处理角色死亡或切换时的UI逻辑
	UE_LOG(LogTemp, Log, TEXT("PlayerController unpossessed"));

	Super::OnUnPossess();
}

void ASL_PlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!InputComponent) return;

	// 方式一：使用 EInputEvent 精确控制按键时序
	   // IE_Pressed:  按下瞬间
	   // IE_Released: 松开瞬间
	   // IE_Repeat:   按住持续触发

	   // 轻攻击（X键 / 鼠标左键）
	InputComponent->BindAction("LightAttack", IE_Pressed, this, &ASL_PlayerControllerBase::OnLightAttackPressed);
	InputComponent->BindAction("LightAttack", IE_Released, this, &ASL_PlayerControllerBase::OnLightAttackReleased);

	// 重攻击（Y键 / 鼠标右键）
	InputComponent->BindAction("HeavyAttack", IE_Pressed, this, &ASL_PlayerControllerBase::OnHeavyAttackPressed);
	InputComponent->BindAction("HeavyAttack", IE_Released, this, &ASL_PlayerControllerBase::OnHeavyAttackReleased);

	// 特殊攻击（B键）
	InputComponent->BindAction("SpecialAttack", IE_Pressed, this, &ASL_PlayerControllerBase::OnSpecialAttackPressed);

	// 闪避（A键）
	InputComponent->BindAction("Dodge", IE_Pressed, this, &ASL_PlayerControllerBase::OnDodgePressed);

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

/************************************************************************/
/*                               界面通用UI-玩家状态UI                                       */
/************************************************************************/

// 创建玩家状态UI
void ASL_PlayerControllerBase::CreatePlayerStatusUI()
{
	// 检测是否持有有效的UI管理子系统,如无则获取,若获取不到则返回
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager == nullptr)
	{
		UE_LOG(LogTemp, Verbose, TEXT("CreatePlayerStatusUI :Can not Get Valid UUIManagerSubsystem Refence"));
		return;
	}
	// 创建参数
	FUICreateParams createParam;
	createParam.Type = EWidgetType::EWIDGET_PlayerStatus;
	// 创建UI实例
	UIManager->OpenWidget(createParam);
	// 获取玩家属性进行初始化
	UUserWidget* playerStatus = UIManager->GetWidget(EWidgetType::EWIDGET_PlayerStatus);
	if (playerStatus != nullptr)
	{
		//playerStatus->
	}
}

// 销毁血量UI
void ASL_PlayerControllerBase::DestroyPlayerStatusUI()
{
	// 检测是否持有有效的UI管理子系统,如无则获取,若获取不到则返回
	if (UIManager == nullptr && (UIManager = UUIManagerSubsystem::Get(this)) == nullptr)
	{
		UE_LOG(LogTemp, Verbose, TEXT("DestroyPlayerStatusUI :Can not Get Valid UUIManagerSubsystem Refence"));
		return;
	}
	UIManager->CloseWidget(EWidgetType::EWIDGET_PlayerStatus);
}

void ASL_PlayerControllerBase::OnLightAttackPressed()
{
	ProcessComboInput(EComboInputActionType::EComboInputAction_Light);
}

void ASL_PlayerControllerBase::OnLightAttackReleased()
{

}

void ASL_PlayerControllerBase::OnHeavyAttackPressed()
{
	ProcessComboInput(EComboInputActionType::EComboInputAction_Height);
}

void ASL_PlayerControllerBase::OnHeavyAttackReleased()
{

}

void ASL_PlayerControllerBase::OnSpecialAttackPressed()
{
	ProcessComboInput(EComboInputActionType::EComboInputAction_Special);
}

void ASL_PlayerControllerBase::OnDodgePressed()
{

}

USL_ComboManagerComponent* ASL_PlayerControllerBase::GetComboManagerComponent() const
{
	// 使用缓存
	if (CachedComboManager.IsValid())
	{
		return CachedComboManager.Get();
	}

	// 从当前控制的 Pawn 上查找
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		CachedComboManager = ControlledPawn->FindComponentByClass<USL_ComboManagerComponent>();
	}

	return CachedComboManager.Get();

}

void ASL_PlayerControllerBase::ProcessComboInput(EComboInputActionType InputType)
{

	USL_ComboManagerComponent* ComboMgr = GetComboManagerComponent();
	if (ComboMgr)
	{
		ComboMgr->HandleInputPressed(InputType);
	}
}
