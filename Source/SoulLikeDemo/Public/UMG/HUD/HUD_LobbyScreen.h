// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataTable.h"
#include "HUD_LobbyScreen.generated.h"

class UTextBlock;
class UButton;
class UWidgetSwitcher;
class UScrollBox;
class UVerticalBox;
class UProgressBar;
class UImage;

/************************************************************************/
/*                               装备槽位类型                            */
/************************************************************************/

UENUM(BlueprintType)
enum class ELobbyEquipSlotType : uint8
{
	Weapon		UMETA(DisplayName = "武器"),
	Head		UMETA(DisplayName = "头部"),
	Chest		UMETA(DisplayName = "胸部"),
	Hands		UMETA(DisplayName = "手部"),
	Legs		UMETA(DisplayName = "腿部"),
	Feet		UMETA(DisplayName = "脚部"),
	Max			UMETA(Hidden)
};

/************************************************************************/
/*                               属性条目                               */
/************************************************************************/

USTRUCT(BlueprintType)
struct FLobbyEquipAttribute
{
	GENERATED_BODY()

	// 属性名称（如"火抗"、"冰抗"）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	FText AttributeName;

	// 当前值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float CurrentValue;

	// 最大值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attribute")
	float MaxValue;
};

/************************************************************************/
/*                               装备数据行(DataTable)                  */
/************************************************************************/

USTRUCT(BlueprintType)
struct FLobbyEquipmentData : public FTableRowBase
{
	GENERATED_BODY()

	// 槽位类型（决定装备挂在哪个槽位）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	ELobbyEquipSlotType SlotType;

	// 装备名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FText EquipmentName;

	// 装备图标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TSoftObjectPtr<UTexture2D> Icon;

	// 攻击力
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float AttackPower;

	// 攻击力上限（用于ProgressBar基准）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float AttackPowerMax;

	// 会心率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float CriticalRate;

	// 会心率上限（用于ProgressBar基准）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	float CriticalRateMax;

	// 额外属性列表（火抗、冰抗等）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TArray<FLobbyEquipAttribute> ExtraAttributes;
};

/**
 * 大厅界面Widget
 * 包含：顶部状态栏（任务等级、通关计时）、左侧Tab导航（选择装备/队伍状况）、
 *       装备详情面板（装备列表 + 详细数值）、角色模型锚点
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class SOULLIKEDEMO_API UHUD_LobbyScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UHUD_LobbyScreen(const FObjectInitializer& ObjectInitializer);

	/************************************************************************/
	/* 继承实现                                                                     */
	/************************************************************************/
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 初始化大厅界面：创建装备槽位、加载默认数据
	UFUNCTION(BlueprintCallable, Category = "LobbyScreen")
	void InitializeLobby();

	// 设置任务等级文本（如 "Master 06"）
	UFUNCTION(BlueprintCallable, Category = "LobbyScreen")
	void SetMissionLevel(const FText& InLevelText);

	// 设置通关计时（如 "02:49"90"）
	UFUNCTION(BlueprintCallable, Category = "LobbyScreen")
	void SetClearTime(const FText& InTimeText);

	// 从DataTable加载所有装备数据
	UFUNCTION(BlueprintCallable, Category = "LobbyScreen")
	void LoadEquipmentFromDataTable(UDataTable* InDataTable);

	// 选中并显示指定槽位的装备详情
	UFUNCTION(BlueprintCallable, Category = "LobbyScreen")
	void SelectEquipSlot(ELobbyEquipSlotType InSlotType);

	// 切换Tab页（0=选择装备，1=队伍状况）
	UFUNCTION(BlueprintCallable, Category = "LobbyScreen")
	void SwitchTab(int32 InTabIndex);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	// 创建装备槽位列表
	void CreateEquipSlots();

	// 刷新右侧详细数值面板
	void RefreshDetailPanel(const FLobbyEquipmentData* InEquipData);

	// 清除额外属性条目（重新生成前调用）
	void ClearExtraAttributeItems();

	// 创建一个属性条目控件并添加到容器
	UUserWidget* CreateAttributeItem(const FLobbyEquipAttribute& InAttribute);

	// 切换Tab事件 - 选择装备
	UFUNCTION()
	void OnTabEquipClicked();

	// 切换Tab事件 - 队伍状况
	UFUNCTION()
	void OnTabTeamClicked();

	// 装备槽位悬停事件
	void OnEquipSlotHovered(ELobbyEquipSlotType InSlotType);

	// 装备槽位离开事件
	void OnEquipSlotUnhovered(ELobbyEquipSlotType InSlotType);

	// 设置UI Only输入模式
	void SetUIInputMode();

	// 恢复Game Only输入模式
	void SetGameInputMode();

protected:
	/************************************************************************/
	/* 内部变量 - BindWidget控件绑定                                                */
	/************************************************************************/

	// 顶部 - 任务等级
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UTextBlock* m_missionLevelText;

	// 顶部 - 通关计时
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UTextBlock* m_clearTimeText;

	// 左侧 - 选择装备Button
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UButton* m_tabEquipButton;

	// 左侧 - 队伍状况Button
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UButton* m_tabTeamButton;

	// 主面板 - WidgetSwitcher（0=装备详情，1=队伍状况）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UWidgetSwitcher* m_mainContentSwitcher;

	// 装备列表ScrollBox
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UScrollBox* m_equipListScrollBox;

	// 装备列表中的槽位容器
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UVerticalBox* m_equipSlotContainer;

	// 右栏 - 攻击力ProgressBar
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UProgressBar* m_attackPowerBar;

	// 右栏 - 攻击力数值文本
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UTextBlock* m_attackPowerText;

	// 右栏 - 会心率ProgressBar
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UProgressBar* m_criticalRateBar;

	// 右栏 - 会心率数值文本
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UTextBlock* m_criticalRateText;

	// 右栏 - 额外属性条目容器
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UVerticalBox* m_extraAttributeContainer;

	// 角色模型锚点（蓝图侧可通过此Image绑定SceneCapture的RenderTarget）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UImage* m_modelDisplayImage;

	// 装备详情面板的背景Image（半透明磨砂材质由蓝图侧设置）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UImage* m_detailPanelBackground;

protected:
	/************************************************************************/
	/* 内部变量 - 数据                                                             */
	/************************************************************************/
	// 当前选中的装备槽位类型
	UPROPERTY()
	ELobbyEquipSlotType m_currentSelectedSlot;

	// 当前装备数据映射（槽位类型 -> 装备数据）
	UPROPERTY()
	TMap<ELobbyEquipSlotType, FLobbyEquipmentData> m_currentEquipmentMap;

	// 槽位Widget类（需在蓝图或C++中指定WBP_EquipSlot）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LobbyScreen")
	TSubclassOf<UUserWidget> m_equipSlotWidgetClass;

	// 属性条目Widget类（需在蓝图或C++中指定WBP_AttributeItem）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LobbyScreen")
	TSubclassOf<UUserWidget> m_attributeItemWidgetClass;

	// 动态创建的装备槽位列表（用于遍历更新）
	UPROPERTY()
	TArray<UUserWidget*> m_equipSlotWidgets;

	// 槽位Widget → 槽位类型映射（替代 GetTag/SetTag）
	UPROPERTY()
	TMap<UUserWidget*, ELobbyEquipSlotType> m_slotTypeMap;

	// 上一次悬停的槽位类型（用于 NativeTick 状态变化检测）
	ELobbyEquipSlotType m_lastHoveredSlot;
};
