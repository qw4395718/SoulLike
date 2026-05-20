// Private/Table/WaveConfigInfoTable.cpp

#include "WaveConfigInfoTable.h"
#include "Engine/DataTable.h"

bool UWaveConfigInfoTable::InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset)
{
	if (!TableAsset.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UWaveConfigInfoTable::InitializeFromAsset - TableAsset is invalid"));
		return false;
	}

	UDataTable* DataTable = TableAsset.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWaveConfigInfoTable::InitializeFromAsset - Failed to load DataTable"));
		return false;
	}

	WaveConfigMap.Empty();
	LevelWaveIndex.Empty();

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FWaveConfigInfo* WaveRow = DataTable->FindRow<FWaveConfigInfo>(RowName, TEXT("InitializeFromAsset"));
		if (WaveRow)
		{
			if (WaveRow->WaveID <= 0 || WaveRow->LevelID <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("UWaveConfigInfoTable::InitializeFromAsset - Invalid ID in row %s"), *RowName.ToString());
				continue;
			}

			WaveConfigMap.Add(WaveRow->WaveID, *WaveRow);

			// 构建关卡索引
			TArray<int32>& WaveIDs = LevelWaveIndex.FindOrAdd(WaveRow->LevelID);
			WaveIDs.AddUnique(WaveRow->WaveID);
		}
	}

	// 对每个关卡的波次ID进行排序
	for (auto& Pair : LevelWaveIndex)
	{
		Pair.Value.Sort();
	}

	UE_LOG(LogTemp, Log, TEXT("UWaveConfigInfoTable::InitializeFromAsset - Loaded %d waves for %d levels"),
		WaveConfigMap.Num(), LevelWaveIndex.Num());
	return true;
}

bool UWaveConfigInfoTable::PostExecuteData()
{
	LoadState = ETableLoadState::Loaded;
	return true;
}

UBaseDataTable* UWaveConfigInfoTable::GetDataTable() const
{
	return nullptr;
}

bool UWaveConfigInfoTable::GetWavesForLevel(int32 LevelID, TArray<FWaveConfigInfo>& OutWaves) const
{
	OutWaves.Empty();

	const TArray<int32>* WaveIDs = LevelWaveIndex.Find(LevelID);
	if (!WaveIDs) return false;

	for (int32 WaveID : *WaveIDs)
	{
		if (const FWaveConfigInfo* Wave = WaveConfigMap.Find(WaveID))
		{
			OutWaves.Add(*Wave);
		}
	}

	return OutWaves.Num() > 0;
}

bool UWaveConfigInfoTable::GetWaveConfig(int32 WaveID, FWaveConfigInfo& OutConfig) const
{
	if (const FWaveConfigInfo* Found = WaveConfigMap.Find(WaveID))
	{
		OutConfig = *Found;
		return true;
	}
	return false;
}

int32 UWaveConfigInfoTable::GetWaveCountForLevel(int32 LevelID) const
{
	const TArray<int32>* WaveIDs = LevelWaveIndex.Find(LevelID);
	return WaveIDs ? WaveIDs->Num() : 0;
}

TArray<int32> UWaveConfigInfoTable::GetAllLevelIDs() const
{
	TArray<int32> OutIDs;
	LevelWaveIndex.GetKeys(OutIDs);
	OutIDs.Sort();
	return OutIDs;
}
