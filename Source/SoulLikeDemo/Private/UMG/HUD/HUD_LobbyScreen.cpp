#include "HUD_LobbyScreen.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/SL_GameSaveSubsystem.h"
#include "Manager/DataTableManager.h"
#include "Table/ClassConfigInfoTable.h"
#include "Table/WaveConfigInfoTable.h"
#include "SoulLikeGameGlobal.h"
#include "SL_GameModeBase.h"
#include <UI_ListItemBase.h>

UHUD_LobbyScreen::UHUD_LobbyScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, m_savedLevelID(1)
	, m_playerClassID(1001)
{
}

/************************************************************************/
/* 继承实现                                                                     */
/************************************************************************/

void UHUD_LobbyScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UHUD_LobbyScreen::NativeConstruct()
{
	Super::NativeConstruct();

	// 设置输入模式：只显示UI，阻止玩家输入
	SetUIInputMode();

	InitializeLobby();
}


void UHUD_LobbyScreen::NativeDestruct()
{
	Super::NativeDestruct();

	SetGameInputMode();
}

/************************************************************************/
/* 外部调用                                                                     */
/************************************************************************/

void UHUD_LobbyScreen::InitializeLobby()
{
	// 1. 读取存档数据
	USL_GameSaveSubsystem* SaveSystem = USL_GameSaveSubsystem::Get(this);
	if (SaveSystem)
	{
		int32 LoadedLevelID = 1;
		int32 LoadedClassID = 1001;
		SaveSystem->LoadGame(LoadedLevelID, LoadedClassID);
		m_savedLevelID = LoadedLevelID;
		m_playerClassID = LoadedClassID;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UHUD_LobbyScreen::InitializeLobby - SaveSystem not found, use defaults"));
		m_savedLevelID = 1;
		m_playerClassID = 1001;
	}

	// 2. 更新职业名显示
	UpdateClassNameDisplay();

	// 3. 构建关卡选择列表
	BuildLevelSelection();
}

/************************************************************************/
/* 内部调用                                                                     */
/************************************************************************/

void UHUD_LobbyScreen::UpdateClassNameDisplay()
{
	if (!m_classNameText)
	{
		return;
	}

	UDataTableManager* DTManager = UDataTableManager::Get(this);
	if (!DTManager)
	{
		m_classNameText->SetText(FText::FromString(TEXT("未知职业")));
		return;
	}

	UClassConfigInfoTable* ClassTable = Cast<UClassConfigInfoTable>(
		DTManager->GetDataTable(EDataTableType::DT_ClassConfigInfo));

	if (!ClassTable)
	{
		m_classNameText->SetText(FText::FromString(TEXT("未知职业")));
		return;
	}

	FClassConfigInfo Config;
	if (ClassTable->GetClassConfig(m_playerClassID, Config))
	{
		m_classNameText->SetText(FText::FromName(Config.ClassName));
	}
	else
	{
		m_classNameText->SetText(FText::FromString(TEXT("未知职业")));
	}
}

void UHUD_LobbyScreen::BuildLevelSelection()
{
	if (!m_levelButtonContainer)
	{
		return;
	}

	// 清空旧按钮
	m_levelButtonContainer->ClearChildren();
	m_levelButtons.Empty();
	m_buttonLevelMap.Empty();

	// 获取波次配置表
	UDataTableManager* DTManager = UDataTableManager::Get(this);
	if (!DTManager)
	{
		return;
	}

	UWaveConfigInfoTable* WaveTable = Cast<UWaveConfigInfoTable>(
		DTManager->GetDataTable(EDataTableType::DT_WaveConfigInfo));

	if (!WaveTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHUD_LobbyScreen::BuildLevelSelection - WaveConfigInfoTable not found"));
		return;
	}

	// 获取所有关卡ID（升序）
	TArray<int32> LevelIDs = WaveTable->GetAllLevelIDs();
	if (LevelIDs.Num() == 0)
	{
		return;
	}

	// 为每个关卡创建按钮
	for (int32 LevelID : LevelIDs)
	{
		// 获取关卡显示名：取该关卡第一波次的名称
		FText DisplayName;
		{
			TArray<FWaveConfigInfo> Waves;
			if (WaveTable->GetWavesForLevel(LevelID, Waves) && Waves.Num() > 0)
			{
				DisplayName = FText::FromName(Waves[0].WaveName);
			}
			else
			{
				DisplayName = FText::Format(NSLOCTEXT("LobbyScreen", "LevelFallback", "关卡 {0}"), LevelID);
			}
		}

		UUI_ListItemBase* NewLevelItem = CreateLevelButton(LevelID, m_savedLevelID, DisplayName);
		if (NewLevelItem)
		{
			// 添加到容器
			UHorizontalBoxSlot* HSlot = m_levelButtonContainer->AddChildToHorizontalBox(NewLevelItem);
			if (HSlot)
			{
				HSlot->SetPadding(FMargin(8.0f, 0.0f));
			}

			// 注册
			m_levelButtons.Add(NewLevelItem);
			m_buttonLevelMap.Add(NewLevelItem, LevelID);

			// 绑定点击事件
			NewLevelItem->OnItemClicked.BindUObject(this, &UHUD_LobbyScreen::OnLevelClicked);
		}
	}
}

UUI_ListItemBase* UHUD_LobbyScreen::CreateLevelButton(int32 InLevelID, int32 InSavedLevelID, const FText& InDisplayName)
{
	// 创建按钮
	if (m_LevelSelectItemWidgetClass && GEngine->GameViewport)
	{
		UUI_ListItemBase* newListItem = CreateWidget<UUI_ListItemBase>(
			GetWorld(),
			m_LevelSelectItemWidgetClass
			);

		if (!newListItem)
		{
			return nullptr;
		}

		newListItem->SetDisplayText(InDisplayName);
		newListItem->SetItemData(InLevelID);
		// 根据关卡状态设置按钮属性
		if (InLevelID > InSavedLevelID)
		{
			// 未解锁
			newListItem->SetItemEnabled(false);
		}
		// 已通过（InLevelID < InSavedLevelID）和可挑战（InLevelID == InSavedLevelID）保持启用

		return newListItem;
	}
	return nullptr;
}

void UHUD_LobbyScreen::OnLevelButtonClicked(int32 InLevelIndex)
{
	OnLevelClicked(InLevelIndex);
}

void UHUD_LobbyScreen::OnLevelClicked(int32 InLevelID)
{
	// 关卡选择后的跳转逻辑（由蓝图侧扩展或在此实现）
	UE_LOG(LogTemp, Log, TEXT("UHUD_LobbyScreen::OnLevelClicked - Level %d selected"), InLevelID);

	// 预留：通过关卡ID加载对应关卡
		// 获取GameMode并加载存档
	if (ASL_GameModeBase* GameMode = Cast<ASL_GameModeBase>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		// 移除当前界面
		RemoveFromParent();
		
		// 从存档加载游戏
		GameMode->StartTargetLevel(InLevelID);
	}
}

/************************************************************************/
/*                               输入模式                                */
/************************************************************************/

void UHUD_LobbyScreen::SetUIInputMode()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		// UE4.26: 设置输入模式为UI Only
		// 只显示鼠标，不处理游戏输入
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);

		// 显示鼠标光标
		PlayerController->bShowMouseCursor = true;
	}
}

void UHUD_LobbyScreen::SetGameInputMode()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		// UE4.26: 恢复为游戏输入模式
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);

		// 隐藏鼠标光标
		PlayerController->bShowMouseCursor = false;
	}
}