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
			// 构建映射表：Tag -> (InputActionType -> ComboInfo)
			FGameplayTag RequiredTag = ComboInfoRow->ActiveRequireWindowTag;
			EComboInputActionType InputType = ComboInfoRow->InputActionType;

			// 检查Tag是否有效
			if (!RequiredTag.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("UComboInfoTable::InitializeFromAsset - Invalid tag in row %s"), *RowName.ToString());
				continue;
			}

			// 检查InputType是否有效
			if (InputType == EComboInputActionType::EComboInputAction_None ||
				InputType >= EComboInputActionType::EComboInputAction_Max)
			{
				UE_LOG(LogTemp, Warning, TEXT("UComboInfoTable::InitializeFromAsset - Invalid InputActionType in row %s"), *RowName.ToString());
				continue;
			}

			// 检查NextAbilityClass是否有效
			if (!ComboInfoRow->NextAbilityClass)
			{
				continue;
			}

			// 添加到映射表
			TMap<EComboInputActionType, FComboInfo>& InputActionMap = ComboInfoMap.FindOrAdd(RequiredTag);

			// 检查是否重复添加
			if (InputActionMap.Contains(InputType))
			{
				UE_LOG(LogTemp, Warning, TEXT("UComboInfoTable::InitializeFromAsset - Duplicate combo info for Tag: %s, InputType: %d"),
					*RequiredTag.ToString(), static_cast<int32>(InputType));
				continue;
			}

			InputActionMap.Add(InputType, *ComboInfoRow);

		}
	}

	UE_LOG(LogTemp, Log, TEXT("UComboInfoTable::InitializeFromAsset - Successfully loaded %d tags"),
		ComboInfoMap.Num());

	return true;
}

bool UComboInfoTable::PostExecuteData()
{
	return true;
}

UBaseDataTable* UComboInfoTable::GetDataTable() const
{
	return nullptr;
}

bool UComboInfoTable::FindNextComboInfo(const FGameplayTagContainer& Tags, EComboInputActionType InputActionType, FComboInfo& ComboInfo)
{
	// 遍历ComboInfoMap
	for (const auto& pair : ComboInfoMap)
	{
		if (Tags.HasTag(pair.Key))
		{
			const auto& subPair = pair.Value;
			if (subPair.Contains(InputActionType))
			{
				// 获取对应的信息
				ComboInfo = subPair[InputActionType];
				return true;
			}
		}
	}
	return false;
}
