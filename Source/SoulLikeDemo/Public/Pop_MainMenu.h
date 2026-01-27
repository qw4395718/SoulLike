// Pop_MainMenu.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Pop_MainMenu.generated.h"

class UUI_IconSlot;
class UButton;

USTRUCT(BlueprintType)
struct FMenuButtonInfo
{
	GENERATED_BODY()

	// 按钮显示文本
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText ButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D ButtonImg;

	// 按钮标签（用于标识）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ButtonTag;

	// 关联的界面控件（可选）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EWidgetType linkWidgetIndex;
};

UCLASS()
class SOULLIKEDEMO_API UPop_MainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	// 添加菜单按钮
	UFUNCTION(BlueprintCallable, Category = "Menu")
		void AddMenuButton(const FMenuButtonInfo& ButtonInfo);

	// 清除所有按钮
	UFUNCTION(BlueprintCallable, Category = "Menu")
		void ClearAllButtons();

	// 移除特定标签的按钮
	UFUNCTION(BlueprintCallable, Category = "Menu")
		bool RemoveButtonByTag(FName ButtonTag);

	// 获取按钮数量
	UFUNCTION(BlueprintPure, Category = "Menu")
		int32 GetButtonCount() const { return m_arrButtonInfos.Num(); }

	// 切换菜单界面
	UFUNCTION(BlueprintCallable, Category = "Menu")
		void SwitchToMenu(int32 MenuIndex);

protected:
	virtual void NativeConstruct() override;

	// 按钮点击处理
	UFUNCTION()
		void OnMenuButtonClicked(FName ButtonTag);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StatusBar")
		TSubclassOf<UUI_MenuItem> m_menuBtnWidgetClass;

private:
	// 存储按钮信息
	UPROPERTY()
		TArray<FMenuButtonInfo> m_arrButtonInfos;

	// 按钮控件映射
	UPROPERTY()
		TMap<FName, UUserWidget*> m_mapButtonWidgets;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		
};