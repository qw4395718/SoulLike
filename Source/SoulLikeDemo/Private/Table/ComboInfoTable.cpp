// Fill out your copyright notice in the Description page of Project Settings.


#include "ComboInfoTable.h"
#include "Engine/DataTable.h"
#include "Abilities/GameplayAbility.h" 

bool UComboInfoTable::InitializeFromAsset(TSoftObjectPtr<UDataTable> TableAsset)
{
	// 检查资源是否有效
	if (!TableAsset.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UComboInfoTable::InitializeFromAsset - TableAsset is invalid"));
		return false;
	}

	// 加载DataTable
	UDataTable* DataTable = TableAsset.LoadSynchronous();
	if (!DataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("UComboInfoTable::InitializeFromAsset - Failed to load DataTable"));
		return false;
	}

	// 清空现有映射表
	ComboInfoMap.Empty();

	// 遍历DataTable中的所有行
	TArray<FName> RowNames = DataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FComboInfo* ComboInfoRow = DataTable->FindRow<FComboInfo>(RowName, TEXT("InitializeFromAsset"));
		if (ComboInfoRow)
		{
			// 构建Key: Tag + InputType
			FComboLookupKey LookupKey;
			LookupKey.WindowTag = ComboInfoRow->ActiveRequireWindowTag;
			LookupKey.InputType = ComboInfoRow->InputActionType;

			// 检查Tag是否有效
			if (!LookupKey.WindowTag.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("UComboInfoTable::InitializeFromAsset - Invalid tag in row %s"), *RowName.ToString());
				continue;
			}

			// 检查InputType是否有效
			if (LookupKey.InputType == EComboInputActionType::EComboInputAction_None ||
				LookupKey.InputType >= EComboInputActionType::EComboInputAction_Max)
			{
				UE_LOG(LogTemp, Warning, TEXT("UComboInfoTable::InitializeFromAsset - Invalid InputActionType in row %s"), *RowName.ToString());
				continue;
			}

			// 检查NextAbilityClass是否有效
			if (!ComboInfoRow->NextAbilityClass)
			{
				continue;
			}

			// 检查是否重复添加（平坦化Map，Key内已包含Tag+InputType）
			if (ComboInfoMap.Contains(LookupKey))
			{
				UE_LOG(LogTemp, Warning, TEXT("UComboInfoTable::InitializeFromAsset - Duplicate combo info for Tag: %s, InputType: %d"),
					*LookupKey.WindowTag.ToString(), static_cast<int32>(LookupKey.InputType));
				continue;
			}

			// 添加到平坦化映射表
			ComboInfoMap.Add(LookupKey, *ComboInfoRow);

		}
	}

	UE_LOG(LogTemp, Log, TEXT("UComboInfoTable::InitializeFromAsset - Successfully loaded %d entries"),
		ComboInfoMap.Num());

	return true;
}

bool UComboInfoTable::PostExecuteData()
{
	LoadState = ETableLoadState::Loaded;
	return true;
}

UBaseDataTable* UComboInfoTable::GetDataTable() const
{
	return nullptr;
}

bool UComboInfoTable::FindNextComboInfo(const FGameplayTagContainer& Tags, EComboInputActionType InputActionType, FComboInfo& ComboInfo)
{
	for (const auto& pair : ComboInfoMap)
	{
		if (Tags.HasTag(pair.Key.WindowTag) && pair.Key.InputType == InputActionType)
		{
			ComboInfo = pair.Value;
			return true;
		}
	}
	return false;
}