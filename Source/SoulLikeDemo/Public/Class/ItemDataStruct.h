#pragma once
#include "Engine/DataTable.h"
#include "ItemDataObject.h"
#include <GameplayTagContainer.h>
#include "ItemDataStruct.generated.h"

class UTexture2D;

// ===== 道具使用行为类型 =====
UENUM(BlueprintType)
enum class EItemUseBehavior : uint8
{
	Default			UMETA(DisplayName = "默认（应用GE效果）"),
	PlaceSummonSign	UMETA(DisplayName = "放置召唤标记")
};

USTRUCT(BlueprintType)
struct FItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

	// 物品基础信息
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	FText ItemDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
	TSoftObjectPtr<UTexture2D> ItemIcon;

	// 物品数量与堆叠
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stack")
	int32 ItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stack")
	int32 MaxStackCount;

	// 分类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	EItemRarity Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
	EItemType ItemType;

	// 经济系统
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	float ItemWeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int32 ItemValue;

	// 行为标记
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	bool bCanDrop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	bool bCanUse;

	// ===== 行为类型 =====
	/** 道具被使用时的行为类型 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	EItemUseBehavior UseBehavior = EItemUseBehavior::Default;

	// ===== GAS能力相关 =====
	// 使用此道具时触发的GameplayTag（对应GAS的ActivationTag）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	FGameplayTag UseAbilityTag;

	// 使用此道具后应用给自己的GameplayEffect的类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSoftClassPtr<class UGameplayEffect> SelfEffectClass;

	// 使用此道具后投掷到目标的GameplayEffect的类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TSoftClassPtr<class UGameplayEffect> TargetEffectClass;
};