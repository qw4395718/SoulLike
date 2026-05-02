// Private/Table/WeaponDataTable.cpp

#include "WeaponDataTable.h"
#include "Engine/DataTable.h"

bool UWeaponDataTable::InitializeFromAsset(TSoftObjectPtr<UDataTable> InTableAsset)
{
	if (!InTableAsset.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponDataTable::InitializeFromAsset - InTableAsset is invalid"));
		return false;
	}

	UDataTable* DataTable = InTableAsset.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWeaponDataTable::InitializeFromAsset - Failed to load DataTable"));
		return false;
	}

	// 清空现有数据
	WeaponDataMap.Empty();

	// 遍历所有行
	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FWeaponDataInfo* WeaponRow = DataTable->FindRow<FWeaponDataInfo>(RowName, TEXT("InitializeFromAsset"));
		if (WeaponRow)
		{
			// 检查武器ID是否有效
			if (WeaponRow->WeaponID <= 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("UWeaponDataTable::InitializeFromAsset - Invalid WeaponID in row %s"), *RowName.ToString());
				continue;
			}

			// 检查是否重复
			if (WeaponDataMap.Contains(WeaponRow->WeaponID))
			{
				UE_LOG(LogTemp, Warning, TEXT("UWeaponDataTable::InitializeFromAsset - Duplicate WeaponID: %d in row %s"),
					WeaponRow->WeaponID, *RowName.ToString());
				continue;
			}

			// 添加到缓存
			WeaponDataMap.Add(WeaponRow->WeaponID, *WeaponRow);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UWeaponDataTable::InitializeFromAsset - Successfully loaded %d weapons"), WeaponDataMap.Num());
	return true;
}

bool UWeaponDataTable::PostExecuteData()
{
	LoadState = ETableLoadState::Loaded;
	return true;
}

UBaseDataTable* UWeaponDataTable::GetDataTable() const
{
	return nullptr;
}

bool UWeaponDataTable::GetWeaponData(int32 InWeaponID, FWeaponDataInfo& OutWeaponData) const
{
	if (const FWeaponDataInfo* Found = WeaponDataMap.Find(InWeaponID))
	{
		OutWeaponData = *Found;
		return true;
	}
	return false;
}

TArray<int32> UWeaponDataTable::GetAllWeaponIDs() const
{
	TArray<int32> IDs;
	WeaponDataMap.GetKeys(IDs);
	return IDs;
}