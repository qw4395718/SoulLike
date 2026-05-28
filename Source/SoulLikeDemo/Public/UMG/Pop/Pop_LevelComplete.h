// Pop_LevelComplete.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Pop_LevelComplete.generated.h"

/**
 * 关卡完成界面 Widget
 *
 * 触发时机：OnAllWavesCompleted() 后自动弹出
 *
 * 布局（蓝图侧）：
 *   视野中央：Image / Text（关卡完成提示）
 *   下方：三个 Button —— 挑战下一关 / 返回大厅 / 退出游戏
 *
 * 按钮逻辑（C++ 侧）：
 *   挑战下一关 → LevelManager::StartLevel(CurrentLevelID + 1)
 *   返回大厅    → 打开 LobbyScreen
 *   退出游戏    → UKismetSystemLibrary::QuitGame()
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class SOULLIKEDEMO_API UPop_LevelComplete : public UUserWidget
{
	GENERATED_BODY()

public:
	UPop_LevelComplete(const FObjectInitializer& ObjectInitializer);

	/************************************************************************/
	/*                               继承实现                               */
	/************************************************************************/
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	/************************************************************************/
	/*                               内部调用                               */
	/************************************************************************/
	// 设置 UI Only 输入模式（显示鼠标，禁用游戏输入）
	void SetUIInputMode();

	// 恢复 Game Only 输入模式
	void SetGameInputMode();

	// 根据是否有下一关更新按钮状态
	void UpdateButtonStates();

	/************************************************************************/
	/*                               按钮事件                               */
	/************************************************************************/
	UFUNCTION()
	void OnNextLevelClicked();

	UFUNCTION()
	void OnLobbyClicked();

	UFUNCTION()
	void OnQuitClicked();

protected:
	/************************************************************************/
	/*                          BindWidget 控件                             */
	/************************************************************************/
	// 挑战下一关按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* m_nextLevelButton;

	// 返回大厅按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* m_lobbyButton;

	// 退出游戏按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* m_quitButton;

	// UIManager 子系统引用
	UPROPERTY()
	class UUIManagerSubsystem* UIManager;
};
