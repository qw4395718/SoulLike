#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD_LobbyScreen.generated.h"

class UTextBlock;
class UHorizontalBox;
class UButton;

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

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 初始化大厅：加载存档 -> 显示职业名 -> 构建关卡选择列表
	UFUNCTION(BlueprintCallable, Category = "LobbyScreen")
	void InitializeLobby();

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	// 从存档和职业配置表更新左上角职业名
	void UpdateClassNameDisplay();

	// 从波次配置表中提取所有关卡ID，构建关卡选择按钮
	void BuildLevelSelection();

	// 为指定关卡ID创建单个选择按钮
	UButton* CreateLevelButton(int32 InLevelID, int32 InSavedLevelID, const FText& InDisplayName);

	// 关卡按钮点击响应
	void OnLevelClicked(int32 InLevelID);

	void SetUIInputMode();
	void SetGameInputMode();
	// 所有按钮共用的点击回调
	UFUNCTION()
	void OnLevelButtonClicked();

protected:
	/************************************************************************/
	/* 内部变量 - BindWidget控件绑定                                                */
	/************************************************************************/

	// 左上角 - 职业名文本
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UTextBlock* m_classNameText;

	// 中间 - 关卡选择按钮水平容器
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "LobbyScreen")
	UHorizontalBox* m_levelButtonContainer;

protected:
	/************************************************************************/
	/* 内部变量 - 数据                                                             */
	/************************************************************************/

	// 存档中的可挑战关卡ID
	int32 m_savedLevelID;

	// 存档中的玩家职业ID
	int32 m_playerClassID;

	// 已创建的关卡按钮列表（按LevelID升序）
	UPROPERTY()
	TArray<UButton*> m_levelButtons;

	// 按钮 -> 关卡ID 映射（用于点击回调识别）
	UPROPERTY()
	TMap<UButton*, int32> m_buttonLevelMap;
};
