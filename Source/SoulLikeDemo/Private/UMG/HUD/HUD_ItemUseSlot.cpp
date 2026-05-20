#include "HUD_ItemUseSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Manager/DataTableManager.h"
#include "Table/ItemDataTable.h"

UHUD_ItemUseSlot::UHUD_ItemUseSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, m_itemID(NAME_None)
	, m_itemCount(0)
{
}

/************************************************************************/
/* 外部调用                                                                     */
/************************************************************************/

void UHUD_ItemUseSlot::SetItemInfo(const FName& InItemID, int32 InCount)
{
	m_itemID = InItemID;
	m_itemCount = InCount;

	if (InItemID == NAME_None || InCount <= 0)
	{
		ClearSlot();
		return;
	}

	// 从 ItemDataTable 获取道具配置
	UDataTableManager* DTManager = UDataTableManager::Get(this);
	if (!DTManager)
	{
		return;
	}

	UItemDataTable* ItemTable = Cast<UItemDataTable>(
		DTManager->GetDataTable(EDataTableType::DT_ItemConfigInfo));

	if (!ItemTable)
	{
		return;
	}

	FItemDataRow ItemData;
	if (!ItemTable->GetItemData(InItemID, ItemData))
	{
		UE_LOG(LogTemp, Warning, TEXT("UHUD_ItemUseSlot::SetItemInfo - Item not found: %s"), *InItemID.ToString());
		ClearSlot();
		return;
	}

	// 设置图标（异步加载）
	if (m_showImage)
	{
		if (ItemData.ItemIcon.IsValid())
		{
			m_showImage->SetBrushFromTexture(ItemData.ItemIcon.Get());
		}
		else
		{
			TSoftObjectPtr<UTexture2D> IconPtr = ItemData.ItemIcon;
			FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
			Streamable.RequestAsyncLoad(IconPtr.ToSoftObjectPath(),
				[this, IconPtr]()
				{
					if (m_showImage && IconPtr.IsValid())
					{
						m_showImage->SetBrushFromTexture(IconPtr.Get());
					}
				});
		}

		m_showImage->SetVisibility(ESlateVisibility::Visible);
	}

	// 设置数量文本
	if (m_stackNum)
	{
		m_stackNum->SetText(FText::AsNumber(InCount));
		m_stackNum->SetVisibility(ESlateVisibility::Visible);
	}
}

void UHUD_ItemUseSlot::SetSelected(bool bInSelected)
{
	if (m_selectedBg)
	{
		m_selectedBg->SetVisibility(bInSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void UHUD_ItemUseSlot::ClearSlot()
{
	m_itemID = NAME_None;
	m_itemCount = 0;

	if (m_showImage)
	{
		m_showImage->SetVisibility(ESlateVisibility::Hidden);
	}

	if (m_stackNum)
	{
		m_stackNum->SetText(FText::AsNumber(0));
		m_stackNum->SetVisibility(ESlateVisibility::Hidden);
	}

	SetSelected(false);
}

void UHUD_ItemUseSlot::UpdateCount(int32 InNewCount)
{
	m_itemCount = InNewCount;

	if (InNewCount <= 0)
	{
		ClearSlot();
		return;
	}

	if (m_stackNum)
	{
		m_stackNum->SetText(FText::AsNumber(InNewCount));
		m_stackNum->SetVisibility(ESlateVisibility::Visible);
	}
}
