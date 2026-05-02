// DataTableManager.cpp
#include "DataTableManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include <ComboInfoTable.h>
#include <WeaponDataTable.h>

// 静态常量定义
const FString UDataTableManager::DefaultConfigTablePath = TEXT("/Game/SoulLikeDemo/DataTables/Config/DT_DataTableConfig.DT_DataTableConfig");

void UDataTableManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 初始化时清空映射
	DataTableConfigs.Empty();
	LoadedDataTables.Empty();
	TableClassRegistry.Empty();

	// 注册默认的类型映射
	RegisterDefaultTableClasses();

	// 加载默认基础配置表
	InitializeManager(nullptr);

	UE_LOG(LogTemp, Log, TEXT("DataTableManager Initialized with %d registered table classes"), TableClassRegistry.Num());
}

void UDataTableManager::Deinitialize()
{
	// 清理所有已加载的数据表
	UnloadAllDataTables();

	// 清空配置映射和注册表
	DataTableConfigs.Empty();
	TableClassRegistry.Empty();

	UE_LOG(LogTemp, Log, TEXT("DataTableManager Deinitialized"));

	Super::Deinitialize();
}

UDataTableManager* UDataTableManager::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("DataTableManager::Get - WorldContextObject is null"));
		return nullptr;
	}

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("DataTableManager::Get - GameInstance is null"));
		return nullptr;
	}

	return GameInstance->GetSubsystem<UDataTableManager>();
}

void UDataTableManager::RegisterDefaultTableClasses()
{
	// ========================================
	// 在这里注册所有数据表类型对应的管理类
	// 这是唯一需要手动维护类映射的地方
	// 新增数据表时，只需要在这里添加一行注册代码
	// ========================================

	// 如果项目中有具体的BaseDataTable子类，在这里注册
	// 如果没有具体的子类，会使用默认的UBaseDataTable（通过NewObject的默认行为）

	// 示例：注册具体的子类
	// RegisterTableClass(EDataTableType::Character, UCharacterDataTable::StaticClass());
	// RegisterTableClass(EDataTableType::Item, UItemDataTable::StaticClass());
	// RegisterTableClass(EDataTableType::Skill, USkillDataTable::StaticClass());

	// 如果没有特定的子类，可以不注册或注册为nullptr
	// 系统会自动使用UBaseDataTable作为默认类

	RegisterTableClass(EDataTableType::DT_ComboInfo, UComboInfoTable::StaticClass());
	RegisterTableClass(EDataTableType::DT_WeaponDataInfo, UWeaponDataTable::StaticClass());

	UE_LOG(LogTemp, Log, TEXT("DataTableManager::RegisterDefaultTableClasses - Default classes registered"));
}

void UDataTableManager::RegisterTableClass(EDataTableType TableType, TSubclassOf<UBaseDataTable> TableClass)
{
	if (TableClass)
	{
		TableClassRegistry.Add(TableType, TableClass);
		UE_LOG(LogTemp, Log, TEXT("DataTableManager::RegisterTableClass - Registered class %s for table type %d"),
			*TableClass->GetName(), (int32)TableType);
	}
	else
	{
		// 如果传入nullptr，移除注册（恢复默认行为）
		TableClassRegistry.Remove(TableType);
		UE_LOG(LogTemp, Log, TEXT("DataTableManager::RegisterTableClass - Removed registration for table type %d"), (int32)TableType);
	}
}

