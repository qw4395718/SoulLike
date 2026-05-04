// Private/Manager/WaveManagerSystem.cpp

#include "WaveManagerSystem.h"
#include "WaveConfigInfoTable.h"
#include "DataTableManager.h"
#include "SL_EnemyBase.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

void UWaveManagerSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentWaveIndex = -1;
	CurrentWaveID = -1;
	TotalWaveCount = 0;
	CurrentWaveState = EWaveState::None;
	bAllWavesCompleted = false;

	UE_LOG(LogTemp, Log, TEXT("UWaveManagerSystem::Initialize"));
}

void UWaveManagerSystem::Deinitialize()
{
	// 清空所有定时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	// 清空怪物列表
	ActiveEnemies.Empty();
	AllSpawnedEnemies.Empty();

	Super::Deinitialize();
}

// ==================== 关卡流程控制 ====================

void UWaveManagerSystem::StartLevel(int32 LevelID)
{
	if (!LoadWaveConfig(LevelID))
	{
		UE_LOG(LogTemp, Error, TEXT("UWaveManagerSystem::StartLevel - Failed to load wave config for LevelID=%d"), LevelID);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("UWaveManagerSystem::StartLevel - Level=%d, TotalWaves=%d"), LevelID, TotalWaveCount);

	// 开始第一波
	CurrentWaveIndex = 0;
	StartNextWave();
}

void UWaveManagerSystem::StartNextWave()
{
	if (CurrentWaveIndex >= WaveConfigs.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("UWaveManagerSystem::StartNextWave - No more waves"));
		return;
	}

	FWaveConfigInfo& WaveConfig = WaveConfigs[CurrentWaveIndex];
	CurrentWaveID = WaveConfig.WaveID;
	CurrentWaveState = EWaveState::Preparing;

	UE_LOG(LogTemp, Log, TEXT("UWaveManagerSystem::StartNextWave - WaveID=%d, Index=%d/%d"),
		CurrentWaveID, CurrentWaveIndex + 1, TotalWaveCount);

	// 广播波次开始事件
	OnWaveStarted.Broadcast(CurrentWaveID);

	// 延迟后开始生成
	FTimerHandle DelayHandle;
	GetWorld()->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateLambda([this, &WaveConfig]()
		{
			CurrentWaveState = EWaveState::Spawning;
			SpawnWaveMonsters(WaveConfig);
		}), WaveConfig.SpawnDelay, false);
}

void UWaveManagerSystem::PauseWave()
{
	if (CurrentWaveState == EWaveState::Active)
	{
		// 暂停生成定时器
		if (SpawnTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().PauseTimer(SpawnTimerHandle);
		}

		// TODO: 暂停所有敌人AI
	}
}

void UWaveManagerSystem::ResumeWave()
{
	if (CurrentWaveState == EWaveState::Active)
	{
		// 继续生成定时器
		if (SpawnTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().UnPauseTimer(SpawnTimerHandle);
		}

		// TODO: 恢复所有敌人AI
	}
}

void UWaveManagerSystem::ResetLevel()
{
	// 销毁所有生成的怪物
	for (ASL_EnemyBase* Enemy : AllSpawnedEnemies)
	{
		if (Enemy && !Enemy->IsPendingKillPending())
		{
			Enemy->Destroy();
		}
	}

	ActiveEnemies.Empty();
	AllSpawnedEnemies.Empty();

	// 重置状态
	CurrentWaveIndex = -1;
	CurrentWaveID = -1;
	CurrentWaveState = EWaveState::None;
	bAllWavesCompleted = false;

	// 清空定时器
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	}

	UE_LOG(LogTemp, Log, TEXT("UWaveManagerSystem::ResetLevel"));
}

// ==================== 配置加载 ====================

bool UWaveManagerSystem::LoadWaveConfig(int32 LevelID)
{
	UDataTableManager* TableManager = UDataTableManager::Get(this);
	if (!TableManager) return false;

	UWaveConfigInfoTable* WaveTable = Cast<UWaveConfigInfoTable>(TableManager->GetDataTable(EDataTableType::DT_WaveConfigInfo));
	if (!WaveTable) return false;

	if (!WaveTable->GetWavesForLevel(LevelID, WaveConfigs))
	{
		return false;
	}

	TotalWaveCount = WaveConfigs.Num();
	return true;
}

// ==================== 怪物生成 ====================

