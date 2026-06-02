// Private/UMG/HUD/HUD_ClassSelectScreen.cpp

#include "HUD_ClassSelectScreen.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Manager/SL_GameSaveSubsystem.h"
#include "Manager/DataTableManager.h"
#include "Manager/UIManagerSubsystem.h"
#include "Table/ClassConfigInfoTable.h"
#include "SoulLikeGameGlobal.h"
#include "UI_ListItemBase.h"
#include "HUD_LobbyScreen.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

UHUD_ClassSelectScreen::UHUD_ClassSelectScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, m_playerClassID(1001)
	, m_selectedClassID(1001)
	, m_lastSelectedButton(nullptr)
{
	bIsFocusable = true;
}

/************************************************************************/
/* 继承实现                                                                     */
/************************************************************************/

void UHUD_ClassSelectScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 绑定关闭按钮
	if (m_closeButton)
	{
		m_closeButton->OnClicked.AddDynamic(this, &UHUD_ClassSelectScreen::OnCloseClicked);
	}

	// 绑定确认按钮
	if (m_confirmButton)
	{
		m_confirmButton->OnClicked.AddDynamic(this, &UHUD_ClassSelectScreen::OnConfirmClicked);
	}
}

void UHUD_ClassSelectScreen::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化界面
	InitializeSelectScreen();
}

void UHUD_ClassSelectScreen::NativeDestruct()
{
	if (m_closeButton)
	{
		m_closeButton->OnClicked.RemoveDynamic(this, &UHUD_ClassSelectScreen::OnCloseClicked);
	}

	if (m_confirmButton)
	{
		m_confirmButton->OnClicked.RemoveDynamic(this, &UHUD_ClassSelectScreen::OnConfirmClicked);
	}

	// 通知 UIManager
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_ClassSelectScreen);
	}

	Super::NativeDestruct();
}

/************************************************************************/
/* 外部调用                                                                     */
/************************************************************************/

void UHUD_ClassSelectScreen::InitializeSelectScreen()
{
	// 1. 读取存档，获取当前职业ID
	USL_GameSaveSubsystem* SaveSystem = USL_GameSaveSubsystem::Get(this);
	if (SaveSystem)
	{
		int32 LoadedLevelID = 1;
		int32 LoadedClassID = 1001;
		SaveSystem->LoadGame(LoadedLevelID, LoadedClassID);
		m_playerClassID = LoadedClassID;
		m_selectedClassID = LoadedClassID;
	}

	// 2. 构建左侧职业按钮列表
	BuildClassList();

	// 3. 默认选中当前职业，更新右侧预览
	UpdatePreview(m_selectedClassID);
}

/************************************************************************/
/* 内部调用                                                                     */
/************************************************************************/

void UHUD_ClassSelectScreen::BuildClassList()
{
	if (!m_classListContainer)
	{
		return;
	}

	// 清空旧按钮
	m_classListContainer->ClearChildren();
	m_classButtons.Empty();
	m_buttonClassMap.Empty();
	m_lastSelectedButton = nullptr;

	// 获取职业配置表
	UDataTableManager* DTManager = UDataTableManager::Get(this);
	if (!DTManager)
	{
		return;
	}

	UClassConfigInfoTable* ClassTable = Cast<UClassConfigInfoTable>(
		DTManager->GetDataTable(EDataTableType::DT_ClassConfigInfo));

	if (!ClassTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHUD_ClassSelectScreen::BuildClassList - ClassConfigInfoTable not found"));
		return;
	}

	// 获取所有职业ID
	TArray<int32> ClassIDs = ClassTable->GetAllClassIDs();

	// 为每个职业创建按钮
	for (int32 ClassID : ClassIDs)
	{
		FClassConfigInfo Config;
		if (!ClassTable->GetClassConfig(ClassID, Config))
		{
			continue;
		}

		FText DisplayName = FText::FromName(Config.ClassName);
		UUI_ListItemBase* NewBtn = CreateClassButton(ClassID, DisplayName);
		if (NewBtn)
		{
			// 添加到 ScrollBox
			UVerticalBoxSlot* VSlot = m_classListContainer->AddChildToVerticalBox(NewBtn);
			if (VSlot)
			{
				VSlot->SetPadding(FMargin(4.0f, 4.0f));
			}

			// 注册
			m_classButtons.Add(NewBtn);
			m_buttonClassMap.Add(ClassID ,NewBtn);

			// 绑定点击事件
			NewBtn->OnItemClicked.BindUObject(this, &UHUD_ClassSelectScreen::OnClassButtonClicked);

			// 如果是当前职业，默认高亮
			if (ClassID == m_playerClassID)
			{
				NewBtn->SetSelected(true);
				m_lastSelectedButton = NewBtn;
			}
		}
	}
}