void UDataTableManager::InitializeManager(UDataTable* ConfigTable)
{
	// 如果传入了配置表，使用传入的
	UDataTable* ConfigDataTable = ConfigTable;

	// 如果没有传入，尝试加载默认配置表
	if (!ConfigDataTable)
	{
		ConfigDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *DefaultConfigTablePath));
		if (!ConfigDataTable)
		{
			UE_LOG(LogTemp, Error, TEXT("DataTableManager::InitializeManager - Failed to load default config table from path: %s"), *DefaultConfigTablePath);
			return;
		}
	}

	// 清空现有配置
	DataTableConfigs.Empty();

	// 读取配置表中的所有行
	TArray<FDataTableConfig*> AllConfigs;
	ConfigDataTable->GetAllRows<FDataTableConfig>(TEXT("DataTableManager"), AllConfigs);

	// 将配置数据存储到映射中
	for (FDataTableConfig* Config : AllConfigs)
	{
		if (Config && Config->TableType != EDataTableType::DT_None)
		{
			DataTableConfigs.Add(Config->TableType, *Config);
			UE_LOG(LogTemp, Log, TEXT("DataTableManager::InitializeManager - Added config for table type: %d, name: %s, AutoLoad: %s"),
				(int32)Config->TableType,
				*Config->TableName,
				Config->bAutoLoad ? TEXT("true") : TEXT("false"));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DataTableManager::InitializeManager - Loaded %d table configs"), DataTableConfigs.Num());

	// 自动加载标记为AutoLoad的数据表
	int32 AutoLoadCount = 0;
	for (const auto& ConfigPair : DataTableConfigs)
	{
		if (ConfigPair.Value.bAutoLoad)
		{
			if (LoadTableByType(ConfigPair.Key))
			{
				AutoLoadCount++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DataTableManager::InitializeManager - Auto-loaded %d/%d tables"),
		AutoLoadCount, DataTableConfigs.Num());
}

bool UDataTableManager::LoadTableByType(EDataTableType TableType)
{
	// 1. 检查是否已经加载
	if (IsDataTableLoaded(TableType))
	{
		UE_LOG(LogTemp, Verbose, TEXT("DataTableManager::LoadTableByType - Table type %d is already loaded"), (int32)TableType);
		return true;
	}

	// 2. 获取配置
	FDataTableConfig* Config = DataTableConfigs.Find(TableType);
	if (!Config)
	{
		UE_LOG(LogTemp, Error, TEXT("DataTableManager::LoadTableByType - No config found for table type: %d"), (int32)TableType);
		return false;
	}

	// 3. 检查资产引用是否有效
	if (Config->DataTableAsset.IsNull())
	{
		UE_LOG(LogTemp, Error, TEXT("DataTableManager::LoadTableByType - DataTableAsset is null for table type: %d, name: %s"),
			(int32)TableType, *Config->TableName);
		return false;
	}

	// 4. 从注册表中查找对应的管理类（工厂方法）
	TSubclassOf<UBaseDataTable> ManagerClass = nullptr;
	if (TSubclassOf<UBaseDataTable>* RegisteredClass = TableClassRegistry.Find(TableType))
	{
		ManagerClass = *RegisteredClass;
	}

	// 5. 创建实例
	UBaseDataTable* NewDataTable = nullptr;
	if (ManagerClass)
	{
		// 使用注册的具体子类
		NewDataTable = NewObject<UBaseDataTable>(this, ManagerClass);
		UE_LOG(LogTemp, Log, TEXT("DataTableManager::LoadTableByType - Created instance of %s for table type: %d"),
			*ManagerClass->GetName(), (int32)TableType);
	}
	else
	{
		// 没有注册特定类，使用基类（适用于不需要特殊处理的通用数据表）
		NewDataTable = NewObject<UBaseDataTable>(this);
		UE_LOG(LogTemp, Log, TEXT("DataTableManager::LoadTableByType - Created default UBaseDataTable instance for table type: %d (no specific class registered)"),
			(int32)TableType);
	}

	if (!NewDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("DataTableManager::LoadTableByType - Failed to create UBaseDataTable instance for type: %d"), (int32)TableType);
		return false;
	}

	// 6. 同步加载数据表资产
	UDataTable* RawTable = Config->DataTableAsset.LoadSynchronous();
	if (!RawTable)
	{
		UE_LOG(LogTemp, Error, TEXT("DataTableManager::LoadTableByType - Failed to load DataTable asset for type: %d, path: %s"),
			(int32)TableType, *Config->DataTableAsset.ToString());
		return false;
	}

	// 7. 执行初始化流程
	if (!NewDataTable->InitializeFromAsset(Config->DataTableAsset))
	{
		UE_LOG(LogTemp, Error, TEXT("DataTableManager::LoadTableByType - InitializeFromAsset failed for table type: %d, name: %s"),
			(int32)TableType, *Config->TableName);
		return false;
	}

	// 8. 执行后处理
	if (!NewDataTable->PostExecuteData())
	{
		UE_LOG(LogTemp, Error, TEXT("DataTableManager::LoadTableByType - PostExecuteData failed for table type: %d, name: %s"),
			(int32)TableType, *Config->TableName);
		return false;
	}

	// 9. 缓存到已加载映射
	LoadedDataTables.Add(TableType, NewDataTable);

	UE_LOG(LogTemp, Log, TEXT("DataTableManager::LoadTableByType - Successfully loaded table type: %d, name: %s"),
		(int32)TableType, *Config->TableName);

	return true;
}

void UDataTableManager::LoadDataTable(EDataTableType TableType)
{
	LoadTableByType(TableType);
}

void UDataTableManager::UnloadDataTable(EDataTableType TableType)
{
	// 查找已加载的数据表
	UBaseDataTable** FoundTable = LoadedDataTables.Find(TableType);
	if (FoundTable && *FoundTable)
	{
		UBaseDataTable* TableToRemove = *FoundTable;

		// 从缓存中移除
		LoadedDataTables.Remove(TableType);

		UE_LOG(LogTemp, Log, TEXT("DataTableManager::UnloadDataTable - Unloaded table type: %d"), (int32)TableType);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DataTableManager::UnloadDataTable - Table type %d is not loaded"), (int32)TableType);
	}
}

void UDataTableManager::LoadDataTables(TArray<EDataTableType> TableTypes)
{
	UE_LOG(LogTemp, Log, TEXT("DataTableManager::LoadDataTables - Loading %d tables"), TableTypes.Num());

	for (const EDataTableType& TableType : TableTypes)
	{
		LoadTableByType(TableType);
	}
}

void UDataTableManager::UnloadAllDataTables()
{
	// 遍历并卸载所有已加载的数据表
	TArray<EDataTableType> LoadedKeys;
	LoadedDataTables.GetKeys(LoadedKeys);

	UE_LOG(LogTemp, Log, TEXT("DataTableManager::UnloadAllDataTables - Unloading %d tables"), LoadedKeys.Num());

	for (const EDataTableType& Key : LoadedKeys)
	{
		UnloadDataTable(Key);
	}

	LoadedDataTables.Empty();
}

UBaseDataTable* UDataTableManager::GetDataTable(EDataTableType TableType) const
{
	// 查找已加载的数据表
	UBaseDataTable* const* FoundBaseTable = LoadedDataTables.Find(TableType);
	if (!FoundBaseTable || !*FoundBaseTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("DataTableManager::GetDataTable - Table type %d is not loaded"), (int32)TableType);
		return nullptr;
	}

	UBaseDataTable* BaseTable = *FoundBaseTable;

	// 检查加载状态
	if (BaseTable == nullptr || !BaseTable->IsLoaded())
	{
		UE_LOG(LogTemp, Warning, TEXT("DataTableManager::GetDataTable - Table type %d exists but is not in loaded state"), (int32)TableType);
		return nullptr;
	}

	return BaseTable;
}

bool UDataTableManager::IsDataTableLoaded(EDataTableType TableType)
{
	UBaseDataTable* const* FoundTable = LoadedDataTables.Find(TableType);
	return FoundTable && *FoundTable && (*FoundTable)->IsLoaded();
}