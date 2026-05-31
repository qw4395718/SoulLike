#include "HUD_PauseMenuScreen.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Manager/UIManagerSubsystem.h"
#include <LevelManager.h>

UHUD_PauseMenuScreen::UHUD_PauseMenuScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsFocusable = true;

}

/************************************************************************/
/* 继承实现                                                                     */
/************************************************************************/

void UHUD_PauseMenuScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 绑定按钮点击事件
	if (m_continueButton)
	{
		m_continueButton->OnClicked.AddDynamic(this, &UHUD_PauseMenuScreen::OnContinueClicked);
	}

	if (m_lobbyButton)
	{
		m_lobbyButton->OnClicked.AddDynamic(this, &UHUD_PauseMenuScreen::OnLobbyClicked);
	}

	if (m_mainMenuButton)
	{
		m_mainMenuButton->OnClicked.AddDynamic(this, &UHUD_PauseMenuScreen::OnMainMenuClicked);
	}

	if (m_quitButton)
	{
		m_quitButton->OnClicked.AddDynamic(this, &UHUD_PauseMenuScreen::OnQuitClicked);
	}
}

void UHUD_PauseMenuScreen::NativeConstruct()
{
	Super::NativeConstruct();

	// 暂停游戏
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(true);
	}

	UE_LOG(LogTemp, Log, TEXT("UHUD_PauseMenuScreen::NativeConstruct - Pause menu opened"));
}

void UHUD_PauseMenuScreen::NativeDestruct()
{
	// 恢复游戏
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(false);
	}

	// 通知 UIManager 关闭（更新 ActiveWidgets 状态）
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_PauseMenu);
	}

	Super::NativeDestruct();

	UE_LOG(LogTemp, Log, TEXT("UHUD_PauseMenuScreen::NativeDestruct - Pause menu closed"));
}

FReply UHUD_PauseMenuScreen::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 再按 ESC 等同于"继续游戏"——关闭菜单
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnContinueClicked();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

/************************************************************************/
/* 外部调用                                                                     */
/************************************************************************/

void UHUD_PauseMenuScreen::OpenPauseMenu(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return;
	}

	// 检查是否已打开暂停菜单，避免重复打开
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(WorldContextObject);
	if (!UIManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHUD_PauseMenuScreen::OpenPauseMenu - UIManager not found"));
		return;
	}

	// 如果 BeginPlayScreen 已打开，不允许唤出暂停菜单
	if (UIManager->IsWidgetOpen(EWidgetType::EWIDGET_BeginPlayScreen))
	{
		UE_LOG(LogTemp, Log, TEXT("UHUD_PauseMenuScreen::OpenPauseMenu - BeginPlayScreen is active, ignore ESC"));
		return;
	}

	// 如果暂停菜单已打开，不做操作
	if (UIManager->IsWidgetOpen(EWidgetType::EWIDGET_PauseMenu))
	{
		return;
	}

	// 打开暂停菜单
	UIManager->OpenScreenWidget(EWidgetType::EWIDGET_PauseMenu);
}

/************************************************************************/
/* 按钮事件                                                                     */
/************************************************************************/

void UHUD_PauseMenuScreen::OnContinueClicked()
{
	// 通过 UIManager 打开大厅界面
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager != nullptr)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_PauseMenu);
	}
}

void UHUD_PauseMenuScreen::OnLobbyClicked()
{

	// 1. 清理关卡状态（销毁所有怪物 + 重置波次）
	ALevelManager* LevelMgr = Cast<ALevelManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ALevelManager::StaticClass()));
	if (LevelMgr)
	{
		LevelMgr->CleanLevel();
	}

	// 2.通过 UIManager 打开大厅界面
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager != nullptr)
	{
		UIManager->CloseAllWidgets();
		UIManager->OpenScreenWidget(EWidgetType::EWIDGET_LobbyScreen);
	}
}

void UHUD_PauseMenuScreen::OnMainMenuClicked()
{
	// 1. 清理关卡状态（销毁所有怪物 + 重置波次）
	ALevelManager* LevelMgr = Cast<ALevelManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ALevelManager::StaticClass()));
	if (LevelMgr)
	{
		LevelMgr->CleanLevel();
	}

	// 2. 通过 UIManager 打开初始界面
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager != nullptr)
	{
		UIManager->CloseAllWidgets();
		UIManager->OpenScreenWidget(EWidgetType::EWIDGET_BeginPlayScreen);
	}
}

void UHUD_PauseMenuScreen::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), GetOwningPlayer(), EQuitPreference::Quit, false);
}