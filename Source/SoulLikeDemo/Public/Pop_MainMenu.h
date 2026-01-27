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

USTRUCT(BlueprintType)
struct FMenuButtonInfo
{
	GENERATED_BODY()

	// 按钮显示文本
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FText ButtonText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UTexture2D* ButtonImg;

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

protected:

	// 创建新的按钮
	void CreateNewMenuItem(const FMenuButtonInfo& info);

	// 移除指定按钮
	void RemoveMenuItem(const FName buttonFlag);

	// 清理所有的按钮
	void ClearAllMenuItems();

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

	//UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
		
		
};