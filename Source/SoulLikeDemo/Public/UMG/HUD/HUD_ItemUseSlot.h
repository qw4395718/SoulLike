#pragma once

#include "CoreMinimal.h"
#include "UMG/SharedCompoent/UI_DefaultSlot.h"
#include "HUD_ItemUseSlot.generated.h"

class UImage;
class UTexture2D;

/**
 * 道具快捷栏槽位Widget
 *
 * 显示内容：
 *   - 道具图标（m_showImage，继承自 UUI_BaseSlot）
 *   - 剩余数量（m_stackNum，继承自 UUI_BaseSlot）
 *   - 选中状态高亮背景（m_selectedBg）
 *
 * 配合 HUD_ItemUseUI 使用，分别作为"上一个"、"当前选中"、"下一个"槽位。
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class SOULLIKEDEMO_API UHUD_ItemUseSlot : public UUI_DefaultSlot
{
	GENERATED_BODY()

public:
	UHUD_ItemUseSlot(const FObjectInitializer& ObjectInitializer);

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	// 设置道具数据（ID + 数量），从 ItemDataTable 加载图标
	void SetItemInfo(const FName& InItemID, int32 InCount);

	// 设置选中状态（高亮边框/背景切换）
	void SetSelected(bool bInSelected);

	// 清空槽位（隐藏图标、数量归零）
	void ClearSlot();

	// 获取当前道具ID
	FName GetItemID() const { return m_itemID; }

	// 获取当前数量
	int32 GetItemCount() const { return m_itemCount; }

	// 更新数量显示（不改变道具ID）
	void UpdateCount(int32 InNewCount);

protected:
	/************************************************************************/
	/* 内部变量 - BindWidget控件绑定                                                */
	/************************************************************************/

	// 选中状态背景（显示/隐藏控制高亮）
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "ItemUseSlot")
	UImage* m_selectedBg;

protected:
	/************************************************************************/
	/* 内部变量 - 数据                                                             */
	/************************************************************************/

	// 当前道具ID（NAME_None 表示空）
	FName m_itemID;

	// 当前剩余数量
	int32 m_itemCount;
};
