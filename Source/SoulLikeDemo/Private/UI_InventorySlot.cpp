#include "UI_InventorySlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UUI_InventorySlot::NativeConstruct()
{
    Super::NativeConstruct();

    bIsSelected = false;
    SlotIndex = -1;

    // 初始化显示
    UpdateDisplay();
}

void UUI_InventorySlot::SetItemData(UItemData* NewItemData)
{
    ItemData = NewItemData;
    UpdateDisplay();
}

void UUI_InventorySlot::SetSelected(bool bSelected)
{
    bIsSelected = bSelected;
    if (SelectionBorder)
    {
        SelectionBorder->SetVisibility(bIsSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UUI_InventorySlot::UpdateDisplay()
{
    if (!ItemData)
    {
        // 空格子
        if (ItemIcon)
        {
            ItemIcon->SetVisibility(ESlateVisibility::Hidden);
        }
        if (ItemCountText)
        {
            ItemCountText->SetVisibility(ESlateVisibility::Hidden);
        }
        if (SlotBorder)
        {
            SlotBorder->SetBrushColor(FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));
        }
    }
    else
    {
        // 有物品的格子
        if (ItemIcon)
        {
            if (ItemData->ItemIcon)
            {
                ItemIcon->SetBrushFromTexture(ItemData->ItemIcon);
                ItemIcon->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                ItemIcon->SetVisibility(ESlateVisibility::Hidden);
            }
        }

        if (ItemCountText)
        {
            if (ItemData->ItemCount > 1 && ItemData->MaxStackCount > 1)
            {
                ItemCountText->SetText(FText::AsNumber(ItemData->ItemCount));
                ItemCountText->SetVisibility(ESlateVisibility::Visible);
            }
            else
            {
                ItemCountText->SetVisibility(ESlateVisibility::Hidden);
            }
        }

        if (SlotBorder)
        {
            // 根据物品稀有度设置边框颜色
            SlotBorder->SetBrushColor(ItemData->GetRarityColor() * 0.3f);
        }
    }
}

void UUI_InventorySlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

    if (HoverBorder)
    {
        HoverBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }

    OnSlotHovered.Broadcast(this);
}

void UUI_InventorySlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);

    if (HoverBorder)
    {
        HoverBorder->SetVisibility(ESlateVisibility::Hidden);
    }

    OnSlotUnhovered.Broadcast(this);
}

FReply UUI_InventorySlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        OnSlotClicked.Broadcast(this);
        return FReply::Handled();
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}