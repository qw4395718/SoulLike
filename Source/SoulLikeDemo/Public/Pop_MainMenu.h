// Pop_MainMenu.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "SoulLikeGameGlobal.h"
#include "Pop_MainMenu.generated.h"

class UUI_IconSlot;
class UButton;
class UUI_MenuItem;
class UScrollBox;
class UGridPanel;

USTRUCT(BlueprintType)
struct FMenuButtonInfo
{
	GENERATED_BODY()

	// 按钮显示文本
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString ButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* ButtonImg;

	// 按钮标签（用于标识）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FName ButtonTag;

	// 关联的界面控件（可选）
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		EWidgetType linkWidgetIndex;

	bool operator==(const FMenuButtonInfo& Other) const
	{
		return ButtonTag == Other.ButtonTag;  // 使用唯一标识进行比较
	}

};

UCLASS()
class SOULLIKEDEMO_API UPop_MainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	// 设置按钮信息
	void SetButtonInfos(const TArray<FMenuButtonInfo>& infos);

	// 清理按钮信息
	void ClearAllButtonInfos();

	// 新增按钮信息
	void AddButtonInfo(const FMenuButtonInfo& info);

	// 移除按钮信息
	void RemoveButtonInfo(const FName buttonFlag);

	// 更新按钮信息
	void UpdateButtonInfo(const FMenuButtonInfo& info);

	// 同步信息到按钮上
	void RefreshMenuToUI();

	// 设置宽高系数
	void SetupGridLayout();

protected:

	// 创建新的按钮
	void CreateNewMenuItem(const FMenuButtonInfo& info);

	// 移除指定按钮
	void RemoveMenuItem(const FName buttonFlag);

	// 更新按钮
	void UpdateMenuItem(const FMenuButtonInfo& info);

	// 清理所有的按钮
	void ClearAllMenuItems();

protected:
	virtual void NativeConstruct() override;

	// 按钮点击处理
	UFUNCTION()
		void OnMenuButtonClicked(FName ButtonTag);

public:
	// 暴露给蓝图的属性
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Settings")
		int32 ColumnsPerRow = 3;  // 每行列数

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Settings")
		float ButtonWidth = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Settings")
		float ButtonHeight = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu Settings")
		FMargin ButtonPadding = FMargin(10.0f);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StatusBar")
		TSubclassOf<UUI_MenuItem> m_menuItemWidgetClass;

	UPROPERTY(meta = (BindWidget))
		UScrollBox* ScrollBox;

	UPROPERTY()
		UWrapBox* MenuWrapBox;

private:
	// 存储按钮信息
	UPROPERTY()
		TArray<FMenuButtonInfo> m_arrButtonInfos;

	// 按钮控件映射
	UPROPERTY()
		TMap<FName, UUI_MenuItem*> m_mapButtonWidgets;
};