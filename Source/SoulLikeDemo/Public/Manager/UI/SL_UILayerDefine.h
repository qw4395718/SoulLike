// SL_UILayerDefine.h
#pragma once

#include "CoreMinimal.h"
#include "SoulLikeGameGlobal.h"
#include "SL_UILayerDefine.generated.h"

/************************************************************************/
/* UI层级枚举                                                                     */
/************************************************************************/
UENUM(BlueprintType)
enum class EUILayer : uint8
{
	Background	UMETA(DisplayName = "背景层"),
	HUD			UMETA(DisplayName = "HUD层"),
	Overlay		UMETA(DisplayName = "叠加层"),
	Popup		UMETA(DisplayName = "弹窗层"),
	Modal		UMETA(DisplayName = "模态层"),
	FullScreen	UMETA(DisplayName = "全屏层"),
	Max			UMETA(Hidden)
};

/************************************************************************/
/* InputMode需求枚举                                                              */
/************************************************************************/
UENUM(BlueprintType)
enum class EUIInputModeRequirement : uint8
{
	Inherit		UMETA(DisplayName = "继承层级默认"),
	GameOnly	UMETA(DisplayName = "纯游戏模式"),
	GameAndUI	UMETA(DisplayName = "游戏+UI混合"),
	UIOnly		UMETA(DisplayName = "纯UI模式"),
	Max			UMETA(Hidden)
};

/************************************************************************/
/* 层级配置结构                                                                   */
/************************************************************************/
USTRUCT(BlueprintType)
struct FUIConfigEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EUIInputModeRequirement DefaultInputMode = EUIInputModeRequirement::GameOnly;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 BaseZOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bBlockUnderlying = false;
};

/************************************************************************/
/* 层级配置查询（内联函数，纯头文件实现，不产生额外编译单元）                                  */
/************************************************************************/
namespace UILayerUtils
{
	constexpr int32 LayerStep = 50;

	// 获取层级配置
	inline FUIConfigEntry GetLayerConfig(EUILayer InLayer)
	{
		FUIConfigEntry Config;
		switch (InLayer)
		{
		case EUILayer::Background:
			Config.BaseZOrder = 0;
			Config.DefaultInputMode = EUIInputModeRequirement::GameOnly;
			Config.bBlockUnderlying = false;
			break;
		case EUILayer::HUD:
			Config.BaseZOrder = 50;
			Config.DefaultInputMode = EUIInputModeRequirement::GameOnly;
			Config.bBlockUnderlying = false;
			break;
		case EUILayer::Overlay:
			Config.BaseZOrder = 100;
			Config.DefaultInputMode = EUIInputModeRequirement::UIOnly;
			Config.bBlockUnderlying = false;
			break;
		case EUILayer::Popup:
			Config.BaseZOrder = 150;
			Config.DefaultInputMode = EUIInputModeRequirement::GameAndUI;
			Config.bBlockUnderlying = true;
			break;
		case EUILayer::Modal:
			Config.BaseZOrder = 200;
			Config.DefaultInputMode = EUIInputModeRequirement::UIOnly;
			Config.bBlockUnderlying = true;
			break;
		case EUILayer::FullScreen:
			Config.BaseZOrder = 250;
			Config.DefaultInputMode = EUIInputModeRequirement::UIOnly;
			Config.bBlockUnderlying = true;
			break;
		default:
			break;
		}
		return Config;
	}

	// EWidgetType → EUILayer 映射
	inline EUILayer GetLayerForWidgetType(EWidgetType InType)
	{
		switch (InType)
		{
		case EWidgetType::EWIDGET_PawnStatusInScreen:
		case EWidgetType::EWIDGET_LockOnIndicator:
			return EUILayer::Background;

		case EWidgetType::EWIDGET_PlayerStatus:
		case EWidgetType::EWIDGET_BossStatus:
		case EWidgetType::EWIDGET_ItemUseUI:
			return EUILayer::HUD;

		case EWidgetType::EWIDGET_InterActPanel:
		case EWidgetType::EWIDGET_NPCDialog:
		case EWidgetType::EWIDGET_Inventory:
			return EUILayer::Popup;

		case EWidgetType::EWIDGET_PauseMenu:
		case EWidgetType::EWIDGET_DeathScreen:
		case EWidgetType::EWIDGET_LevelComplete:
			return EUILayer::Modal;

		case EWidgetType::EWIDGET_BeginPlayScreen:
		case EWidgetType::EWIDGET_LobbyScreen:
		case EWidgetType::EWIDGET_ClassSelectScreen:
			return EUILayer::FullScreen;

		case EWidgetType::EWIDGET_MainMenu:
			return EUILayer::Popup;

		default:
			return EUILayer::Background;
		}
	}

	// 根据 EWidgetType 获取基准 ZOrder
	inline int32 GetBaseZOrderForWidget(EWidgetType InType)
	{
		return GetLayerConfig(GetLayerForWidgetType(InType)).BaseZOrder;
	}

}
