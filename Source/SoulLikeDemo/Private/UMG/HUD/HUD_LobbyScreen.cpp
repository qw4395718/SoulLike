// Fill out your copyright notice in the Description page of Project Settings.

#include "HUD_LobbyScreen.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Components/ScrollBox.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/DataTable.h"
#include "Styling/CoreStyle.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"

/************************************************************************/
/*                               槽位默认显示名称                        */
/************************************************************************/

static const TMap<ELobbyEquipSlotType, FText> SlotDefaultNameMap = {
	{ ELobbyEquipSlotType::Weapon, FText::FromString(TEXT("武器")) },
	{ ELobbyEquipSlotType::Head,   FText::FromString(TEXT("头部")) },
	{ ELobbyEquipSlotType::Chest,  FText::FromString(TEXT("胸部")) },
	{ ELobbyEquipSlotType::Hands,  FText::FromString(TEXT("手部")) },
	{ ELobbyEquipSlotType::Legs,   FText::FromString(TEXT("腿部")) },
	{ ELobbyEquipSlotType::Feet,   FText::FromString(TEXT("脚部")) },
};

UHUD_LobbyScreen::UHUD_LobbyScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, m_currentSelectedSlot(ELobbyEquipSlotType::Weapon)
	, m_lastHoveredSlot(ELobbyEquipSlotType::Max)
{
}

/************************************************************************/
/* 继承实现                                                                     */
/************************************************************************/

void UHUD_LobbyScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// 绑定Tab按钮点击事件
	if (m_tabEquipButton)
	{
		m_tabEquipButton->OnClicked.AddDynamic(this, &UHUD_LobbyScreen::OnTabEquipClicked);
	}

	if (m_tabTeamButton)
	{
		m_tabTeamButton->OnClicked.AddDynamic(this, &UHUD_LobbyScreen::OnTabTeamClicked);
	}
}

void UHUD_LobbyScreen::NativeConstruct()
{
	Super::NativeConstruct();

	// 设置UI输入模式
	SetUIInputMode();

	// 初始化大厅内容
	InitializeLobby();

	UE_LOG(LogTemp, Log, TEXT("UHUD_LobbyScreen::NativeConstruct - Lobby screen displayed"));
}

void UHUD_LobbyScreen::NativeDestruct()
{
	// 恢复游戏输入模式
	SetGameInputMode();

	Super::NativeDestruct();

	UE_LOG(LogTemp, Log, TEXT("UHUD_LobbyScreen::NativeDestruct - Lobby screen removed"));
}

void UHUD_LobbyScreen::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	// 获取鼠标屏幕位置
	float MouseX = 0.0f, MouseY = 0.0f;
	if (!PC->GetMousePosition(MouseX, MouseY)) return;
	const FVector2D MousePos(MouseX, MouseY);

	// 遍历所有槽位检测鼠标悬停
	ELobbyEquipSlotType HoveredSlot = ELobbyEquipSlotType::Max;
	for (const auto& Pair : m_slotTypeMap)
	{
		UUserWidget* SlotWidget = Pair.Key;
		if (!SlotWidget || !SlotWidget->IsVisible()) continue;

		const FGeometry SlotGeom = SlotWidget->GetCachedGeometry();
		if (SlotGeom.IsUnderLocation(MousePos))
		{
			HoveredSlot = Pair.Value;
			break;
		}
	}

	// 悬停状态变化时触发事件
	if (HoveredSlot != m_lastHoveredSlot)
	{
		// 离开上一个槽位
		if (m_lastHoveredSlot != ELobbyEquipSlotType::Max)
		{
			OnEquipSlotUnhovered(m_lastHoveredSlot);
		}
		// 进入新槽位
		if (HoveredSlot != ELobbyEquipSlotType::Max)
		{
			OnEquipSlotHovered(HoveredSlot);
		}
		m_lastHoveredSlot = HoveredSlot;
	}
}

