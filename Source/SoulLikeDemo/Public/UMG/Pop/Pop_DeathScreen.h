// Pop_DeathScreen.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Pop_DeathScreen.generated.h"

/**
 * 死亡界面 Widget
 *
 * 布局（蓝图侧）：
 *   视野中央：Image（死亡图标）
 *   下方：三个 Button —— 重新挑战 / 返回大厅 / 退出游戏
 *
 * 按钮逻辑（C++ 侧）：
 *   重新挑战 → LevelManager::RetryLevel()
 *   返回大厅 → 打开 LobbyScreen（后续切换为大厅地图 OpenLevel）
 *   退出游戏 → UKismetSystemLibrary::QuitGame()
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class SOULLIKEDEMO_API UPop_DeathScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	UPop_DeathScreen(const FObjectInitializer& ObjectInitializer);

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

	/************************************************************************/
	/*                               按钮事件                               */
	/************************************************************************/
	UFUNCTION()
	void OnRetryClicked();

	UFUNCTION()
	void OnLobbyClicked();

	UFUNCTION()
	void OnQuitClicked();

protected:
	/************************************************************************/
	/*                          BindWidget 控件                             */
	/************************************************************************/
	// 重新挑战按钮
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* m_retryButton;

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
