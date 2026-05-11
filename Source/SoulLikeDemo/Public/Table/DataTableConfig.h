// DataTableType.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataTableConfig.generated.h"

// 定义表格类型的枚举，按功能组件分类
UENUM(BlueprintType)
enum class EDataTableType : uint8
{
	DT_None					UMETA(DisplayName = "None"),
	DT_ComboInfo			UMETA(DisplayName = "ComboInfo"),
	DT_WeaponDataInfo			UMETA(DisplayName = "WeaponDataInfo"),
	DT_ClassConfigInfo			UMETA(DisplayName = "ClassConfigInfo"),
	DT_WaveConfigInfo			UMETA(DisplayName = "WaveConfigInfo"),
	DT_EnemyConfigInfo		UMETA(DisplayName = "EnemyConfigInfo"),
	DT_ItemConfigInfo		UMETA(DisplayName = "ItemConfigInfo"),
	DT_Max					UMETA(Hidden)
};

// 单个DataTable的配置信息
USTRUCT(BlueprintType)
struct FDataTableConfig : public FTableRowBase
{
	GENERATED_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable Config")
		FString TableName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable Config")
		EDataTableType TableType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable Config")
		TSoftObjectPtr<UDataTable> DataTableAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable Config")
		bool bAutoLoad = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable Config")
		int32 Priority = 0;
};