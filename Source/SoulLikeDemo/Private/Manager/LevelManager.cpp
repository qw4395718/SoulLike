// Private/Manager/LevelManager.cpp

#include "LevelManager.h"
#include "WaveManagerSystem.h"
#include "WaveConfigInfoTable.h"
#include "DataTableManager.h"
#include "UIManagerSubsystem.h"
#include "SL_CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include <Manager/SL_GameSaveSubsystem.h>
#include <DataTableManager.h>
#include <LevelConfigInfoTable.h>
#include <GameFramework/PlayerStart.h>
#include <GlobalDelegatesManager.h>

ALevelManager::ALevelManager()
{
	PrimaryActorTick.bCanEverTick = false;

	CurrentLevelID = 1;
	CurrentPlayerClassID = 1001;
}

void ALevelManager::BeginPlay()
{
	Super::BeginPlay();

	// 获取 WaveManagerSystem
	UWaveManagerSystem* WaveManagerSystem = GetWorld()->GetSubsystem<UWaveManagerSystem>();
	if (!WaveManagerSystem)
	{
		UE_LOG(LogTemp, Error, TEXT("LevelManager::BeginPlay - WaveManagerSystem not found!"));
		return;
	}

	// 绑定波次事件
	WaveManagerSystem->OnWaveStarted.AddDynamic(this, &ALevelManager::OnWaveStarted);
	WaveManagerSystem->OnWaveCompleted.AddDynamic(this, &ALevelManager::OnWaveCompleted);
	WaveManagerSystem->OnAllWavesCompleted.AddDynamic(this, &ALevelManager::OnAllWavesCompleted);

	UE_LOG(LogTemp, Log, TEXT("LevelManager::BeginPlay - Level initialization started"));
}

/************************************************************************/
/*                              关卡控制                                */
/************************************************************************/

void ALevelManager::StartLevel(int32 LevelID, int32 PlayerClassID)
{
	CurrentLevelID = LevelID;
	CurrentPlayerClassID = PlayerClassID;

	// 获取 WaveManagerSystem
	UWaveManagerSystem* WaveManagerSystem = GetWorld()->GetSubsystem<UWaveManagerSystem>();
	if (!WaveManagerSystem) return;

	// 加载关卡配置（复活点、地图名等）
	LoadLevelConfig(LevelID);

	// 重置玩家状态（职业初始化 + 传送 + 满血满体）
	ResetPlayerState();

	// 开始关卡流程
	WaveManagerSystem->StartLevel(LevelID);

	// 显示关卡开始UI
	ShowLevelStartUI(LevelID);


	UE_LOG(LogTemp, Log, TEXT("LevelManager::StartLevel - Level %d started with Class %d"), LevelID, PlayerClassID);
}

/************************************************************************/
/*                         关卡配置加载                                 */
/************************************************************************/

void ALevelManager::LoadLevelConfig(int32 LevelID)
{
	LevelConfigTable = Cast<ULevelConfigInfoTable>(
		UDataTableManager::Get(this)->GetDataTable(EDataTableType::DT_LevelConfigInfo));

	if (LevelConfigTable)
	{
		LevelConfigTable->GetLevelConfig(LevelID, CurrentLevelConfig);
	}
}

/************************************************************************/
/*                              玩家状态管理                             */
/************************************************************************/

void ALevelManager::ResetPlayerState()
{
	ASL_CharacterBase* PlayerCharacter = Cast<ASL_CharacterBase>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("LevelManager::ResetPlayerState - PlayerCharacter not found"));
		return;
	}

	// 1. 查找玩家出生点并传送
	TeleportPlayerToStart(PlayerCharacter);

	// 2. 职业初始化（含血量/体力/魔法满值重置 + 装备加载）
	PlayerCharacter->InitCharacterWithClassID(CurrentPlayerClassID);

}

/************************************************************************/
/*                              出生点传送                              */
/************************************************************************/

