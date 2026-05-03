// Public/Table/ClassConfigTable.h

#pragma once

#include "CoreMinimal.h"
#include "BaseDataTable.h"
#include "SoulLikeGameGlobal.h"
#include "ClassConfigInfoTable.generated.h"

UCLASS(BlueprintType)
class SOULLIKEDEMO_API UClassConfigInfoTable : public UBaseDataTable
{
	GENERATED_BODY()

public:
	virtual bool InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset) override;
	virtual bool PostExecuteData() override;
	virtual UBaseDataTable* GetDataTable() const override;

	/** 根据职业ID获取职业配置 */
	UFUNCTION(BlueprintPure, Category = "ClassConfig")
		bool GetClassConfig(int32 ClassID, FClassConfigInfo& OutConfig) const;

	/** 获取所有职业ID列表 */
	UFUNCTION(BlueprintPure, Category = "ClassConfig")
		TArray<int32> GetAllClassIDs() const;

protected:
	/** 职业配置缓存：ClassID -> ClassConfig */
	UPROPERTY()
		TMap<int32, FClassConfigInfo> ClassConfigMap;
};