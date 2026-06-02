// Private/Game/SL_GameModeBase.cpp
#include "SL_GameModeBase.h"
#include "LevelManager.h"
#include "SL_CharacterBase.h"
#include "WaveManagerSystem.h"
#include "SL_GameSaveSubsystem.h"
#include "Kismet/GameplayStatics.h"

ASL_GameModeBase::ASL_GameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
	LevelManagerClass = ALevelManager::StaticClass();
	bUseSaveData = false;
}

void ASL_GameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// 创建 LevelManager
	CreateLevelManager();
}

void ASL_GameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理定时器
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	Super::EndPlay(EndPlayReason);
}

void ASL_GameModeBase::CreateLevelManager()
{
	if (!LevelManagerClass)
	{
		LevelManagerClass = ALevelManager::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags = RF_Transient;

	LevelManager = GetWorld()->SpawnActor<ALevelManager>(LevelManagerClass, FTransform::Identity, SpawnParams);

	if (LevelManager)
	{
		UE_LOG(LogTemp, Log, TEXT("SL_GameModeBase::CreateLevelManager - LevelManager created"));
	}
}

/************************************************************************/
/*                               关卡控制                                */
/************************************************************************/
void ASL_GameModeBase::StartTargetLevel(int32 InLevelID)
{
	RETURN_IF_TRUE(LevelManager == nullptr);

	int32 PlayerClassID = 1001;
	int32 LevelID = 1;

	// ===== 尝试从存档读取 =====
	USL_GameSaveSubsystem* SaveSubsystem = USL_GameSaveSubsystem::Get(this);
	if (SaveSubsystem)
	{
		bool bHasSave = SaveSubsystem->LoadGame(LevelID, PlayerClassID);
		if (!bHasSave)
		{
			PlayerClassID = 1001;
		}
	}

	// 初始化玩家装备
	InitializePlayer(PlayerClassID);

	// 开始关卡
	LevelManager->StartLevel(InLevelID, PlayerClassID);
}

void ASL_GameModeBase::RestartLevel()
{
	if (LevelManager)
	{
		LevelManager->RetryLevel();
	}
}

void ASL_GameModeBase::LoadLevel(int32 InLevelID)
{
	// 保存当前进度
	SaveCurrentProgress();

	// 加载新地图
	FString LevelName = FString::Printf(TEXT("Level_%d"), InLevelID);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName));
}

/************************************************************************/
/*                               存档控制                                */
/************************************************************************/

void ASL_GameModeBase::SaveCurrentProgress()
{
	if (!LevelManager) return;

	int32 CurrentLevelID = LevelManager->GetCurrentLevelID();
	int32 PlayerClassID = 1001;

	USL_GameSaveSubsystem* SaveSubsystem = USL_GameSaveSubsystem::Get(this);
	if (SaveSubsystem)
	{
		bool bSuccess = SaveSubsystem->SaveGame(CurrentLevelID, PlayerClassID);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("SL_GameModeBase::SaveCurrentProgress - Saved Level=%d, ClassID=%d"),
				CurrentLevelID, PlayerClassID);
		}
	}
}

/************************************************************************/
/*                               玩家初始化                              */
/************************************************************************/

void ASL_GameModeBase::InitializePlayer(int32 InPlayerClassID)
{
	ASL_CharacterBase* PlayerCharacter = Cast<ASL_CharacterBase>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (PlayerCharacter)
	{
		PlayerCharacter->SetClassID(InPlayerClassID);
		UE_LOG(LogTemp, Log, TEXT("SL_GameModeBase::InitializePlayer - Player initialized with class %d"), InPlayerClassID);
	}
}

bool ASL_GameModeBase::HasSaveData() const
{
    if (USL_GameSaveSubsystem* SaveSubsystem = USL_GameSaveSubsystem::Get(this))
    {
        return SaveSubsystem->HasSaveData();
    }
    return false;
}