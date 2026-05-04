// Private/Table/EnemyConfigInfoTable.cpp

#include "EnemyConfigInfoTable.h"
#include "Engine/DataTable.h"

bool UEnemyConfigInfoTable::InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset)
{
	if (!TableAsset.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UEnemyConfigInfoTable::InitializeFromAsset - TableAsset is invalid"));
		return false;
	}

	UDataTable* DataTable = TableAsset.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEnemyConfigInfoTable::InitializeFromAsset - Failed to load DataTable"));
		return false;
	}

	// 清空缓存
	EnemyConfigMap.Empty();
	EnemyTypeIndex.Empty();

	// 遍历所有行
	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FEnemyConfigInfo* ConfigRow = DataTable->FindRow<FEnemyConfigInfo>(RowName, TEXT("InitializeFromAsset"));
		if (ConfigRow)
		{
			// 检查EnemyID是否有效
			if (ConfigRow->EnemyID <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("UEnemyConfigInfoTable::InitializeFromAsset - Invalid EnemyID in row %s"), *RowName.ToString());
				continue;
			}

			// 检查是否重复
			if (EnemyConfigMap.Contains(ConfigRow->EnemyID))
			{
				UE_LOG(LogTemp, Warning, TEXT("UEnemyConfigInfoTable::InitializeFromAsset - Duplicate EnemyID: %d in row %s"),
					ConfigRow->EnemyID, *RowName.ToString());
				continue;
			}

			// 添加到缓存
			EnemyConfigMap.Add(ConfigRow->EnemyID, *ConfigRow);

			// 构建类型索引
			TArray<int32>& EnemyIDs = EnemyTypeIndex.FindOrAdd(ConfigRow->EnemyType);
			EnemyIDs.Add(ConfigRow->EnemyID);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UEnemyConfigInfoTable::InitializeFromAsset - Successfully loaded %d enemies"), EnemyConfigMap.Num());
	return true;
}

bool UEnemyConfigInfoTable::PostExecuteData()
{
	LoadState = ETableLoadState::Loaded;
	return true;
}

UBaseDataTable* UEnemyConfigInfoTable::GetDataTable() const
{
	return nullptr;
}

bool UEnemyConfigInfoTable::GetEnemyConfig(int32 EnemyID, FEnemyConfigInfo& OutConfig) const
{
	if (const FEnemyConfigInfo* Found = EnemyConfigMap.Find(EnemyID))
	{
		OutConfig = *Found;
		return true;
	}
	return false;
}

TArray<FEnemyConfigInfo> UEnemyConfigInfoTable::GetEnemiesByType(EEnemyType EnemyType) const
{
	TArray<FEnemyConfigInfo> Result;

	const TArray<int32>* EnemyIDs = EnemyTypeIndex.Find(EnemyType);
	if (EnemyIDs)
	{
		for (int32 EnemyID : *EnemyIDs)
		{
			if (const FEnemyConfigInfo* Config = EnemyConfigMap.Find(EnemyID))
			{
				Result.Add(*Config);
			}
		}
	}

	return Result;
}

TArray<int32> UEnemyConfigInfoTable::GetAllEnemyIDs() const
{
	TArray<int32> IDs;
	EnemyConfigMap.GetKeys(IDs);
	return IDs;
}