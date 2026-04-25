// BaseDataTable.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "BaseDataTable.generated.h"

// 表格加载状态
UENUM(BlueprintType)
enum class ETableLoadState : uint8
{
	NotLoaded    UMETA(DisplayName = "未加载"),
	Loading      UMETA(DisplayName = "加载中"),
	Loaded       UMETA(DisplayName = "已加载"),
	Failed       UMETA(DisplayName = "加载失败")
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UBaseDataTable : public UObject
{
	GENERATED_BODY()

public:
	/************************************************************************/
	/*                              外部调用方法                            */
	/************************************************************************/
	/** 从DataTable资产路径初始化 子类自己完成各自的数据初始化*/
	UFUNCTION(BlueprintCallable, Category = "DataTable")
		virtual bool InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset) { return true; }

	/** 基础数据加工,必须在已完成基础上数据加载后进行 */
	UFUNCTION(BlueprintCallable, Category = "DataTable")
		virtual bool PostExecuteData(){return true;}

	/** 获取加载状态 */
	UFUNCTION(BlueprintPure, Category = "DataTable|State")
		ETableLoadState GetLoadState() const { return LoadState; }

	/** 是否已加载 */
	UFUNCTION(BlueprintPure, Category = "DataTable|State")
		bool IsLoaded() const { return LoadState == ETableLoadState::Loaded; }

	// 获取指定类型的数据表
	UFUNCTION(BlueprintCallable, Category = "DataTable")
		virtual UBaseDataTable* GetDataTable() const { return nullptr; }

protected:

	// 加载状态
	UPROPERTY(BlueprintReadOnly, Category = "DataTable")
		ETableLoadState LoadState = ETableLoadState::NotLoaded;
};
