// Pop_LevelComplete.cpp

#include "Pop_LevelComplete.h"
#include "UIManagerSubsystem.h"
#include "LevelManager.h"
#include "SoulLikeGameGlobal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

UPop_LevelComplete::UPop_LevelComplete(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsFocusable = true;
}

/************************************************************************/
/*                               继承实现                               */
/************************************************************************/

void UPop_LevelComplete::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 绑定按钮点击事件
	if (m_nextLevelButton)
	{
		m_nextLevelButton->OnClicked.AddDynamic(this, &UPop_LevelComplete::OnNextLevelClicked);
	}

	if (m_lobbyButton)
	{
		m_lobbyButton->OnClicked.AddDynamic(this, &UPop_LevelComplete::OnLobbyClicked);
	}

	if (m_quitButton)
	{
		m_quitButton->OnClicked.AddDynamic(this, &UPop_LevelComplete::OnQuitClicked);
	}
}

void UPop_LevelComplete::NativeConstruct()
{
	Super::NativeConstruct();

	// 设置 UI Only 输入模式，禁用玩家输入
	SetUIInputMode();

	// 根据下一关是否存在更新按钮状态
	UpdateButtonStates();

	UE_LOG(LogTemp, Log, TEXT("Pop_LevelComplete::NativeConstruct - Level complete screen displayed"));
}

void UPop_LevelComplete::NativeDestruct()
{
	// 通知 UIManager 关闭（更新 ActiveWidgets 状态）
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_LevelComplete);
	}

	Super::NativeDestruct();

	UE_LOG(LogTemp, Log, TEXT("Pop_LevelComplete::NativeDestruct - Level complete screen removed"));
}

/************************************************************************/
/*                               按钮状态                               */
/************************************************************************/

void UPop_LevelComplete::UpdateButtonStates()
{
	if (!m_nextLevelButton)
	{
		return;
	}

	// 查询 LevelManager 是否有下一关
	ALevelManager* LevelMgr = Cast<ALevelManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ALevelManager::StaticClass()));

	bool bHasNextLevel = LevelMgr && LevelMgr->GetNextLevelID() > 0;
	m_nextLevelButton->SetIsEnabled(bHasNextLevel);

	UE_LOG(LogTemp, Log, TEXT("Pop_LevelComplete::UpdateButtonStates - Has next level: %d"), bHasNextLevel);
}

/************************************************************************/
/*                               按钮事件                               */
/************************************************************************/

// 挑战下一关
void UPop_LevelComplete::OnNextLevelClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Pop_LevelComplete::OnNextLevelClicked - Go to next level"));

	// 安全校验：确认有下一关
	ALevelManager* LevelMgr = Cast<ALevelManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ALevelManager::StaticClass()));
	if (!LevelMgr || LevelMgr->GetNextLevelID() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pop_LevelComplete::OnNextLevelClicked - No next level available"));
		return;
	}

	// 1. 恢复 Game Only 输入模式
	SetGameInputMode();

	// 2. 启动下一关
	LevelMgr->GoToNextLevel();

	// 3. 最后关闭当前界面（更新UIManager内部状态）
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_LevelComplete);
	}
}

// 返回大厅
void UPop_LevelComplete::OnLobbyClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Pop_LevelComplete::OnLobbyClicked - Go to lobby"));

	// 1. 恢复 Game Only 输入模式
	SetGameInputMode();

	// 2. 关闭所有界面，打开 LobbyScreen
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseAllWidgets();
		UIManager->OpenScreenWidget(EWidgetType::EWIDGET_LobbyScreen, 100);
	}

	// 3. 最后关闭当前界面（更新UIManager内部状态）
	if (UIManager)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_LevelComplete);
	}
}

// 退出游戏
void UPop_LevelComplete::OnQuitClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Pop_LevelComplete::OnQuitClicked - Quit game"));

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
void UPop_LevelComplete::SetUIInputMode()
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
void UPop_LevelComplete::SetGameInputMode()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
}