void UWaveManagerSystem::SpawnWaveMonsters(FWaveConfigInfo& WaveConfig)
{
	// 准备生成数据
	SpawnerIDs.Empty();
	WaveConfig.MonsterSpawnCounts.GetKeys(SpawnerIDs);

	PendingSpawnCount = 0;
	for (const auto& Pair : WaveConfig.MonsterSpawnCounts)
	{
		PendingSpawnCount += Pair.Value;
	}
	SpawnedCount = 0;

	if (PendingSpawnCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UWaveManagerSystem::SpawnWaveMonsters - No monsters to spawn"));
		OnWaveCompletedInternal();
		return;
	}

	// 获取地图上的生成点
	TArray<FSpawnPointInfo> SpawnPoints = GetSpawnPointsForLevel(WaveConfig.LevelID);
	if (SpawnPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("UWaveManagerSystem::SpawnWaveMonsters - No spawn points found"));
		OnWaveCompletedInternal();
		return;
	}

	// 开始定时生成
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, FTimerDelegate::CreateLambda([this, &WaveConfig, SpawnPoints]()
		{
			if (SpawnedCount >= PendingSpawnCount)
			{
				// 所有怪物生成完毕
				GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
				CurrentWaveState = EWaveState::Active;
				UE_LOG(LogTemp, Log, TEXT("UWaveManagerSystem::SpawnWaveMonsters - All monsters spawned, entering Active state"));
				return;
			}

			// 找到下一个要生成的怪物类型
			int32 SpawnerID = -1;
			int32 Count	 = -1;
			bool FindFlag = false;
			for (auto& Pair : WaveConfig.MonsterSpawnCounts)
			{
				if (Pair.Value > 0)
				{
					SpawnerID = Pair.Key;
					Count = Pair.Value -1;
					FindFlag = true;
					break;
				}
			}
			if (FindFlag == true)
			{
				WaveConfig.MonsterSpawnCounts[SpawnerID] = Count;
			}

			if (SpawnerID < 0) return;

			// 选择一个生成点
			int32 PointIndex = SpawnedCount % SpawnPoints.Num();
			const FSpawnPointInfo& SpawnPoint = SpawnPoints[PointIndex];

			// 生成怪物
			FTransform SpawnTransform(SpawnPoint.Rotation, SpawnPoint.Location);
			ASL_EnemyBase* Enemy = SpawnEnemyAtPoint(SpawnerID, SpawnTransform);

			if (Enemy)
			{
				ActiveEnemies.Add(Enemy);
				AllSpawnedEnemies.Add(Enemy);

				// 绑定死亡事件
				Enemy->OnEnemyDied.AddDynamic(this, &UWaveManagerSystem::OnEnemyDiedCallback);
			}

			SpawnedCount++;
			UE_LOG(LogTemp, Verbose, TEXT("UWaveManagerSystem::SpawnWaveMonsters - Spawned %d/%d"), SpawnedCount, PendingSpawnCount);

		}), WaveConfig.SpawnInterval, true);
}

ASL_EnemyBase* UWaveManagerSystem::SpawnEnemyAtPoint(int32 SpawnerID, const FTransform& SpawnTransform)
{
	if (!GetWorld()) return nullptr;

	// TODO: 从EnemyDataTable获取敌人类型
	// 这里先使用默认的敌人基类
	TSubclassOf<ASL_EnemyBase> EnemyClass = ASL_EnemyBase::StaticClass();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASL_EnemyBase* Enemy = GetWorld()->SpawnActorDeferred<ASL_EnemyBase>(
		EnemyClass,
		SpawnTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);

	if (Enemy)
	{
		Enemy->InitializeEnemy(SpawnerID);
		Enemy->FinishSpawning(SpawnTransform);

		UE_LOG(LogTemp, Log, TEXT("UWaveManagerSystem::SpawnEnemyAtPoint - Spawned enemy at %s"), *SpawnTransform.ToString());
	}

	return Enemy;
}

// ==================== 波次状态检查 ====================

void UWaveManagerSystem::CheckWaveCompletion()
{
	// 清理已经失效的敌人
	ActiveEnemies.RemoveAll([](ASL_EnemyBase* Enemy) {
		return Enemy == nullptr || Enemy->IsPendingKillPending() || Enemy->GetEnemyState() == EEnemyState::Dead;
		});

	if (ActiveEnemies.Num() == 0 && SpawnedCount >= PendingSpawnCount)
	{
		OnWaveCompletedInternal();
	}
}

void UWaveManagerSystem::OnEnemyDiedCallback()
{
	// 从活动列表中移除
	ActiveEnemies.RemoveAll([](ASL_EnemyBase* Enemy) {
		return Enemy == nullptr || Enemy->IsPendingKillPending();
		});

	// 检查波次是否完成
	if (CurrentWaveState == EWaveState::Active && ActiveEnemies.Num() == 0 && SpawnedCount >= PendingSpawnCount)
	{
		OnWaveCompletedInternal();
	}
}

void UWaveManagerSystem::OnWaveCompletedInternal()
{
	CurrentWaveState = EWaveState::Completed;
	OnWaveCompleted.Broadcast(CurrentWaveID);

	UE_LOG(LogTemp, Log, TEXT("UWaveManagerSystem::OnWaveCompleted - WaveID=%d"), CurrentWaveID);

	const FWaveConfigInfo& WaveConfig = WaveConfigs[CurrentWaveIndex];

	// 检查是否是最后一波
	if (WaveConfig.bIsFinalWave || CurrentWaveIndex >= WaveConfigs.Num() - 1)
	{
		OnAllWavesCompletedInternal();
		return;
	}

	// 延迟后开始下一波
	CurrentWaveIndex++;
	FTimerHandle NextWaveHandle;
	GetWorld()->GetTimerManager().SetTimer(NextWaveHandle, FTimerDelegate::CreateLambda([this]()
		{
			StartNextWave();
		}), WaveConfig.NextWaveDelay, false);
}

void UWaveManagerSystem::OnAllWavesCompletedInternal()
{
	bAllWavesCompleted = true;
	CurrentWaveState = EWaveState::Completed;
	OnAllWavesCompleted.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("UWaveManagerSystem::OnAllWavesCompleted - All waves completed!"));
}

// ==================== 生成点获取 ====================

TArray<FSpawnPointInfo> UWaveManagerSystem::GetSpawnPointsForLevel(int32 LevelID)
{
	TArray<FSpawnPointInfo> SpawnPoints;

	if (!GetWorld()) return SpawnPoints;

	// 从地图上查找所有 EnemySpawnPoint 标签的Actor
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("EnemySpawnPoint"), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		FSpawnPointInfo Point;
		Point.SpawnPointID = FName(*Actor->GetName());
		Point.Location = Actor->GetActorLocation();
		Point.Rotation = Actor->GetActorRotation();
		SpawnPoints.Add(Point);
	}

	UE_LOG(LogTemp, Log, TEXT("UWaveManagerSystem::GetSpawnPointsForLevel - Found %d spawn points"), SpawnPoints.Num());
	return SpawnPoints;
}