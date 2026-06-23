// Public/Table/PartBreakDataTable.h
#pragma once

#include "CoreMinimal.h"
#include "BaseDataTable.h"
#include "SoulLikeGameGlobal.h"
#include "PartBreakDataTable.generated.h"

UCLASS(BlueprintType)
class SOULLIKEDEMO_API UPartBreakDataTable : public UBaseDataTable
{
	GENERATED_BODY()

public:
	virtual bool InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset) override;
	virtual bool PostExecuteData() override;
	virtual UBaseDataTable* GetDataTable() const override;

	UFUNCTION(BlueprintPure, Category = "PartBreak")
	bool GetPartBreakConfigs(int32 EnemyID, TArray<FPartBreakConfig>& OutConfigs) const;

	UFUNCTION(BlueprintPure, Category = "PartBreak")
	TArray<int32> GetAllEnemyIDs() const;

protected:

	TMap<int32, TArray<FPartBreakConfig>> PartBreakConfigMap;
};