void ALevelManager::TeleportPlayerToStart(ASL_CharacterBase* InPlayerCharacter)
{
	if (!InPlayerCharacter || !GetWorld())
	{
		return;
	}

	// 从关卡配置读取出生点坐标
	FVector TargetLocation = CurrentLevelConfig.PlayerSpawnLocation;
	FRotator TargetRotation = CurrentLevelConfig.PlayerSpawnRotation;

	// 如果坐标为默认零向量，回退到地图上的第一个 PlayerStart
	if (TargetLocation.IsZero())
	{
		AActor* StartSpot = UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());

		if (StartSpot)
		{
			TargetLocation = StartSpot->GetActorLocation();
			TargetRotation = StartSpot->GetActorRotation();
		}
	}

	// 执行传送
	InPlayerCharacter->SetActorLocationAndRotation(TargetLocation, TargetRotation, false, nullptr, ETeleportType::TeleportPhysics);

	UE_LOG(LogTemp, Log, TEXT("LevelManager::TeleportPlayerToStart - Teleported to %s"), *TargetLocation.ToString());
}

void ALevelManager::OnWaveStarted(int32 WaveID)
{
	UE_LOG(LogTemp, Log, TEXT("LevelManager::OnWaveStarted - Wave %d started"), WaveID);

	// 获取波次配置
	UWaveConfigInfoTable* WaveTable = Cast<UWaveConfigInfoTable>(
		UDataTableManager::Get(this)->GetDataTable(EDataTableType::DT_WaveConfigInfo));

	FWaveConfigInfo WaveConfig;
	if (WaveTable && WaveTable->GetWaveConfig(WaveID, WaveConfig))
	{
		ShowWaveStartUI(WaveConfig);
	}

	UpdateWaveHUD();
}

void ALevelManager::OnWaveCompleted(int32 WaveID)
{
	UE_LOG(LogTemp, Log, TEXT("LevelManager::OnWaveCompleted - Wave %d completed"), WaveID);

	ShowWaveCompleteUI(WaveID);
	PlayWaveCompleteEffect();
	UpdateWaveHUD();
}

void ALevelManager::OnAllWavesCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("LevelManager::OnAllWavesCompleted - All waves completed!"));

	ShowLevelCompleteUI();
	PlayLevelCompleteEffect();

	// 有下一关才更新存档
	if (GetNextLevelID() > 0)
	{
		UnlockNextLevel(GetNextLevelID());
		SaveGameProgress();
	}
}

void ALevelManager::OnPlayerDied()
{
	UE_LOG(LogTemp, Log, TEXT("LevelManager::OnPlayerDied - Player died!"));

	// 暂停波次
	UWaveManagerSystem* WaveManagerSystem = GetWorld()->GetSubsystem<UWaveManagerSystem>();
	if (WaveManagerSystem)
	{
		WaveManagerSystem->PauseWave();
	}

	PlayPlayerDiedEffect();

	UE_LOG(LogTemp, Log, TEXT("LevelManager::OnPlayerDied - Waiting for death screen input"));
}

void ALevelManager::RetryLevel()
{
	UWaveManagerSystem* WaveManagerSystem = GetWorld()->GetSubsystem<UWaveManagerSystem>();
	if (WaveManagerSystem)
	{
		WaveManagerSystem->ResetLevel();
	}

	// 重置玩家状态（传送 + 全恢复）
	ResetPlayerState();

	// 重启波次
	if (WaveManagerSystem)
	{
		WaveManagerSystem->StartLevel(CurrentLevelID);
	}

	UE_LOG(LogTemp, Log, TEXT("LevelManager::RetryLevel - Level %d retry started"), CurrentLevelID);
}

/************************************************************************/
/*                              下一关                                  */
/************************************************************************/

void ALevelManager::GoToNextLevel()
{
	int32 NextLevelID = GetNextLevelID();
	if (NextLevelID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("LevelManager::GoToNextLevel - No next level (NextLevelID=%d)"), NextLevelID);
		return;
	}

	// 读取下一关配置
	FLevelConfigInfo NextLevelConfig;
	if (LevelConfigTable && LevelConfigTable->GetLevelConfig(NextLevelID, NextLevelConfig))
	{
		// 检查是否需要跨地图
		if (NextLevelConfig.MapName != NAME_None)
		{
			// 先存档（把 NextLevelID 写进存档），新地图 GameMode 从存档读取
			CurrentLevelID = NextLevelID;
			SaveGameProgress();

			UGameplayStatics::OpenLevel(GetWorld(), NextLevelConfig.MapName);
		}
		else
		{
			// 同地图内下一关
			StartLevel(NextLevelID, CurrentPlayerClassID);
		}
	}
}

// ==================== UI控制 ====================

