#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Texture2D.h"
#include "ItemData.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
    Common     UMETA(DisplayName = "Common"),      // 普通
    Uncommon   UMETA(DisplayName = "Uncommon"),    // 稀有
    Rare       UMETA(DisplayName = "Rare"),        // 罕见
    Epic       UMETA(DisplayName = "Epic"),        // 史诗
    Legendary  UMETA(DisplayName = "Legendary")    // 传说
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Consumable UMETA(DisplayName = "Consumable"),  // 消耗品
    Equipment  UMETA(DisplayName = "Equipment"),   // 装备
    Material   UMETA(DisplayName = "Material"),    // 材料
    Quest      UMETA(DisplayName = "Quest")        // 任务物品
};

/**
 * 物品基础数据结构
 * 可以在蓝图中创建和编辑
 */
UCLASS(Blueprintable, BlueprintType)
class SOULLIKEDEMO_API UItemData : public UObject
{
    GENERATED_BODY()

public:
    UItemData();

    // 物品ID - 唯一标识
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FName ItemID;

    // 物品显示名称
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText ItemName;

    // 物品描述
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    FText ItemDescription;

    // 物品图标
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    UTexture2D* ItemIcon;

    // 物品数量
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 ItemCount;

    // 最大堆叠数量
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 MaxStackCount;

    // 物品稀有度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemRarity Rarity;

    // 物品类型
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    EItemType ItemType;

    // 物品重量（用于负重系统）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    float ItemWeight;

    // 物品价值（售价）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 ItemValue;

    // 是否可丢弃
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bCanDrop;

    // 是否可使用
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    bool bCanUse;

    // 使用效果（蓝图可覆盖）
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
    void UseItem(AActor* TargetActor);
    virtual void UseItem_Implementation(AActor* TargetActor) {}

    // 获取背景颜色（根据稀有度）
    UFUNCTION(BlueprintCallable, Category = "Item")
    FLinearColor GetRarityColor() const;

    // 是否可以堆叠
    UFUNCTION(BlueprintCallable, Category = "Item")
    bool CanStackWith(const UItemData* OtherItem) const;

    // 合并堆叠
    UFUNCTION(BlueprintCallable, Category = "Item")
    int32 MergeStack(UItemData* OtherItem);
};