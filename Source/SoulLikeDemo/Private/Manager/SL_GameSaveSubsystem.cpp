// Private/Save/SL_GameSaveSubsystem.cpp
#include "SL_GameSaveSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

const FString USL_GameSaveSubsystem::SaveSlotName = TEXT("SoulLikeSaveSlot");
const int32 USL_GameSaveSubsystem::SaveUserIndex = 0;

USL_GameSaveSubsystem::USL_GameSaveSubsystem()
{
	CurrentSaveData = nullptr;
}

void USL_GameSaveSubsystem::Initialize(FSubsystemCollectionBase& InCollection)
{
	Super::Initialize(InCollection);

	UE_LOG(LogTemp, Log, TEXT("USL_GameSaveSubsystem::Initialize - SaveSystem initialized"));
}

void USL_GameSaveSubsystem::Deinitialize()
{
	CurrentSaveData = nullptr;

	Super::Deinitialize();
}

USL_GameSaveSubsystem* USL_GameSaveSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("USL_GameSaveSubsystem::Get - WorldContextObject is null"));
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		// CDO / 默认子对象在 Cook 和引擎初始化阶段没有有效的 World 上下文
		// 这是预期行为，不打印错误日志
		if (WorldContextObject->IsTemplate())
		{
			return nullptr;
		}
		UE_LOG(LogTemp, Error, TEXT("UGlobalDelegatesManager::Get: Cannot get World"));
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance) return nullptr;

	return GameInstance->GetSubsystem<USL_GameSaveSubsystem>();
}

/************************************************************************/
/*                               外部调用                                */
/************************************************************************/

bool USL_GameSaveSubsystem::SaveGame(int32 InLevelID, int32 InPlayerClassID)
{
	// 1. 创建存档对象
	USL_GameSaveData* SaveData = nullptr;

	if (HasSaveData())
	{
		// 已有存档，读取后更新
		SaveData = Cast<USL_GameSaveData>(
			UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
	}

	if (!SaveData)
	{
		// 没有存档或读取失败，创建新的
		SaveData = Cast<USL_GameSaveData>(
			UGameplayStatics::CreateSaveGameObject(USL_GameSaveData::StaticClass()));
	}

	if (!SaveData)
	{
		UE_LOG(LogTemp, Error, TEXT("USL_GameSaveSubsystem::SaveGame - Failed to create save data object"));
		return false;
	}

	// 2. 更新数据
	SaveData->SavedLevelID = InLevelID;
	SaveData->SavedPlayerClassID = InPlayerClassID;
	SaveData->SaveTime = FDateTime::Now();

	// 3. 写入磁盘
	bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, SaveUserIndex);

	if (bSuccess)
	{
		// 更新缓存
		CurrentSaveData = SaveData;
		UE_LOG(LogTemp, Log, TEXT("USL_GameSaveSubsystem::SaveGame - Saved Level=%d, ClassID=%d"),
			InLevelID, InPlayerClassID);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("USL_GameSaveSubsystem::SaveGame - Failed to write to slot %s"), *SaveSlotName);
	}

	return bSuccess;
}

bool USL_GameSaveSubsystem::LoadGame(int32& OutLevelID, int32& OutPlayerClassID)
{
	// 设置默认值
	OutLevelID = 1;
	OutPlayerClassID = 1001;

	// 1. 检查存档是否存在
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_GameSaveSubsystem::LoadGame - No save data found in slot %s"), *SaveSlotName);
		return false;
	}

	// 2. 从磁盘加载
	USL_GameSaveData* LoadedData = Cast<USL_GameSaveData>(
		UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));

	if (!LoadedData)
	{
		UE_LOG(LogTemp, Error, TEXT("USL_GameSaveSubsystem::LoadGame - Failed to load from slot %s"), *SaveSlotName);
		return false;
	}

	// 3. 更新缓存
	CurrentSaveData = LoadedData;

	// 4. 返回值
	OutLevelID = LoadedData->SavedLevelID;
	OutPlayerClassID = LoadedData->SavedPlayerClassID;

	UE_LOG(LogTemp, Log, TEXT("USL_GameSaveSubsystem::LoadGame - Loaded Level=%d, ClassID=%d, SaveTime=%s"),
		OutLevelID,
		OutPlayerClassID,
		*LoadedData->SaveTime.ToString());

	return true;
}

bool USL_GameSaveSubsystem::HasSaveData() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex);
}

bool USL_GameSaveSubsystem::DeleteSaveData()
{
	bool bSuccess = UGameplayStatics::DeleteGameInSlot(SaveSlotName, SaveUserIndex);

	if (bSuccess)
	{
		CurrentSaveData = nullptr;
		UE_LOG(LogTemp, Log, TEXT("USL_GameSaveSubsystem::DeleteSaveData - Deleted save slot %s"), *SaveSlotName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_GameSaveSubsystem::DeleteSaveData - Failed to delete slot %s"), *SaveSlotName);
	}

	return bSuccess;
}