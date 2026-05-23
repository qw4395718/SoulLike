// BaseDataTable.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BaseDataTable.h"
#include <GameplayTagContainer.h>
#include <SoulLikeGameGlobal.h>
#include "ComboInfoTable.generated.h"

UCLASS(BlueprintType)
class SOULLIKEDEMO_API UComboInfoTable : public UBaseDataTable
{
	GENERATED_BODY()

public:
	/************************************************************************/
	/*									通用逻辑                            */
	/************************************************************************/
	/** 从DataTable资产路径初始化 子类自己完成各自的数据初始化*/
	virtual bool InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset) override;

	/** 基础数据加工,必须在已完成基础上数据加载后进行 */
	virtual bool PostExecuteData()override;

	// 获取指定类型的数据表
	virtual UBaseDataTable* GetDataTable() const override;

	/************************************************************************/
	/*                              外部调用方法                            */
	/************************************************************************/
	UFUNCTION()
		bool FindNextComboInfo(const FGameplayTagContainer& Tags, EComboInputActionType InputActionType, FComboInfo& ComboInfo);

protected:

	// 连击系统相关数据(复合Key: 窗口Tag + 输入类型)
	UPROPERTY()
	TMap<FComboLookupKey, FComboInfo> ComboInfoMap;

};
