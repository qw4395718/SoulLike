// Private/Table/PartBreakDataTable.cpp
#include "PartBreakDataTable.h"
#include "Engine/DataTable.h"

bool UPartBreakDataTable::InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset)
{
	if (!TableAsset.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UPartBreakDataTable::InitializeFromAsset - TableAsset is invalid"));
		return false;
	}

	UDataTable* DataTable = TableAsset.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UPartBreakDataTable::InitializeFromAsset - Failed to load DataTable"));
		return false;
	}

	PartBreakConfigMap.Empty();

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FPartBreakEnemyRow* ConfigRow = DataTable->FindRow<FPartBreakEnemyRow>(RowName, TEXT("InitializeFromAsset"));
		if (ConfigRow && ConfigRow->EnemyID > 0)
		{
			if (PartBreakConfigMap.Contains(ConfigRow->EnemyID))
			{
				UE_LOG(LogTemp, Warning, TEXT("UPartBreakDataTable::InitializeFromAsset - Duplicate EnemyID: %d"), ConfigRow->EnemyID);
				continue;
			}
			PartBreakConfigMap.Add(ConfigRow->EnemyID, ConfigRow->Parts);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UPartBreakDataTable::InitializeFromAsset - Loaded %d enemy part break configs"), PartBreakConfigMap.Num());
	return true;
}

bool UPartBreakDataTable::PostExecuteData()
{
	LoadState = ETableLoadState::Loaded;
	return true;
}

UBaseDataTable* UPartBreakDataTable::GetDataTable() const
{
	return nullptr;
}

bool UPartBreakDataTable::GetPartBreakConfigs(int32 EnemyID, TArray<FPartBreakConfig>& OutConfigs) const
{
	if (const TArray<FPartBreakConfig>* Found = PartBreakConfigMap.Find(EnemyID))
	{
		OutConfigs = *Found;
		return true;
	}
	return false;
}

TArray<int32> UPartBreakDataTable::GetAllEnemyIDs() const
{
	TArray<int32> IDs;
	PartBreakConfigMap.GetKeys(IDs);
	return IDs;
}
