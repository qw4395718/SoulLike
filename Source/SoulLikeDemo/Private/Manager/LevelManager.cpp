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

	// 延迟启动关卡
	FTimerHandle StartDelayHandle;
	GetWorld()->GetTimerManager().SetTimer(StartDelayHandle, FTimerDelegate::CreateLambda([this]()
		{
			StartLevel(CurrentLevelID, CurrentPlayerClassID);
		}), 2.0f, false);

	UE_LOG(LogTemp, Log, TEXT("LevelManager::BeginPlay - Level initialization started"));
}

void ALevelManager::StartLevel(int32 LevelID, int32 PlayerClassID)
{
	CurrentLevelID = LevelID;
	CurrentPlayerClassID = PlayerClassID;

	// 获取 WaveManagerSystem
	UWaveManagerSystem* WaveManagerSystem = GetWorld()->GetSubsystem<UWaveManagerSystem>();
	if (!WaveManagerSystem) return;

	// 初始化玩家装备
	ASL_CharacterBase* PlayerCharacter = Cast<ASL_CharacterBase>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (PlayerCharacter)
	{
		PlayerCharacter->InitCharacterWithClassID(PlayerClassID);
	}

	// 开始关卡流程
	WaveManagerSystem->StartLevel(LevelID);

	// 显示关卡开始UI
	ShowLevelStartUI(LevelID);

	UE_LOG(LogTemp, Log, TEXT("LevelManager::StartLevel - Level %d started with Class %d"), LevelID, PlayerClassID);
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
	UnlockNextLevel(CurrentLevelID + 1);
	SaveGameProgress();
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
	ShowPlayerDiedUI();

	// 延迟后重试
	FTimerHandle RetryTimer;
	GetWorld()->GetTimerManager().SetTimer(RetryTimer, FTimerDelegate::CreateLambda([this]()
		{
			RetryLevel();
		}), 3.0f, false);
}

void ALevelManager::RetryLevel()
{
	UWaveManagerSystem* WaveManagerSystem = GetWorld()->GetSubsystem<UWaveManagerSystem>();
	if (WaveManagerSystem)
	{
		WaveManagerSystem->ResetLevel();
	}

	StartLevel(CurrentLevelID, CurrentPlayerClassID);
}

void ALevelManager::GoToNextLevel()
{
	// TODO: 加载下一关地图
	// UGameplayStatics::OpenLevel(GetWorld(), NextLevelName);
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

	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
	{
		// TODO: 通过UIManager创建UI
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
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
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

}

void ALevelManager::ShowPlayerDiedUI()
{
	UUIManagerSubsystem* UIManager = UUIManagerSubsystem::Get(this);
	if (UIManager)
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

void ALevelManager::UnlockNextLevel(int32 NextLevelID)
{
	// 更新当前关卡ID为下一关
	CurrentLevelID = InNextLevelID;

	UE_LOG(LogTemp, Log, TEXT("LevelManager::UnlockNextLevel - Next level ID: %d"), CurrentLevelID);

	// 保存进度
	SaveGameProgress();
}