FReply UHUD_LobbyScreen::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 仅处理左键点击
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	const FVector2D MousePos = InMouseEvent.GetScreenSpacePosition();

	// 遍历槽位检测点击
	for (const auto& Pair : m_slotTypeMap)
	{
		UUserWidget* SlotWidget = Pair.Key;
		if (!SlotWidget || !SlotWidget->IsVisible()) continue;

		const FGeometry SlotGeom = SlotWidget->GetCachedGeometry();
		if (SlotGeom.IsUnderLocation(MousePos))
		{
			SelectEquipSlot(Pair.Value);
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

/************************************************************************/
/* 外部调用                                                                     */
/************************************************************************/

void UHUD_LobbyScreen::InitializeLobby()
{
	// 创建装备槽位列表
	CreateEquipSlots();

	// 默认选中武器槽位
	if (FLobbyEquipmentData* DefaultData = m_currentEquipmentMap.Find(ELobbyEquipSlotType::Weapon))
	{
		SelectEquipSlot(ELobbyEquipSlotType::Weapon);
	}
}

void UHUD_LobbyScreen::SetMissionLevel(const FText& InLevelText)
{
	if (m_missionLevelText)
	{
		// 发光（Outer Glow）效果建议在蓝图TextBlock的Font→Outline Settings中配置
		m_missionLevelText->SetText(InLevelText);
	}
}

void UHUD_LobbyScreen::SetClearTime(const FText& InTimeText)
{
	if (m_clearTimeText)
	{
		m_clearTimeText->SetText(InTimeText);
	}
}

void UHUD_LobbyScreen::LoadEquipmentFromDataTable(UDataTable* InDataTable)
{
	if (!InDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UHUD_LobbyScreen::LoadEquipmentFromDataTable - InDataTable is null"));
		return;
	}

	// 清空旧数据
	m_currentEquipmentMap.Empty();

	// 获取DataTable中所有行数据
	static const FString ContextStr(TEXT("UHUD_LobbyScreen::LoadEquipmentFromDataTable"));
	TArray<FLobbyEquipmentData*> AllRows;
	InDataTable->GetAllRows<FLobbyEquipmentData>(ContextStr, AllRows);

	for (const FLobbyEquipmentData* Row : AllRows)
	{
		if (Row)
		{
			m_currentEquipmentMap.Add(Row->SlotType, *Row);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UHUD_LobbyScreen::LoadEquipmentFromDataTable - Loaded %d equipment entries"), AllRows.Num());

	// 刷新所有槽位显示
	for (UUserWidget* SlotWidget : m_equipSlotWidgets)
	{
		if (!SlotWidget) continue;

		// 通过映射表获取槽位类型（替代GetTag）
		const ELobbyEquipSlotType* SlotTypePtr = m_slotTypeMap.Find(SlotWidget);
		if (!SlotTypePtr) continue;
		const ELobbyEquipSlotType SlotType = *SlotTypePtr;

		// 查找对应的装备数据并更新槽位显示
		if (const FLobbyEquipmentData* EquipData = m_currentEquipmentMap.Find(SlotType))
		{
			// 更新槽位图标和名称
			UImage* SlotIcon = Cast<UImage>(SlotWidget->GetWidgetFromName(TEXT("SlotIcon")));
			if (SlotIcon && !EquipData->Icon.IsNull())
			{
				UTexture2D* IconTexture = EquipData->Icon.LoadSynchronous();
				if (IconTexture)
				{
					SlotIcon->SetBrushFromTexture(IconTexture);
				}
			}

			UTextBlock* SlotName = Cast<UTextBlock>(SlotWidget->GetWidgetFromName(TEXT("SlotName")));
			if (SlotName)
			{
				SlotName->SetText(EquipData->EquipmentName);
			}
		}
	}

	// 刷新当前选中的槽位详情
	SelectEquipSlot(m_currentSelectedSlot);
}

void UHUD_LobbyScreen::SelectEquipSlot(ELobbyEquipSlotType InSlotType)
{
	m_currentSelectedSlot = InSlotType;

	// 高亮当前选中的槽位（通过设置透明度/颜色变化）
	for (UUserWidget* SlotWidget : m_equipSlotWidgets)
	{
		if (!SlotWidget) continue;

		const ELobbyEquipSlotType* SlotTypePtr = m_slotTypeMap.Find(SlotWidget);
		if (!SlotTypePtr) continue;
		const ELobbyEquipSlotType SlotType = *SlotTypePtr;

		// 通过Widget的RenderOpacity来简单区分选中/未选中
		// 更复杂的高亮效果（边框变色等）建议在蓝色中通过动画实现
		SlotWidget->SetRenderOpacity(SlotType == InSlotType ? 1.0f : 0.6f);
	}

	// 刷新右侧详细面板
	if (const FLobbyEquipmentData* EquipData = m_currentEquipmentMap.Find(InSlotType))
	{
		RefreshDetailPanel(EquipData);
	}
	else
	{
		// 该槽位无装备数据，清空详情面板
		if (m_attackPowerBar) m_attackPowerBar->SetPercent(0.0f);
		if (m_attackPowerText) m_attackPowerText->SetText(FText::GetEmpty());
		if (m_criticalRateBar) m_criticalRateBar->SetPercent(0.0f);
		if (m_criticalRateText) m_criticalRateText->SetText(FText::GetEmpty());
		ClearExtraAttributeItems();
	}
}

void UHUD_LobbyScreen::SwitchTab(int32 InTabIndex)
{
	if (m_mainContentSwitcher)
	{
		m_mainContentSwitcher->SetActiveWidgetIndex(InTabIndex);
	}
}

/************************************************************************/
/* 内部调用                                                                     */
/************************************************************************/

void UHUD_LobbyScreen::CreateEquipSlots()
{
	if (!m_equipSlotContainer) return;

	// 清除已有的槽位
	m_equipSlotContainer->ClearChildren();
	m_equipSlotWidgets.Empty();

	// 遍历所有装备槽位类型（不含Max）
	for (uint8 SlotTypeValue = 0; SlotTypeValue < static_cast<uint8>(ELobbyEquipSlotType::Max); ++SlotTypeValue)
	{
		const ELobbyEquipSlotType SlotType = static_cast<ELobbyEquipSlotType>(SlotTypeValue);

		// 使用预设的槽位Widget类创建槽位
		UUserWidget* SlotWidget = nullptr;

		if (m_equipSlotWidgetClass)
		{
			SlotWidget = CreateWidget<UUserWidget>(this, m_equipSlotWidgetClass);
		}

		// 如果没有预设类，使用默认的简单控件布局
		if (!SlotWidget)
		{
			// 在WidgetTree中创建一个简单的HorizontalBox作为槽位
			UHorizontalBox* DefaultSlot = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
			if (!DefaultSlot) continue;

			// 创建图标Image
			UImage* SlotIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("SlotIcon"));
			if (SlotIcon)
			{
				// 设置一个默认的占位颜色
				SlotIcon->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
				// 直接访问Brush属性（UE4.26中UImage无GetBrush方法）
				SlotIcon->Brush.ImageSize = FVector2D(48.0f, 48.0f);
				DefaultSlot->AddChild(SlotIcon);
			}

			// 创建名称TextBlock
			UTextBlock* SlotName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SlotName"));
			if (SlotName)
			{
				const FText* DefaultName = SlotDefaultNameMap.Find(SlotType);
				SlotName->SetText(DefaultName ? *DefaultName : FText::GetEmpty());
				SlotName->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 16));
				DefaultSlot->AddChild(SlotName);
			}

			// 将默认槽位添加到容器（UHorizontalBox无悬停能力，不加入m_slotTypeMap）
			m_equipSlotContainer->AddChild(DefaultSlot);
			continue;
		}

		// 建立槽位Widget → 类型映射（替代SetTag）
		m_slotTypeMap.Add(SlotWidget, SlotType);

		// 设置默认显示名称
		UTextBlock* SlotName = Cast<UTextBlock>(SlotWidget->GetWidgetFromName(TEXT("SlotName")));
		if (SlotName)
		{
			const FText* DefaultName = SlotDefaultNameMap.Find(SlotType);
			if (DefaultName)
			{
				SlotName->SetText(*DefaultName);
			}
		}

		// 添加到容器
		m_equipSlotContainer->AddChild(SlotWidget);
		m_equipSlotWidgets.Add(SlotWidget);
	}
}

void UHUD_LobbyScreen::RefreshDetailPanel(const FLobbyEquipmentData* InEquipData)
{
	if (!InEquipData) return;

	// 更新攻击力
	if (m_attackPowerBar && InEquipData->AttackPowerMax > 0.0f)
	{
		m_attackPowerBar->SetPercent(InEquipData->AttackPower / InEquipData->AttackPowerMax);
	}
	if (m_attackPowerText)
	{
		const FString AttackStr = FString::Printf(TEXT("%.0f / %.0f"), InEquipData->AttackPower, InEquipData->AttackPowerMax);
		m_attackPowerText->SetText(FText::FromString(AttackStr));
	}

	// 更新会心率
	if (m_criticalRateBar && InEquipData->CriticalRateMax > 0.0f)
	{
		m_criticalRateBar->SetPercent(InEquipData->CriticalRate / InEquipData->CriticalRateMax);
	}
	if (m_criticalRateText)
	{
		const FString CritStr = FString::Printf(TEXT("%.1f%%"), InEquipData->CriticalRate);
		m_criticalRateText->SetText(FText::FromString(CritStr));
	}

	// 更新额外属性列表
	ClearExtraAttributeItems();
	for (const FLobbyEquipAttribute& Attr : InEquipData->ExtraAttributes)
	{
		CreateAttributeItem(Attr);
	}
}

void UHUD_LobbyScreen::ClearExtraAttributeItems()
{
	if (m_extraAttributeContainer)
	{
		m_extraAttributeContainer->ClearChildren();
	}
}

UUserWidget* UHUD_LobbyScreen::CreateAttributeItem(const FLobbyEquipAttribute& InAttribute)
{
	if (!m_extraAttributeContainer) return nullptr;

	// 使用预设的属性条目Widget类
	if (m_attributeItemWidgetClass)
	{
		UUserWidget* AttrItem = CreateWidget<UUserWidget>(this, m_attributeItemWidgetClass);
		if (AttrItem)
		{
			// 通过Name查找子控件并设置数据
			UTextBlock* AttrNameText = Cast<UTextBlock>(AttrItem->GetWidgetFromName(TEXT("AttrName")));
			if (AttrNameText)
			{
				AttrNameText->SetText(InAttribute.AttributeName);
			}

			UProgressBar* AttrBar = Cast<UProgressBar>(AttrItem->GetWidgetFromName(TEXT("AttrBar")));
			if (AttrBar && InAttribute.MaxValue > 0.0f)
			{
				AttrBar->SetPercent(InAttribute.CurrentValue / InAttribute.MaxValue);
			}

			UTextBlock* AttrValueText = Cast<UTextBlock>(AttrItem->GetWidgetFromName(TEXT("AttrValue")));
			if (AttrValueText)
			{
				const FString ValueStr = FString::Printf(TEXT("%.0f / %.0f"), InAttribute.CurrentValue, InAttribute.MaxValue);
				AttrValueText->SetText(FText::FromString(ValueStr));
			}

			m_extraAttributeContainer->AddChild(AttrItem);
			return AttrItem;
		}
	}

	// 无预设类时使用默认简单构建
	UHorizontalBox* DefaultAttrRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	if (!DefaultAttrRow) return nullptr;

	// 属性名称
	UTextBlock* AttrNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AttrName"));
	if (AttrNameText)
	{
		AttrNameText->SetText(InAttribute.AttributeName);
		AttrNameText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14));
		DefaultAttrRow->AddChild(AttrNameText);
	}

	// 属性ProgressBar
	UProgressBar* AttrBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("AttrBar"));
	if (AttrBar && InAttribute.MaxValue > 0.0f)
	{
		AttrBar->SetPercent(InAttribute.CurrentValue / InAttribute.MaxValue);
		AttrBar->SetRenderTransformPivot(FVector2D(0.0f, 0.5f));
		DefaultAttrRow->AddChild(AttrBar);
	}

	// 属性数值文本
	UTextBlock* AttrValueText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AttrValue"));
	if (AttrValueText)
	{
		const FString ValueStr = FString::Printf(TEXT("%.0f / %.0f"), InAttribute.CurrentValue, InAttribute.MaxValue);
		AttrValueText->SetText(FText::FromString(ValueStr));
		AttrValueText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14));
		DefaultAttrRow->AddChild(AttrValueText);
	}

	m_extraAttributeContainer->AddChild(DefaultAttrRow);
	return nullptr; // 默认布局返回null（非UUserWidget）
}

