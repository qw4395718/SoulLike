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
/*                   Phantom 客户端连接检测                              */
/************************************************************************/

void ASL_GameModeBase::PreLogin(const FString& Options, const FString& Address,
	const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	// 检查连接 URL 中是否包含 PhantomSession 参数
	FString SessionID = UGameplayStatics::ParseOption(Options, TEXT("PhantomSession"));
	if (!SessionID.IsEmpty())
	{
		ExpectedPhantomSessions.Add(SessionID);
		UE_LOG(LogTemp, Log, TEXT("GameMode: PreLogin - Expected phantom session %s"), *SessionID);
	}

	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

void ASL_GameModeBase::PostLogin(APlayerController* NewPlayer)
{
	if (!NewPlayer)
	{
		Super::PostLogin(nullptr);
		return;
	}

	// 检查这个客户端是否在等待灵体（基于 PreLogin 中保存的 ExpectedPhantomSessions）
	bool bPhantomPossessed = false;
	for (int32 i = ExpectedPhantomSessions.Num() - 1; i >= 0; i--)
	{
		const FString& SessionID = ExpectedPhantomSessions[i];
		ASL_CharacterBase* Phantom = TakePendingPhantom(SessionID);
		if (Phantom)
		{
			// 关键：在 Super::PostLogin 之前 Possess，
			// 这样 HandleStartingNewPlayer 发现已有 Pawn 会跳过 RestartPlayer
			NewPlayer->Possess(Phantom);
			ExpectedPhantomSessions.RemoveAt(i);
			bPhantomPossessed = true;

			UE_LOG(LogTemp, Log, TEXT("GameMode: PostLogin - Client possessed phantom %s (session=%s)"),
				*Phantom->GetName(), *SessionID);
			break;
		}
	}

	if (!bPhantomPossessed && ExpectedPhantomSessions.Num() > 0)
	{
		// PhantomCharacter 还没生成，保存 PC 等待
		FString SessionID = ExpectedPhantomSessions.Pop();
		WaitingPhantomControllers.Add(SessionID, NewPlayer);

		// 每帧重试，直到 Phantom 生成或超时
		FString SessionIDCopy = SessionID;
		GetWorld()->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateLambda([this, SessionIDCopy]()
			{
				RetryPhantomPossession(SessionIDCopy);
			}));

		UE_LOG(LogTemp, Log, TEXT("GameMode: PostLogin - Phantom not ready yet, queued PC for session %s"), *SessionID);
	}

	if (bPhantomPossessed)
	{
		// 灵体客户端：安全调用 Super::PostLogin
		// 已在上方 Possess 了 Phantom，HandleStartingNewPlayer 发现已有 Pawn 会跳过 RestartPlayer
		// 确保 PlayerState 创建、K2_PostLogin 等标准初始化流程正常运行
		Super::PostLogin(NewPlayer);
	}
	else
	{
		Super::PostLogin(NewPlayer);
	}
}

/************************************************************************/
/*                    灵体追踪管理                                       */
/************************************************************************/

void ASL_GameModeBase::RegisterPendingPhantom(const FString& InSessionID, ASL_CharacterBase* InPhantom)
{
	if (!InPhantom || InSessionID.IsEmpty()) return;

	PendingPhantoms.Add(InSessionID, InPhantom);

	// 检查是否有 PC 正在等待这个 PhantomCharacter
	if (APlayerController** PC = WaitingPhantomControllers.Find(InSessionID))
	{
		// Phantom 已就绪，立即 Possess
		if (APawn* OldPawn = (*PC)->GetPawn())
		{
			OldPawn->Destroy();
		}
		(*PC)->Possess(InPhantom);
		PendingPhantoms.Remove(InSessionID);
		WaitingPhantomControllers.Remove(InSessionID);

		UE_LOG(LogTemp, Log, TEXT("GameMode: RegisterPendingPhantom - Immediate possess of %s (session=%s)"),
			*InPhantom->GetName(), *InSessionID);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("GameMode: Registered pending phantom %s for session %s"),
			*InPhantom->GetName(), *InSessionID);
	}
}

ASL_CharacterBase* ASL_GameModeBase::TakePendingPhantom(const FString& InSessionID)
{
	ASL_CharacterBase** Found = PendingPhantoms.Find(InSessionID);
	if (Found)
	{
		ASL_CharacterBase* Result = *Found;
		PendingPhantoms.Remove(InSessionID);
		return Result;
	}
	return nullptr;
}

void ASL_GameModeBase::RetryPhantomPossession(const FString& InSessionID)
{
	APlayerController** PC = WaitingPhantomControllers.Find(InSessionID);
	if (!PC)
	{
		// PC 已被其他路径取出（例如 RegisterPendingPhantom 中的提前处理）
		return;
	}

	ASL_CharacterBase* Phantom = TakePendingPhantom(InSessionID);
	if (Phantom)
	{
		// Phantom 已就绪
		if (APawn* OldPawn = (*PC)->GetPawn())
		{
			OldPawn->Destroy();
		}
		(*PC)->Possess(Phantom);
		WaitingPhantomControllers.Remove(InSessionID);

		UE_LOG(LogTemp, Log, TEXT("GameMode: RetryPhantomPossession - Possessed phantom (session=%s)"), *InSessionID);
	}
	else if ((*PC)->GetPawn())
	{
		// 已有默认 Pawn（RestartPlayer 给了默认角色），尝试销毁重新 Possess
		// 为防止死循环，设定一个最大尝试次数
		static int32 RetryCount = 0;
		if (RetryCount < 100)
		{
			RetryCount++;
			FString SessionIDCopy = InSessionID;
			GetWorld()->GetTimerManager().SetTimerForNextTick(
				FTimerDelegate::CreateLambda([this, SessionIDCopy]()
				{
					RetryPhantomPossession(SessionIDCopy);
				}));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GameMode: RetryPhantomPossession - MAX RETRIES for session %s"), *InSessionID);
			WaitingPhantomControllers.Remove(InSessionID);
			RetryCount = 0;
		}
	}
	else
	{
		// 继续等待
		FString SessionIDCopy = InSessionID;
		GetWorld()->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateLambda([this, SessionIDCopy]()
			{
				RetryPhantomPossession(SessionIDCopy);
			}));
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
