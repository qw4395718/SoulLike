// Public/Table/ItemDataTable.h
#pragma once

#include "CoreMinimal.h"
#include "BaseDataTable.h"
#include "ItemDataStruct.h"
#include "ItemDataTable.generated.h"

UCLASS(BlueprintType)
class SOULLIKEDEMO_API UItemDataTable : public UBaseDataTable
{
	GENERATED_BODY()

public:
	/************************************************************************/
	/*                              通用逻辑                                */
	/************************************************************************/
	virtual bool InitializeFromAsset(TSoftObjectPtr<UDataTable> InTableAsset) override;
	virtual bool PostExecuteData() override;
	virtual UBaseDataTable* GetDataTable() const override;

	/************************************************************************/
	/*                              外部调用                                */
	/************************************************************************/
	// 根据物品ID获取物品配置
	UFUNCTION(BlueprintPure, Category = "ItemData")
		bool GetItemData(FName InItemID, FItemDataRow& OutItemData) const;

	// 获取所有物品ID列表
	UFUNCTION(BlueprintPure, Category = "ItemData")
		TArray<FName> GetAllItemIDs() const;

protected:
	/************************************************************************/
	/*                               内部访问                               */
	/************************************************************************/
	// 物品数据缓存：物品ID -> 物品配置
	UPROPERTY()
		TMap<FName, FItemDataRow> ItemDataMap;
};