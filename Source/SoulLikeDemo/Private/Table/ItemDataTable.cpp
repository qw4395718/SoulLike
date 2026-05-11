// Private/Table/ItemDataTable.cpp
#include "ItemDataTable.h"
#include "Engine/DataTable.h"

bool UItemDataTable::InitializeFromAsset(TSoftObjectPtr<UDataTable> InTableAsset)
{
	if (!InTableAsset.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UItemDataTable::InitializeFromAsset - TableAsset is invalid"));
		return false;
	}

	UDataTable* DataTable = InTableAsset.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UItemDataTable::InitializeFromAsset - Failed to load DataTable"));
		return false;
	}

	ItemDataMap.Empty();

	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FItemDataRow* DataRow = DataTable->FindRow<FItemDataRow>(RowName, TEXT("InitializeFromAsset"));
		if (DataRow)
		{
			if (DataRow->ItemID.IsNone())
			{
				UE_LOG(LogTemp, Warning, TEXT("UItemDataTable::InitializeFromAsset - Invalid ItemID in row %s"), *RowName.ToString());
				continue;
			}

			if (ItemDataMap.Contains(DataRow->ItemID))
			{
				UE_LOG(LogTemp, Warning, TEXT("UItemDataTable::InitializeFromAsset - Duplicate ItemID: %s"), *DataRow->ItemID.ToString());
				continue;
			}

			ItemDataMap.Add(DataRow->ItemID, *DataRow);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UItemDataTable::InitializeFromAsset - Successfully loaded %d item configs"), ItemDataMap.Num());
	return true;
}

bool UItemDataTable::PostExecuteData()
{
	LoadState = ETableLoadState::Loaded;
	return true;
}

UBaseDataTable* UItemDataTable::GetDataTable() const
{
	return nullptr;
}

bool UItemDataTable::GetItemData(FName InItemID, FItemDataRow& OutItemData) const
{
	if (const FItemDataRow* Found = ItemDataMap.Find(InItemID))
	{
		OutItemData = *Found;
		return true;
	}
	return false;
}

TArray<FName> UItemDataTable::GetAllItemIDs() const
{
	TArray<FName> IDs;
	ItemDataMap.GetKeys(IDs);
	return IDs;
}