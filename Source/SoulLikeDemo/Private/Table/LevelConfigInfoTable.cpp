// LevelConfigInfoTable.cpp

#include "LevelConfigInfoTable.h"
#include "Engine/DataTable.h"

bool ULevelConfigInfoTable::InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset)
{
	if (!TableAsset.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ULevelConfigInfoTable::InitializeFromAsset - TableAsset is invalid"));
		return false;
	}

	UDataTable* DataTable = TableAsset.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ULevelConfigInfoTable::InitializeFromAsset - Failed to load DataTable"));
		return false;
	}

	LevelConfigMap.Empty();

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FLevelConfigInfo* LevelRow = DataTable->FindRow<FLevelConfigInfo>(RowName, TEXT("InitializeFromAsset"));
		if (LevelRow)
		{
			if (LevelRow->LevelID <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("ULevelConfigInfoTable::InitializeFromAsset - Invalid LevelID in row %s"), *RowName.ToString());
				continue;
			}

			LevelConfigMap.Add(LevelRow->LevelID, *LevelRow);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ULevelConfigInfoTable::InitializeFromAsset - Loaded %d level configs"), LevelConfigMap.Num());
	return true;
}

bool ULevelConfigInfoTable::PostExecuteData()
{
	LoadState = ETableLoadState::Loaded;
	return true;
}

UBaseDataTable* ULevelConfigInfoTable::GetDataTable() const
{
	return nullptr;
}

bool ULevelConfigInfoTable::GetLevelConfig(int32 LevelID, FLevelConfigInfo& OutConfig) const
{
	if (const FLevelConfigInfo* Found = LevelConfigMap.Find(LevelID))
	{
		OutConfig = *Found;
		return true;
	}
	return false;
}

TArray<int32> ULevelConfigInfoTable::GetAllLevelIDs() const
{
	TArray<int32> OutIDs;
	LevelConfigMap.GetKeys(OutIDs);
	OutIDs.Sort();
	return OutIDs;
}

FName ULevelConfigInfoTable::GetLevelName(int32 LevelID) const
{
	if (const FLevelConfigInfo* Found = LevelConfigMap.Find(LevelID))
	{
		return Found->LevelName;
	}
	return NAME_None;
}

FName ULevelConfigInfoTable::GetMapName(int32 LevelID) const
{
	if (const FLevelConfigInfo* Found = LevelConfigMap.Find(LevelID))
	{
		return Found->MapName;
	}
	return NAME_None;
}
