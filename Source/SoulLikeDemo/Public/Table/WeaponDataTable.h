// Public/Table/WeaponDataTable.h

#pragma once

#include "CoreMinimal.h"
#include "BaseDataTable.h"
#include "SoulLikeGameGlobal.h"
#include "WeaponDataTable.generated.h"

UCLASS(BlueprintType)
class SOULLIKEDEMO_API UWeaponDataTable : public UBaseDataTable
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
	/*                              外部调用方法                            */
	/************************************************************************/
	/** 根据武器ID获取武器配置 */
	UFUNCTION(BlueprintPure, Category = "WeaponData")
		bool GetWeaponData(int32 InWeaponID, FWeaponDataInfo& OutWeaponData) const;

	/** 获取所有武器ID列表 */
	UFUNCTION(BlueprintPure, Category = "WeaponData")
		TArray<int32> GetAllWeaponIDs() const;

protected:
	/** 武器数据缓存：武器ID -> 武器配置 */
	UPROPERTY()
		TMap<int32, FWeaponDataInfo> WeaponDataMap;
};