UUI_ListItemBase* UHUD_ClassSelectScreen::CreateClassButton(int32 InClassID, const FText& InDisplayName)
{
	if (m_ClassSelectItemWidgetClass && GEngine && GEngine->GameViewport)
	{
		UUI_ListItemBase* NewItem = CreateWidget<UUI_ListItemBase>(
			GetWorld(), m_ClassSelectItemWidgetClass);

		if (!NewItem)
		{
			return nullptr;
		}

		NewItem->SetDisplayText(InDisplayName);
		NewItem->SetItemData(InClassID);

		return NewItem;
	}
	return nullptr;
}

void UHUD_ClassSelectScreen::UpdatePreview(int32 InClassID)
{
	UDataTableManager* DTManager = UDataTableManager::Get(this);
	if (!DTManager)
	{
		return;
	}

	UClassConfigInfoTable* ClassTable = Cast<UClassConfigInfoTable>(
		DTManager->GetDataTable(EDataTableType::DT_ClassConfigInfo));

	if (!ClassTable)
	{
		return;
	}

	FClassConfigInfo Config;
	if (!ClassTable->GetClassConfig(InClassID, Config))
	{
		return;
	}

	// 更新职业名
	if (m_classNameText)
	{
		m_classNameText->SetText(FText::FromName(Config.ClassName));
	}

	// 更新职业描述
	if (m_classDescText)
	{
		if (!Config.ClassDescription.IsEmpty())
		{
			m_classDescText->SetText(Config.ClassDescription);
			m_classDescText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			m_classDescText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 更新预览图片
	if (m_previewImage)
	{
		if (!Config.ClassPreviewImage.IsNull())
		{
			UTexture2D* PreviewTex = Config.ClassPreviewImage.LoadSynchronous();
			if (PreviewTex)
			{
				m_previewImage->SetBrushFromTexture(PreviewTex);
				m_previewImage->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				m_previewImage->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
		else
		{
			m_previewImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 更新属性概览
	if (m_attrText)
	{
		FText AttrText = FText::Format(
			NSLOCTEXT("ClassSelect", "AttrFormat",
				"生命: {0}  攻击: {1}  防御: {2}  耐力: {3}  速度: {4}"),
			FText::AsNumber(Config.BaseHealth),
			FText::AsNumber(Config.BaseAttack),
			FText::AsNumber(Config.BaseDefense),
			FText::AsNumber(Config.BaseStamina),
			FText::AsNumber(Config.BaseMoveSpeed));
		m_attrText->SetText(AttrText);
	}
}

void UHUD_ClassSelectScreen::OnClassButtonClicked(int32 InClassID)
{
	// 查找对应的按钮
	UUI_ListItemBase* ClickedBtn = m_buttonClassMap.FindRef(InClassID);
	if (!ClickedBtn)
	{
		return;
	}

	// 清除上一个选中按钮的高亮
	if (m_lastSelectedButton && m_lastSelectedButton != ClickedBtn)
	{
		m_lastSelectedButton->SetSelected(false);
	}

	// 高亮当前按钮
	ClickedBtn->SetSelected(true);
	m_lastSelectedButton = ClickedBtn;

	// 更新选中ID
	m_selectedClassID = InClassID;

	// 同步更新右侧预览
	UpdatePreview(InClassID);

	UE_LOG(LogTemp, Log, TEXT("UHUD_ClassSelectScreen::OnClassButtonClicked - Selected ClassID: %d"), InClassID);
}

void UHUD_ClassSelectScreen::OnConfirmClicked()
{
	// 如果没变动，直接关闭
	if (m_selectedClassID == m_playerClassID)
	{
		CloseScreen();
		return;
	}

	// 保存新的职业ID到存档
	USL_GameSaveSubsystem* SaveSystem = USL_GameSaveSubsystem::Get(this);
	if (SaveSystem)
	{
		// 保持当前关卡ID不变，只更新职业ID
		int32 CurrentLevelID = 1;
		int32 DummyClassID = 1;
		SaveSystem->LoadGame(CurrentLevelID, DummyClassID);
		SaveSystem->SaveGame(CurrentLevelID, m_selectedClassID);

		UE_LOG(LogTemp, Log, TEXT("UHUD_ClassSelectScreen::OnConfirmClicked - Class changed from %d to %d"),
			m_playerClassID, m_selectedClassID);
	}

	// 通知 LobbyScreen 刷新职业名显示
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UUserWidget* FoundWidget = UIManager->GetWidget(EWidgetType::EWIDGET_LobbyScreen);
		if (FoundWidget)
		{
			if (UHUD_LobbyScreen* LobbyScreen = Cast<UHUD_LobbyScreen>(FoundWidget))
			{
				LobbyScreen->RefreshClassDisplay();
			}
		}
	}

	// 关闭界面
	CloseScreen();
}

void UHUD_ClassSelectScreen::OnCloseClicked()
{
	UE_LOG(LogTemp, Log, TEXT("UHUD_ClassSelectScreen::OnCloseClicked - Close without changes"));
	CloseScreen();
}

void UHUD_ClassSelectScreen::CloseScreen()
{
	// 通过 UIManager 关闭（移除视口 + 清理 ActiveWidgets）
	UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		UIManager->CloseScreenWidget(EWidgetType::EWIDGET_ClassSelectScreen);
	}
}
