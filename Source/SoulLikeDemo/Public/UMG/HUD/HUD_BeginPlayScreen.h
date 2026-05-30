// Public/UI/HUD/HUD_BeginPlayScreen.h

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD_BeginPlayScreen.generated.h"

class UButton;

/**
 * 开始界面Widget
 * 包含：新游戏、加载存档、退出游戏三个功能按钮
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class SOULLIKEDEMO_API UHUD_BeginPlayScreen : public UUserWidget
{
    GENERATED_BODY()

public:
    UHUD_BeginPlayScreen(const FObjectInitializer& ObjectInitializer);

    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** 初始化按钮状态（根据是否有存档） */
    UFUNCTION(BlueprintCallable, Category = "BeginPlayScreen")
    void InitializeScreen(bool bHasSaveData);

    // 跳转到大厅界面
    void ShowLobbyScreen();
protected:
    /************************************************************************/
    /*                               按钮事件                                */
    /************************************************************************/

    /** 新游戏按钮点击 */
    UFUNCTION()
    virtual void OnNewGameClicked();

    /** 加载存档按钮点击 */
    UFUNCTION()
    virtual void OnLoadGameClicked();

    /** 退出游戏按钮点击 */
    UFUNCTION()
    virtual void OnQuitGameClicked();

protected:
    /************************************************************************/
    /*                               内部访问                                */
    /************************************************************************/

	UPROPERTY()
		class UUIManagerSubsystem* UIManager;

    /** 新游戏按钮 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "BeginPlayScreen")
    UButton* NewGameButton;

    /** 加载存档按钮 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "BeginPlayScreen")
    UButton* LoadGameButton;

    /** 退出游戏按钮 */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "BeginPlayScreen")
    UButton* QuitGameButton;

private:

};