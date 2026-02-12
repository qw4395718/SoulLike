#include "ItemData.h"
#include "Engine/Engine.h"

UItemData::UItemData()
{
    ItemCount = 1;
    MaxStackCount = 99;
    ItemWeight = 0.0f;
    ItemValue = 0;
    bCanDrop = true;
    bCanUse = false;
    Rarity = EItemRarity::Common;
    ItemType = EItemType::Material;
    ItemIcon = nullptr;
}

FLinearColor UItemData::GetRarityColor() const
{
    switch (Rarity)
    {
    case EItemRarity::Common:
        return FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);      // 灰色
    case EItemRarity::Uncommon:
        return FLinearColor(0.12f, 1.0f, 0.0f, 1.0f);        // 绿色
    case EItemRarity::Rare:
        return FLinearColor(0.0f, 0.44f, 0.87f, 1.0f);       // 蓝色
    case EItemRarity::Epic:
        return FLinearColor(0.64f, 0.21f, 0.93f, 1.0f);      // 紫色
    case EItemRarity::Legendary:
        return FLinearColor(1.0f, 0.5f, 0.0f, 1.0f);         // 橙色
    default:
        return FLinearColor::White;
    }
}

bool UItemData::CanStackWith(const UItemData* OtherItem) const
{
    if (!OtherItem) return false;
    
    // 相同ID且未达到最大堆叠
    return ItemID == OtherItem->ItemID && 
           ItemCount < MaxStackCount && 
           OtherItem->ItemCount < OtherItem->MaxStackCount;
}

int32 UItemData::MergeStack(UItemData* OtherItem)
{
    if (!CanStackWith(OtherItem)) return 0;
    
    int32 TotalCount = ItemCount + OtherItem->ItemCount;
    int32 MaxStack = MaxStackCount;
    
    if (TotalCount <= MaxStack)
    {
        ItemCount = TotalCount;
        OtherItem->ItemCount = 0;
        return TotalCount;
    }
    else
    {
        ItemCount = MaxStack;
        OtherItem->ItemCount = TotalCount - MaxStack;
        return MaxStack;
    }
}