void ALevelManager::ShowLevelStartUI(int32 LevelID)
{
	UWaveConfigInfoTable* WaveTable = Cast<UWaveConfigInfoTable>(
		UDataTableManager::Get(this)->GetDataTable(EDataTableType::DT_WaveConfigInfo));

	if (!WaveTable) return;

	TArray<FWaveConfigInfo> Waves;
	WaveTable->GetWavesForLevel(LevelID, Waves);

	int32 TotalMonsters = 0;
	for (const FWaveConfigInfo& Wave : Waves)
	{
		for (const auto& Pair : Wave.MonsterSpawnCounts)
		{
			TotalMonsters += Pair.Value;
		}
	}

	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		// TODO: 通过UIManager创建UI
		// 打开血量UI
		UIManager->OpenScreenWidget(EWidgetType::EWIDGET_PlayerStatus,100);
		// 打开道具使用UI
		UIManager->OpenScreenWidget(EWidgetType::EWIDGET_ItemUseUI, 100);
		UE_LOG(LogTemp, Log, TEXT("LevelManager::ShowLevelStartUI - Level %d, Waves: %d, Monsters: %d"),
			LevelID, Waves.Num(), TotalMonsters);
	}

	// 3秒后自动关闭
	FTimerHandle HideTimer;
	GetWorld()->GetTimerManager().SetTimer(HideTimer, FTimerDelegate::CreateLambda([this]()
		{
			UE_LOG(LogTemp, Log, TEXT("LevelManager::ShowLevelStartUI - Auto close"));
		}), 3.0f, false);
}

void ALevelManager::ShowWaveStartUI(const FWaveConfigInfo& WaveConfig)
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UE_LOG(LogTemp, Log, TEXT("LevelManager::ShowWaveStartUI - Wave: %s"), *WaveConfig.WaveName.ToString());
	}

	// 2秒后自动关闭
	FTimerHandle HideTimer;
	GetWorld()->GetTimerManager().SetTimer(HideTimer, FTimerDelegate::CreateLambda([this]()
		{
			UE_LOG(LogTemp, Log, TEXT("LevelManager::ShowWaveStartUI - Auto close"));
		}), 2.0f, false);
}

void ALevelManager::ShowWaveCompleteUI(int32 WaveID)
{

}

void ALevelManager::ShowLevelCompleteUI()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UIManager->OpenScreenWidget(EWidgetType::EWIDGET_LevelComplete, 200);

		UE_LOG(LogTemp, Log, TEXT("LevelManager::ShowLevelCompleteUI - Level %d complete screen opened"), CurrentLevelID);
	}
}

void ALevelManager::ShowPlayerDiedUI()
{
	if (UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this))
	{
		UE_LOG(LogTemp, Log, TEXT("LevelManager::ShowPlayerDiedUI - Showing death screen"));
	}
}

void ALevelManager::UpdateWaveHUD()
{

}

// ==================== 特效控制 ====================

void ALevelManager::PlayWaveCompleteEffect()
{
	if (WaveCompleteEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), WaveCompleteEffect, FVector::ZeroVector);
	}

	if (WaveCompleteSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), WaveCompleteSound);
	}
}

void ALevelManager::PlayLevelCompleteEffect()
{
	if (LevelCompleteEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), LevelCompleteEffect, FVector::ZeroVector);
	}

	if (LevelCompleteSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), LevelCompleteSound);
	}
}

void ALevelManager::PlayPlayerDiedEffect()
{
	if (PlayerDiedEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerDiedEffect, FVector::ZeroVector);
	}

	if (PlayerDiedSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), PlayerDiedSound);
	}
}

// ==================== 存档管理 ====================

void ALevelManager::SaveGameProgress()
{
	// 通过存档子系统保存
	USL_GameSaveSubsystem* SaveSubsystem = USL_GameSaveSubsystem::Get(this);
	if (SaveSubsystem)
	{
		bool bSuccess = SaveSubsystem->SaveGame(CurrentLevelID, CurrentPlayerClassID);
		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("LevelManager::SaveGameProgress - Saved progress: Level=%d, ClassID=%d"),
				CurrentLevelID, CurrentPlayerClassID);
		}
	}
}

void ALevelManager::UnlockNextLevel(int32 InNextLevelID)
{
	// 更新当前关卡ID为下一关
	CurrentLevelID = InNextLevelID;

	UE_LOG(LogTemp, Log, TEXT("LevelManager::UnlockNextLevel - Next level ID: %d"), CurrentLevelID);

	// 保存进度
	SaveGameProgress();
}