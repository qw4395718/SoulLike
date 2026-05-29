#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_ListItemBase.generated.h"

class UButton;
class UTextBlock;
class UImage;

DECLARE_DELEGATE_OneParam(FOnListItemClickedSignature, int32);

/**
 * 通用列表项基类
 *
 * 适用于"按钮 + 文本 + 可选图标"的动态列表项场景。
 * 蓝图中应外套 SizeBox 以控制固定尺寸。
 *
 * 直接使用示例：LobbyScreen 的关卡选择列表
 * 已有派生类（非强制迁移）：UUI_MenuItem、UUI_InterActButton
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class SOULLIKEDEMO_API UUI_ListItemBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UUI_ListItemBase(const FObjectInitializer& ObjectInitializer);

	/************************************************************************/
	/* 继承实现                                                                     */
	/************************************************************************/
	virtual void NativeConstruct() override;

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 设置按钮显示文本
	void SetDisplayText(const FText& InText);

	// 设置图标（可选，如蓝图无对应 BindWidget 则静默忽略）
	void SetIcon(UTexture2D* InTexture);

	// 设置关联整型数据（如关卡ID、道具ID等）
	void SetItemData(int32 InData);
	int32 GetItemData() const;

	// 启用/禁用交互（禁用时同时置灰文本）
	void SetItemEnabled(bool bInEnabled);

	// 设置选中状态
	UFUNCTION(BlueprintNativeEvent)
		void SetSelected(bool bInSelected);

	// 点击回调委托（父级绑定）
	FOnListItemClickedSignature OnItemClicked;

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	UFUNCTION()
	virtual void OnButtonClicked();

protected:
	/************************************************************************/
	/* 内部变量 - BindWidget                                                      */
	/************************************************************************/
	// 主按钮（必需绑定）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UButton* m_mainButton;

	// 标题文本（必需绑定）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* m_titleText;

	// 图标（可选绑定）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, OptionalWidget = true))
	UImage* m_iconImage;

protected:
	/************************************************************************/
	/* 内部变量 - 数据                                                             */
	/************************************************************************/
	int32 m_itemData;

	bool m_bSelected;
};
