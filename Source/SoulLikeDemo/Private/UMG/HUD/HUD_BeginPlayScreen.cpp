// Private/UI/HUD/HUD_BeginPlayScreen.cpp

#include "HUD_BeginPlayScreen.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "SL_PlayerControllerBase.h"
#include "SL_GameSaveSubsystem.h"
#include "UIManagerSubsystem.h"
#include "SoulLikeGameGlobal.h"

UHUD_BeginPlayScreen::UHUD_BeginPlayScreen(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bIsFocusable = true;
}

void UHUD_BeginPlayScreen::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // 绑定按钮点击事件
    if (NewGameButton)
    {
        NewGameButton->OnClicked.AddDynamic(this, &UHUD_BeginPlayScreen::OnNewGameClicked);
    }

    if (LoadGameButton)
    {
        LoadGameButton->OnClicked.AddDynamic(this, &UHUD_BeginPlayScreen::OnLoadGameClicked);
    }

    if (QuitGameButton)
    {
        QuitGameButton->OnClicked.AddDynamic(this, &UHUD_BeginPlayScreen::OnQuitGameClicked);
    }
}

void UHUD_BeginPlayScreen::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Log, TEXT("HUD_BeginPlayScreen::NativeConstruct - BeginPlayScreen displayed"));
}

void UHUD_BeginPlayScreen::NativeDestruct()
{
	// 绑定按钮点击事件
	if (NewGameButton)
	{
		NewGameButton->OnClicked.RemoveDynamic(this, &UHUD_BeginPlayScreen::OnNewGameClicked);
	}

	if (LoadGameButton)
	{
		LoadGameButton->OnClicked.RemoveDynamic(this, &UHUD_BeginPlayScreen::OnLoadGameClicked);
	}

	if (QuitGameButton)
	{
		QuitGameButton->OnClicked.RemoveDynamic(this, &UHUD_BeginPlayScreen::OnQuitGameClicked);
	}

	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
        UIManager->CloseScreenWidget(EWidgetType::EWIDGET_BeginPlayScreen);
	}

	Super::NativeDestruct();
}

void UHUD_BeginPlayScreen::InitializeScreen(bool bHasSaveData)
{
    // 根据是否有存档控制"加载存档"按钮的可用性
    if (LoadGameButton)
    {
        LoadGameButton->SetIsEnabled(bHasSaveData);

        UE_LOG(LogTemp, Log, TEXT("HUD_BeginPlayScreen::InitializeScreen - Has save data: %d"), bHasSaveData);
    }
}

void UHUD_BeginPlayScreen::ShowLobbyScreen()
{
	UIManager = UUIManagerSubsystem::Get(this);
	if (!UIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("UHUD_BeginPlayScreen::ShowLobbyScreen - UIManagerSubsystem not found!"));
		return;
	}

	// 通过UIManager打开界面（ZOrder=100确保在最上层）
	UIManager->OpenScreenWidget(EWidgetType::EWIDGET_LobbyScreen, 100);

	// 检查存档状态并初始化按钮
	if (UUserWidget* Widget = UIManager->GetWidget(EWidgetType::EWIDGET_LobbyScreen))
	{
        UE_LOG(LogTemp, Log, TEXT("SL_PlayerControllerBase::ShowLobbyScreen - LobbyScreen displayed"));
	}

    UIManager->CloseScreenWidget(EWidgetType::EWIDGET_BeginPlayScreen);
}

/************************************************************************/
/*                               按钮事件                                */
/************************************************************************/

void UHUD_BeginPlayScreen::OnNewGameClicked()
{
    UE_LOG(LogTemp, Log, TEXT("HUD_BeginPlayScreen::OnNewGameClicked - New Game"));

    // 清除存档数据（客户端可执行）
    if (USL_GameSaveSubsystem* SaveSubsystem = USL_GameSaveSubsystem::Get(this))
    {
        SaveSubsystem->DeleteSaveData();
    }

    // 通过 PlayerController 通知服务器执行 GameMode 操作
    if (ASL_PlayerControllerBase* PC = Cast<ASL_PlayerControllerBase>(GetOwningPlayer()))
    {
        PC->RequestNewGame();
    }

    // 加载大厅界面
    ShowLobbyScreen();
}

void UHUD_BeginPlayScreen::OnLoadGameClicked()
{
    UE_LOG(LogTemp, Log, TEXT("HUD_BeginPlayScreen::OnLoadGameClicked - Load Game"));

    // 通过 PlayerController 通知服务器执行 GameMode 操作
    if (ASL_PlayerControllerBase* PC = Cast<ASL_PlayerControllerBase>(GetOwningPlayer()))
    {
        PC->RequestLoadGame();
    }

    // 加载大厅界面
    ShowLobbyScreen();
}

void UHUD_BeginPlayScreen::OnQuitGameClicked()
{
    UE_LOG(LogTemp, Log, TEXT("HUD_BeginPlayScreen::OnQuitGameClicked - Quit Game"));

    // 退出游戏
    // UE4.26: 获取玩家控制器并退出
    APlayerController* PlayerController = GetOwningPlayer();
    if (PlayerController)
    {
        // 在编辑器中无法退出，只在打包后生效
        UKismetSystemLibrary::QuitGame(GetWorld(), PlayerController, EQuitPreference::Quit, false);
    }
}

