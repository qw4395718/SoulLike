// Private/Table/ClassConfigInfoTable.cpp

#include "ClassConfigInfoTable.h"
#include "Engine/DataTable.h"

bool UClassConfigInfoTable::InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset)
{
	if (!TableAsset.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UClassConfigInfoTable::InitializeFromAsset - TableAsset is invalid"));
		return false;
	}

	UDataTable* DataTable = TableAsset.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UClassConfigInfoTable::InitializeFromAsset - Failed to load DataTable"));
		return false;
	}

	ClassConfigMap.Empty();

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FClassConfigInfo* ConfigRow = DataTable->FindRow<FClassConfigInfo>(RowName, TEXT("InitializeFromAsset"));
		if (ConfigRow)
		{
			if (ConfigRow->ClassID <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("UClassConfigInfoTable::InitializeFromAsset - Invalid ClassID in row %s"), *RowName.ToString());
				continue;
			}

			if (ClassConfigMap.Contains(ConfigRow->ClassID))
			{
				UE_LOG(LogTemp, Warning, TEXT("UClassConfigInfoTable::InitializeFromAsset - Duplicate ClassID: %d"), ConfigRow->ClassID);
				continue;
			}

			ClassConfigMap.Add(ConfigRow->ClassID, *ConfigRow);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UClassConfigInfoTable::InitializeFromAsset - Successfully loaded %d class configs"), ClassConfigMap.Num());
	return true;
}

bool UClassConfigInfoTable::PostExecuteData()
{
	LoadState = ETableLoadState::Loaded;
	return true;
}

UBaseDataTable* UClassConfigInfoTable::GetDataTable() const
{
	return nullptr;
}

bool UClassConfigInfoTable::GetClassConfig(int32 ClassID, FClassConfigInfo& OutConfig) const
{
	if (const FClassConfigInfo* Found = ClassConfigMap.Find(ClassID))
	{
		OutConfig = *Found;
		return true;
	}
	return false;
}

TArray<int32> UClassConfigInfoTable::GetAllClassIDs() const
{
	TArray<int32> IDs;
	ClassConfigMap.GetKeys(IDs);
	return IDs;
}