#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD_LobbyScreen.generated.h"

class UTextBlock;
class UHorizontalBox;
class UUI_ListItemBase;
class UButton;
class UScrollBox;

/**
 * 大厅界面Widget（完全重写版）
 *
 * 布局：
 *   左上角 TextBlock —— 显示当前职业名
 *   中间 HorizontalBox —— 关卡选择按钮列表
 *
 * 关卡状态规则（基于存档 SL_GameSaveData::SavedLevelID）：
 *   LevelID < SavedLevelID  -> 已通过
 *   LevelID == SavedLevelID -> 可进行挑战
 *   LevelID > SavedLevelID  -> 未解锁（bIsEnabled = false）
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

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 初始化大厅：加载存档 -> 显示职业名 -> 构建关卡选择列表
	UFUNCTION(BlueprintCallable, Category = "LobbyScreen")
	void InitializeLobby();

	// 供 ClassSelectScreen 在切换职业后调用，刷新职业名显示
	void RefreshClassDisplay();

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	// 从存档和职业配置表更新左上角职业名
	void UpdateClassNameDisplay();

	// 从波次配置表中提取所有关卡ID，构建关卡选择按钮
	void BuildLevelSelection();

	// 为指定关卡ID创建单个选择按钮
	UUI_ListItemBase* CreateLevelButton(int32 InLevelID, int32 InSavedLevelID, const FText& InDisplayName);

	// 打开职业选择界面
	void OnSwitchClassClicked();

	// 关卡按钮点击响应
	void OnLevelClicked(int32 InLevelID);

	void SetUIInputMode();
	void SetGameInputMode();
	// 所有按钮共用的点击回调
	UFUNCTION()
	void OnLevelButtonClicked(int32 InLevelIndex);

protected:
	/************************************************************************/
	/* 内部变量 - BindWidget控件绑定                                                */
	/************************************************************************/

	// 左上角 - 职业名文本
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
		UTextBlock* m_classNameText;

	// 左上角 - 切换职业按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
		UButton* m_switchClassButton;

	// 中间 - 水平框的容器
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		UScrollBox* m_scrollBox;

	// 中间 - 关卡选择按钮水平容器
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
		UHorizontalBox* m_levelButtonContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "LobbyScreen")
		TSubclassOf<UUI_ListItemBase> m_LevelSelectItemWidgetClass;

protected:
	/************************************************************************/
	/* 内部变量 - 数据                                                             */
	/************************************************************************/

	UPROPERTY()
		class UUIManagerSubsystem* UIManager;

	// 存档中的可挑战关卡ID
	int32 m_savedLevelID;

	// 存档中的玩家职业ID
	int32 m_playerClassID;

	// 已创建的关卡按钮列表（按LevelID升序）
	UPROPERTY()
	TArray<UUI_ListItemBase*> m_levelButtons;

	// 按钮 -> 关卡ID 映射（用于点击回调识别）
	UPROPERTY()
	TMap<UUI_ListItemBase*, int32> m_buttonLevelMap;
};