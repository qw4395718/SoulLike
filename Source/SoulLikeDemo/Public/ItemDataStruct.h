#pragma once
#include "Engine/DataTable.h"
#include "ItemDataStruct.generated.h"

class EItemRarity;
class EItemType;
class UTexture2D;

USTRUCT(BlueprintType)
struct FItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

	FName ItemID;

	// 物品显示名称
	FText ItemName;

	// 物品描述
	FText ItemDescription;

	// 物品图标
	UTexture2D* ItemIcon;

	// 物品数量
	int32 ItemCount;

	// 最大堆叠数量
	int32 MaxStackCount;

	// 物品稀有度
	EItemRarity Rarity;

	// 物品类型
	EItemType ItemType;

	// 物品重量（用于负重系统）
	float ItemWeight;

	// 物品价值（售价）
	int32 ItemValue;

	// 是否可丢弃
	bool bCanDrop;

	// 是否可使用
	bool bCanUse;

   
};
