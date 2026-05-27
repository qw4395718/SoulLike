#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD_PauseMenuScreen.generated.h"

class UTextBlock;
class UVerticalBox;
class UButton;

/**
 * 暂停菜单界面Widget
 *
 * 布局：
 *   中间偏上 —— 标题文本
 *   标题下方 —— VerticalBox 内放置功能按钮（每个按钮内含 Text 描述功能）
 *
 * 功能按钮：
 *   1. 继续游戏（关闭菜单并恢复游戏）
 *   2. 回到大厅（打开 LobbyScreen）
 *   3. 回到初始界面（打开 BeginPlayScreen）
 *   4. 退出游戏
 *
 * 触发规则：
 *   - 按 ESC 唤出（由外部 PlayerController 或 ActionBinding 调用 OpenPauseMenu）
 *   - 在 BeginPlayScreen 已打开时无法唤出
 *   - 再次按 ESC 或点击"继续游戏"关闭
 *   - 打开时游戏暂停（SetPause）
 *   - 关闭时恢复游戏
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class SOULLIKEDEMO_API UHUD_PauseMenuScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UHUD_PauseMenuScreen(const FObjectInitializer& ObjectInitializer);

	/************************************************************************/
	/* 继承实现                                                                     */
	/************************************************************************/
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 打开暂停菜单（检查 BeginPlayScreen 是否活跃，若活跃则不打开）
	UFUNCTION(BlueprintCallable, Category = "PauseMenu", meta = (WorldContext = "WorldContextObject"))
	static void OpenPauseMenu(const UObject* WorldContextObject);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	// 设置 UI Only 输入模式
	void SetUIInputMode();

	// 恢复 Game Only 输入模式
	void SetGameInputMode();

	/************************************************************************/
	/* 按钮事件                                                                     */
	/************************************************************************/
	UFUNCTION()
	void OnContinueClicked();

	UFUNCTION()
	void OnLobbyClicked();

	UFUNCTION()
	void OnMainMenuClicked();

	UFUNCTION()
	void OnQuitClicked();

protected:
	/************************************************************************/
	/* 内部变量 - BindWidget控件绑定                                                */
	/************************************************************************/

	// 标题文本（中间偏上）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "PauseMenu")
	UTextBlock* m_titleText;

	// 按钮容器
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "PauseMenu")
	UVerticalBox* m_buttonContainer;

	// 继续游戏按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "PauseMenu")
	UButton* m_continueButton;

	// 回到大厅按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "PauseMenu")
	UButton* m_lobbyButton;

	// 回到初始界面按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "PauseMenu")
	UButton* m_mainMenuButton;

	// 退出游戏按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "PauseMenu")
	UButton* m_quitButton;

	// UIManager 子系统引用
	UPROPERTY()
	class UUIManagerSubsystem* UIManager;
};
