// DataTableManager.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "DataTableConfig.h"
#include "BaseDataTable.h"
#include "DataTableManager.generated.h"


UCLASS(BlueprintType, Blueprintable)
class SOULLIKEDEMO_API UDataTableManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/****************************************************************************/
	/*										外部调用									*/
	/****************************************************************************/
	// 获取单例实例
	UFUNCTION(BlueprintPure, Category = "UIManagerSubsystem", meta = (WorldContext = "WorldContextObject"))
		static UDataTableManager* Get(const UObject* WorldContextObject);

	// 初始化数据表管理器
	UFUNCTION(BlueprintCallable, Category = "DataTable Manager")
		void InitializeManager(UDataTable* ConfigTable = nullptr);

	// 获取指定类型的数据表
	UFUNCTION(BlueprintCallable, Category = "DataTable Manager", meta = (DeterminesOutputType = "TableType"))
		UBaseDataTable* GetDataTable(EDataTableType TableType) const;

	// 检查数据表是否已加载
	UFUNCTION(BlueprintPure, Category = "DataTable Manager")
		bool IsDataTableLoaded(EDataTableType TableType);

	// 注册数据表类型对应的管理类（公共接口，方便外部扩展）
	UFUNCTION(BlueprintCallable, Category = "DataTable Manager")
		void RegisterTableClass(EDataTableType TableType, TSubclassOf<UBaseDataTable> TableClass);


protected:
	/****************************************************************************/
	/*										内部调用									*/
	/****************************************************************************/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 加载指定类型的数据表
	UFUNCTION(BlueprintCallable, Category = "DataTable Manager")
		void LoadDataTable(EDataTableType TableType);

	// 卸载指定类型的数据表
	UFUNCTION(BlueprintCallable, Category = "DataTable Manager")
		void UnloadDataTable(EDataTableType TableType);

	// 批量加载数据表
	UFUNCTION(BlueprintCallable, Category = "DataTable Manager")
		void LoadDataTables(TArray<EDataTableType> TableTypes);

	// 卸载所有数据表
	UFUNCTION(BlueprintCallable, Category = "DataTable Manager")
		void UnloadAllDataTables();

	// 初始化默认的类型注册
	void RegisterDefaultTableClasses();

	// 核心加载逻辑
	bool LoadTableByType(EDataTableType TableType);


private:

	// 数据表配置映射
	UPROPERTY()
		TMap<EDataTableType, FDataTableConfig> DataTableConfigs;

	// 已加载的数据表缓存
	UPROPERTY()
		TMap<EDataTableType, UBaseDataTable*> LoadedDataTables;

	// 类型注册表（EDataTableType -> BaseDataTable子类）
	UPROPERTY()
		TMap<EDataTableType, TSubclassOf<UBaseDataTable>> TableClassRegistry;

	// 默认配置表
	UPROPERTY()
		UDataTable* DefaultConfigTable;

	// 默认配置表路径
	static const FString DefaultConfigTablePath;
};