/************************************************************************/
/*                               按钮事件                                */
/************************************************************************/

void UHUD_LobbyScreen::OnTabEquipClicked()
{
	SwitchTab(0);
	UE_LOG(LogTemp, Log, TEXT("UHUD_LobbyScreen::OnTabEquipClicked - Switch to equipment tab"));
}

void UHUD_LobbyScreen::OnTabTeamClicked()
{
	SwitchTab(1);
	UE_LOG(LogTemp, Log, TEXT("UHUD_LobbyScreen::OnTabTeamClicked - Switch to team tab"));
}

/************************************************************************/
/*                               悬停事件                                */
/************************************************************************/

void UHUD_LobbyScreen::OnEquipSlotHovered(ELobbyEquipSlotType InSlotType)
{
	// 鼠标悬停时，更新右侧详情面板为对应的装备数据
	if (const FLobbyEquipmentData* EquipData = m_currentEquipmentMap.Find(InSlotType))
	{
		RefreshDetailPanel(EquipData);
	}

	UE_LOG(LogTemp, Verbose, TEXT("UHUD_LobbyScreen::OnEquipSlotHovered - Slot %d"), static_cast<uint8>(InSlotType));
}

void UHUD_LobbyScreen::OnEquipSlotUnhovered(ELobbyEquipSlotType InSlotType)
{
	// 鼠标离开槽位时，恢复显示当前选中的装备详情
	SelectEquipSlot(m_currentSelectedSlot);

	UE_LOG(LogTemp, Verbose, TEXT("UHUD_LobbyScreen::OnEquipSlotUnhovered - Slot %d"), static_cast<uint8>(InSlotType));
}

/************************************************************************/
/*                               输入模式                                */
/************************************************************************/

void UHUD_LobbyScreen::SetUIInputMode()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);

		PlayerController->bShowMouseCursor = true;
	}
}

void UHUD_LobbyScreen::SetGameInputMode()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);

		PlayerController->bShowMouseCursor = false;
	}
}
