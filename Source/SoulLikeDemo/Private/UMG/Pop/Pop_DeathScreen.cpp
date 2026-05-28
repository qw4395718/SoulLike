// Pop_DeathScreen.cpp

#include "Pop_DeathScreen.h"
#include "UIManagerSubsystem.h"
#include "LevelManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "SoulLikeGameGlobal.h"

UPop_DeathScreen::UPop_DeathScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsFocusable = true;
}

/************************************************************************/
/*                               继承实现                               */
/************************************************************************/

void UPop_DeathScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 绑定按钮点击事件
	if (m_retryButton)
	{
		m_retryButton->OnClicked.AddDynamic(this, &UPop_DeathScreen::OnRetryClicked);
	}

	if (m_lobbyButton)
	{
		m_lobbyButton->OnClicked.AddDynamic(this, &UPop_DeathScreen::OnLobbyClicked);
	}

	if (m_quitButton)
	{
		m_quitButton->OnClicked.AddDynamic(this, &UPop_DeathScreen::OnQuitClicked);
	}
}

void UPop_DeathScreen::NativeConstruct()
{
	Super::NativeConstruct();

	// 设置 UI Only 输入模式，禁用玩家输入
	SetUIInputMode();

	UE_LOG(LogTemp, Log, TEXT("Pop_DeathScreen::NativeConstruct - Death screen displayed"));
}

void UPop_DeathScreen::NativeDestruct()
{
	// 通知 UIManager 关闭（更新 ActiveWidgets 状态）
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_DeathScreen);
	}

	Super::NativeDestruct();

	UE_LOG(LogTemp, Log, TEXT("Pop_DeathScreen::NativeDestruct - Death screen removed"));
}

/************************************************************************/
/*                               按钮事件                               */
/************************************************************************/

// 外部调用：重新挑战
void UPop_DeathScreen::OnRetryClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Pop_DeathScreen::OnRetryClicked - Retry level"));

	// 1. 恢复 Game Only 输入模式（RetryLevel 会重新初始化角色）
	SetGameInputMode();

	// 2. 获取 LevelManager 执行重新挑战
	ALevelManager* LevelMgr = Cast<ALevelManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ALevelManager::StaticClass()));
	if (LevelMgr)
	{
		LevelMgr->RetryLevel();
	}

	// 3. 最后关闭死亡界面（更新UIManager内部状态）
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_DeathScreen);
	}
}

// 外部调用：返回大厅
void UPop_DeathScreen::OnLobbyClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Pop_DeathScreen::OnLobbyClicked - Go to lobby"));

	// 1. 恢复 Game Only 输入模式
	SetGameInputMode();

	// 2. 关闭所有界面，打开 LobbyScreen
	// TODO: 后续大厅地图实现后，改为 UGameplayStatics::OpenLevel(GetWorld(), "LobbyMap");
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseAllWidgets();
		UIManager->OpenScreenWidget(EWidgetType::EWIDGET_LobbyScreen, 100);
	}
}

// 外部调用：退出游戏
void UPop_DeathScreen::OnQuitClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Pop_DeathScreen::OnQuitClicked - Quit game"));

	UKismetSystemLibrary::QuitGame(
		GetWorld(),
		GetOwningPlayer(),
		EQuitPreference::Quit,
		false);
}

/************************************************************************/
/*                               输入模式                               */
/************************************************************************/

// 内部调用：设置 UI Only 输入模式
void UPop_DeathScreen::SetUIInputMode()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

// 内部调用：恢复 Game Only 输入模式
void UPop_DeathScreen::SetGameInputMode()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}
