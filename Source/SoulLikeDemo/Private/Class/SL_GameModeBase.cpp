// Private/Game/SL_GameModeBase.cpp
#include "SL_GameModeBase.h"
#include "LevelManager.h"
#include "SL_CharacterBase.h"
#include "WaveManagerSystem.h"
#include "Kismet/GameplayStatics.h"

ASL_GameModeBase::ASL_GameModeBase()
{
	PrimaryActorTick.bCanEverTick = false;
	LevelManagerClass = ALevelManager::StaticClass();
}

void ASL_GameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// 创建 LevelManager
	CreateLevelManager();

	// 延迟启动关卡
	FTimerHandle StartDelayHandle;
	GetWorld()->GetTimerManager().SetTimer(StartDelayHandle, FTimerDelegate::CreateLambda([this]()
		{
			StartCurrentLevel();
		}), LevelStartDelay, false);

	UE_LOG(LogTemp, Log, TEXT("SL_GameModeBase::BeginPlay - GameMode initialized"));
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
	SpawnParams.ObjectFlags = RF_Transient;  // 不保存到地图

	LevelManager = GetWorld()->SpawnActor<ALevelManager>(LevelManagerClass, FTransform::Identity, SpawnParams);

	if (LevelManager)
	{
		UE_LOG(LogTemp, Log, TEXT("SL_GameModeBase::CreateLevelManager - LevelManager created"));
	}
}

void ASL_GameModeBase::StartCurrentLevel()
{
	if (!LevelManager) return;

	// 从存档读取玩家职业和关卡ID
	int32 PlayerClassID = 1001;  // TODO: 从存档读取
	int32 LevelID = 1;        // TODO: 从存档读取

	// 初始化玩家装备
	InitializePlayer(PlayerClassID);

	// 开始关卡
	LevelManager->StartLevel(LevelID, PlayerClassID);

	UE_LOG(LogTemp, Log, TEXT("SL_GameModeBase::StartCurrentLevel - Level %d started"), LevelID);
}

void ASL_GameModeBase::RestartLevel()
{
	if (LevelManager)
	{
		LevelManager->RetryLevel();
	}
}

void ASL_GameModeBase::LoadLevel(int32 LevelID)
{
	// 保存当前进度
	// TODO: 存档系统

	// 加载新地图
	FString LevelName = FString::Printf(TEXT("Level_%d"), LevelID);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelName));
}

void ASL_GameModeBase::InitializePlayer(int32 PlayerClassID)
{
	ASL_CharacterBase* PlayerCharacter = Cast<ASL_CharacterBase>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (PlayerCharacter)
	{
		PlayerCharacter->InitEquipmentWithClass(PlayerClassID);
		UE_LOG(LogTemp, Log, TEXT("SL_GameModeBase::InitializePlayer - Player initialized with class %d"), PlayerClassID);
	}
}