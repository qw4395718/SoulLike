#include "SL_SummonSessionComponent.h"
#include "SL_SummonSign.h"
#include "Manager/SL_SignManager.h"
#include "Manager/Online/SL_MatchClientSubsystem.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Class/SL_GameModeBase.h"
#include "Manager/GlobalDelegatesManager.h"
#include "Manager/DataTableManager.h"
#include "Table/ItemDataTable.h"
#include "Class/ItemDataStruct.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include <GameFramework/PlayerState.h>

#include "Materials/MaterialInstanceDynamic.h"
#include <Animation/AnimInstance.h>
#include <Policies/CondensedJsonPrintPolicy.h>
#include <SL_CharacterBase.h>

USL_SummonSessionComponent::USL_SummonSessionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CurrentState = EOnlinePlayerState::Solo;
	CurrentSummonSign = nullptr;
	TargetSummonSign = nullptr;

}

void USL_SummonSessionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!SummonSignClass)
	{
		SummonSignClass = LoadClass<ASL_SummonSign>(
			nullptr,
			TEXT("/Game/SoulLikeDemo/Blueprints/Actor/BP_SummonSign.BP_SummonSign_C")
			);
	}
	if (!PhantomCharacterClass)
	{
		PhantomCharacterClass = LoadClass<ASL_CharacterBase>(
			nullptr,
			TEXT("/Game/SoulLikeDemo/Blueprints/CharacterLogic/SoulLike_AnimMan_CharacterBP.SoulLike_AnimMan_CharacterBP_C")
			);
	}
	
	// 监听道具使用事件（召唤符道具通过此回调触发放置标记）
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		DelegateMgr->OnItemUsed.AddUObject(this, &USL_SummonSessionComponent::OnItemUsedCallback);
		UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent::BeginPlay - Bound to OnItemUsed"));
	}

	// Phase 2: 绑定远程标记查询结果回调
	if (USL_MatchClientSubsystem* MC = GetWorld()->GetGameInstance()->GetSubsystem<USL_MatchClientSubsystem>())
	{
		MC->OnSignQueryResult.AddUObject(this, &USL_SummonSessionComponent::OnRemoteQueryResult);
		MC->OnSummonRequested.AddUObject(this, &USL_SummonSessionComponent::OnSummonRequestReceived);
		MC->OnSummonAccepted.AddUObject(this, &USL_SummonSessionComponent::OnSummonAcceptedByRemote);
		MC->OnSummonDeclined.AddUObject(this, &USL_SummonSessionComponent::OnSummonDeclinedByRemote);
		MC->OnPhantomDataReceived.AddUObject(this, &USL_SummonSessionComponent::OnPhantomDataReceived);
		MC->OnPhantomReady.AddUObject(this, &USL_SummonSessionComponent::OnPhantomReadyReceived);
		MC->OnReadyQuery.AddUObject(this, &USL_SummonSessionComponent::OnReadyQueryReceived);
		MC->OnSummonError.AddUObject(this, &USL_SummonSessionComponent::OnSummonErrorReceived);
	}

	// Phase 2: 定时查询远程标记（每 5 秒刷新一次）
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(PeriodicQueryTimerHandle, FTimerDelegate::CreateLambda([this]()
		{
			USL_MatchClientSubsystem* MC = GetWorld()->GetGameInstance()->GetSubsystem<USL_MatchClientSubsystem>();
			if (MC && MC->IsConnected())
			{
				MC->QuerySigns(GetWorld()->GetMapName(), GetPlayerLevel(), GetPlayerWeaponLevel());
			}
		}), 3.0f, true);  // true = 循环执行
	}
}

void USL_SummonSessionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理 ready_query 定时器和周期性查询定时器
	GetWorld()->GetTimerManager().ClearTimer(ReadyQueryTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(PeriodicQueryTimerHandle);

	Super::EndPlay(EndPlayReason);
}

/************************************************************************/
/*                               状态管理                                */
/************************************************************************/

void USL_SummonSessionComponent::SetState(EOnlinePlayerState InNewState)
{
	if (CurrentState == InNewState)
	{
		return;
	}

	EOnlinePlayerState OldState = CurrentState;
	CurrentState = InNewState;

	UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent::SetState - %d -> %d"), (int32)OldState, (int32)InNewState);
}

/************************************************************************/
/*                               辅助方法                               */
/************************************************************************/

FString USL_SummonSessionComponent::GetPlayerDisplayName() const
{
	APlayerController* PC = GetOwningPlayerController();
	if (PC && PC->PlayerState)
	{
		return PC->PlayerState->GetPlayerName();
	}
	return TEXT("Unknown");
}

int32 USL_SummonSessionComponent::GetPlayerLevel() const
{
	// Phase1: 默认返回 1，Phase2+ 从存档/属性系统获取
	return 1;
}

int32 USL_SummonSessionComponent::GetPlayerWeaponLevel() const
{
	// Phase1: 默认返回 1，Phase2+ 从装备系统获取
	return 1;
}

USL_SignManager* USL_SummonSessionComponent::GetSignManager() const
{
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!GameInstance) return nullptr;

	return GameInstance->GetSubsystem<USL_SignManager>();
}

APlayerController* USL_SummonSessionComponent::GetOwningPlayerController() const
{
	return Cast<APlayerController>(GetOwner());
}

ACharacter* USL_SummonSessionComponent::GetOwningCharacter() const
{
	APlayerController* PC = GetOwningPlayerController();
	return PC ? PC->GetCharacter() : nullptr;
}

/************************************************************************/
/*                         放置召唤标记                                  */
/************************************************************************/

void USL_SummonSessionComponent::PlaceSummonSign()
{
	if (!GetWorld()) return;

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_SummonSessionComponent::PlaceSummonSign - Must be called on server"));
		return;
	}

	// 状态校验
	if (CurrentState != EOnlinePlayerState::Solo && CurrentState != EOnlinePlayerState::SignActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_SummonSessionComponent::PlaceSummonSign - Invalid state: %d"), (int32)CurrentState);
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	ACharacter* Character = GetOwningCharacter();
	if (!PC || !Character)
	{
		return;
	}

	// 如果已有标记，先销毁
	if (CurrentSummonSign)
	{
		DestroyCurrentSign();
	}

	// 在当前角色位置生成标记
	FTransform SpawnTransform = Character->GetActorTransform();
	SpawnTransform.SetLocation(Character->GetActorLocation() + Character->GetActorForwardVector() * 100.0f);

	CurrentSummonSign = SpawnSummonSignActor(SpawnTransform);
	if (!CurrentSummonSign)
	{
		UE_LOG(LogTemp, Error, TEXT("USL_SummonSessionComponent::PlaceSummonSign - Failed to spawn sign actor"));
		return;
	}

	SetState(EOnlinePlayerState::SignActive);

	UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent::PlaceSummonSign - Sign placed by %s"), *GetPlayerDisplayName());
}

void USL_SummonSessionComponent::CancelSummonSign()
{
	if (CurrentState != EOnlinePlayerState::SignActive)
	{
		return;
	}

	DestroyCurrentSign();
	SetState(EOnlinePlayerState::Solo);

	UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent::CancelSummonSign - Sign cancelled"));
}

ASL_SummonSign* USL_SummonSessionComponent::SpawnSummonSignActor(const FTransform& InTransform)
{
	if (!SummonSignClass || !GetWorld())
	{
		return nullptr;
	}

	APlayerController* PC = GetOwningPlayerController();
	ACharacter* Character = GetOwningCharacter();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASL_SummonSign* Sign = GetWorld()->SpawnActorDeferred<ASL_SummonSign>(SummonSignClass, InTransform);
	if (Sign)
	{
		// 构造标记数据
		FSummonSignInfo SignInfo;
		SignInfo.SignID = FGuid::NewGuid();
		SignInfo.OwnerPlayerName = GetPlayerDisplayName();
		SignInfo.OwnerLevel = GetPlayerLevel();
		SignInfo.OwnerWeaponLevel = GetPlayerWeaponLevel();
		SignInfo.CurrentLevelName = FName(*GetWorld()->GetMapName());
		SignInfo.SignTransform = InTransform;
		SignInfo.TimeRemaining = 300.0f;	// 5分钟
		SignInfo.State = ESummonSignState::Active;
		SignInfo.WorldInstanceID = FName(TEXT("Default"));	// Phase1: 单服

		Sign->InitializeSign(SignInfo);
		Sign->FinishSpawning(InTransform);
	}

	return Sign;
}

void USL_SummonSessionComponent::DestroyCurrentSign()
{
	if (CurrentSummonSign)
	{
		CurrentSummonSign->RemoveSign();
		CurrentSummonSign = nullptr;
	}
}

/************************************************************************/
/*                         响应召唤请求                                  */
/************************************************************************/

void USL_SummonSessionComponent::AcceptSummon()
{
	if (CurrentState != EOnlinePlayerState::BeingSummoned)
	{
		return;
	}

	SetState(EOnlinePlayerState::SummonedAsPhantom);

	// 取消当前标记（如果有）
	DestroyCurrentSign();

	// Phase 2+3: 远程召唤请求 + 时序保护
	if (!PendingRequesterInstance.IsEmpty())
	{
		// 生成 Phantom 会话 ID（与 PhantomData 中的一致，用于时序保护）
		CurrentPhantomSessionID = FGuid::NewGuid().ToString(EGuidFormats::Short);
		ReadyQueryRetryCount = 0;

		USL_MatchClientSubsystem* MC = GetWorld()->GetGameInstance()->GetSubsystem<USL_MatchClientSubsystem>();
		if (MC && MC->IsConnected())
		{
			// 通知中间服务：接受召唤
			MC->AcceptSummon(PendingRemoteSignID, PendingRequesterInstance);
			UE_LOG(LogTemp, Log, TEXT("SummonSession: Remote summon accepted, notified match server"));

			// 打包 PhantomData，使用一致的 SessionID
			FPhantomData PhantomData = PackPhantomData();
			PhantomData.SummonSessionID = FGuid(CurrentPhantomSessionID);
			FString PhantomJSON = PhantomData.ToJSON();
			MC->TransferPhantomData(PendingRequesterInstance, PhantomJSON);
			UE_LOG(LogTemp, Log, TEXT("SummonSession: PhantomData sent to %s (session=%s)"),
				*PendingRequesterInstance, *CurrentPhantomSessionID);
		}

		// 启动 ready_query 定时器：每 1s 询问 B 是否准备好了，最多 10 次
		GetWorld()->GetTimerManager().SetTimer(ReadyQueryTimerHandle,
			FTimerDelegate::CreateLambda([this]()
			{
				if (!CurrentPhantomSessionID.IsEmpty())
				{
					USL_MatchClientSubsystem* MC = GetWorld() ? GetWorld()->GetGameInstance()->GetSubsystem<USL_MatchClientSubsystem>() : nullptr;
					if (MC && MC->IsConnected())
					{
						MC->SendReadyQuery(CurrentPhantomSessionID, PendingRequesterInstance);
						ReadyQueryRetryCount++;

						if (ReadyQueryRetryCount >= 10)
						{
							// 超时：中断流程
							GetWorld()->GetTimerManager().ClearTimer(ReadyQueryTimerHandle);
							OnSummonTimeout();
						}
					}
				}
			}), 1.0f, true, 0.5f);  // 首次 0.5s 后开始，之后每 1s

		// 不再立即 ClientTravel，而是等收到 phantom_ready 后由 OnPhantomReadyReceived 处理
	}
	else
	{
		// 单服 PIE 模式：无远程请求，直接跳过
		UE_LOG(LogTemp, Log, TEXT("SummonSession: AcceptSummon in PIE mode, no remote travel"));
		CurrentPhantomSessionID.Empty();
	}

	UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent::AcceptSummon - Summon accepted, waiting for phantom_ready"));
}

void USL_SummonSessionComponent::OnPhantomReadyReceived(const FString& InSessionID)
{
	UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent: OnPhantomReadyReceived Start"));
	// 检查是否匹配当前等待的会话
	if (InSessionID != CurrentPhantomSessionID) return;
	if (CurrentState != EOnlinePlayerState::SummonedAsPhantom) return;

	// 停止 ready_query 定时器
	GetWorld()->GetTimerManager().ClearTimer(ReadyQueryTimerHandle);

	// 执行 ClientTravel（B 已确认 PhantomCharacter 初始化完成）
	if (!PendingRequesterIP.IsEmpty() && PendingRequesterPort > 0)
	{
		APlayerController* PC = GetOwningPlayerController();
		if (PC)
		{
			FString TravelURL = FString::Printf(TEXT("%s:%d?PhantomSession=%s"),
				*PendingRequesterIP, PendingRequesterPort, *CurrentPhantomSessionID);
			PC->ClientTravel(TravelURL, TRAVEL_Absolute);
			UE_LOG(LogTemp, Log, TEXT("SummonSession: phantom_ready received, ClientTravel to %s"), *TravelURL);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("SummonSession: phantom_ready received but no remote IP, skip travel"));
	}

	// 清理临时数据
	CurrentPhantomSessionID.Empty();
	ReadyQueryRetryCount = 0;
	PendingRemoteSignID.Empty();
	PendingRequesterName.Empty();
	PendingRequesterInstance.Empty();
	PendingRequesterIP.Empty();
	PendingRequesterPort = 0;

	UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent: OnPhantomReadyReceived End"));

}

void USL_SummonSessionComponent::OnReadyQueryReceived(const FString& InSessionID, const FString& InRequesterInstance)
{
	// B 侧：收到 A 的 ready_query，检查当前状态
	if (CurrentState != EOnlinePlayerState::HasPhantom && CurrentState != EOnlinePlayerState::SummoningOther)
	{
		// 还没准备好（PhantomCharacter 还未生成），忽略这次查询
		UE_LOG(LogTemp, Verbose, TEXT("SummonSession: OnReadyQueryReceived ignored (state=%d)"), (int32)CurrentState);
		return;
	}

	if (USL_MatchClientSubsystem* MC = GetWorld()->GetGameInstance()->GetSubsystem<USL_MatchClientSubsystem>())
	{
		if (MC->IsConnected() && !PlacerInstanceID.IsEmpty())
		{
			// 回复 phantom_ready，通知 A 可以连接了
			MC->SendPhantomReady(InSessionID, InRequesterInstance);
			UE_LOG(LogTemp, Log, TEXT("SummonSession: OnReadyQueryReceived - sent phantom_ready to %s (session=%s)"),
				*InRequesterInstance, *InSessionID);
		}
	}
}

void USL_SummonSessionComponent::OnSummonTimeout()
{
	UE_LOG(LogTemp, Warning, TEXT("SummonSession: Summon TIMEOUT after %d retries, returning to Solo"), ReadyQueryRetryCount);

	// 清理所有待处理数据
	CurrentPhantomSessionID.Empty();
	ReadyQueryRetryCount = 0;
	PendingRemoteSignID.Empty();
	PendingRequesterName.Empty();
	PendingRequesterInstance.Empty();
	PendingRequesterIP.Empty();
	PendingRequesterPort = 0;

	SetState(EOnlinePlayerState::Solo);
}

void USL_SummonSessionComponent::OnSummonErrorReceived(const FString& InSessionID, const FString& InErrorReason)
{
	UE_LOG(LogTemp, Warning, TEXT("SummonSession: Summon error (session=%s, reason=%s) - aborting"), *InSessionID, *InErrorReason);
	OnSummonTimeout();
}

void USL_SummonSessionComponent::DeclineSummon()
{
	if (CurrentState != EOnlinePlayerState::BeingSummoned)
	{
		return;
	}

	// 恢复标记状态
	if (CurrentSummonSign)
	{
		CurrentSummonSign->InitializeSign(CurrentSummonSign->GetSignInfo());
	}

	SetState(EOnlinePlayerState::SignActive);
	PendingSummonSignID = FGuid();

	// Phase 2: 如果是远程召唤请求，通知中间服务
	if (!PendingRequesterInstance.IsEmpty())
	{
		USL_MatchClientSubsystem* MC = GetWorld()->GetGameInstance()->GetSubsystem<USL_MatchClientSubsystem>();
		if (MC && MC->IsConnected())
		{
			MC->DeclineSummon(PendingRemoteSignID, PendingRequesterInstance);
			UE_LOG(LogTemp, Log, TEXT("SummonSession: Remote summon declined, notified match server"));
		}
		PendingRemoteSignID.Empty();
		PendingRequesterName.Empty();
		PendingRequesterInstance.Empty();
	}

	UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent::DeclineSummon - Summon declined"));
}

/************************************************************************/
/*                         交互召唤标记                                  */
/************************************************************************/

void USL_SummonSessionComponent::InteractWithSign(ASL_SummonSign* InSign)
{
	if (!InSign || !GetWorld()) return;

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	if (CurrentState != EOnlinePlayerState::Solo)
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_SummonSessionComponent::InteractWithSign - Invalid state: %d"), (int32)CurrentState);
		return;
	}

	// 通过 SignManager 报告交互
	USL_SignManager* SignManager = GetSignManager();
	if (!SignManager)
	{
		return;
	}

	const FSummonSignInfo& SignInfo = InSign->GetSignInfo();

	// 校验匹配条件
	// 远程标记已在中间服务查询时经过匹配过滤，直接放行
	// 本地标记走 ActiveSigns 查询验证
	bool bIsValid = InSign->bIsRemoteSign;
	if (!bIsValid)
	{
		TArray<FSummonSignInfo> AvailableSigns = QueryAvailableSigns();
		for (const FSummonSignInfo& Info : AvailableSigns)
		{
			if (Info.SignID == SignInfo.SignID)
			{
				bIsValid = true;
				break;
			}
		}
	}

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_SummonSessionComponent::InteractWithSign - Sign %s not available for this player"),
			*SignInfo.SignID.ToString());
		return;
	}

	// Phase 2: 远程标记走中间服务，本地标记走 SignManager
	if (InSign->bIsRemoteSign)
	{
		USL_MatchClientSubsystem* MC = GetWorld()->GetGameInstance()->GetSubsystem<USL_MatchClientSubsystem>();
		if (!MC || !MC->IsConnected())
		{
			UE_LOG(LogTemp, Warning, TEXT("USL_SummonSessionComponent::InteractWithSign - Not connected to match server"));
			return;
		}

		MC->RequestSummon(InSign->RemoteSignID, GetPlayerDisplayName(),
			MC->GetInstanceID(), GetPlayerLevel());
		UE_LOG(LogTemp, Log, TEXT("SummonSession: Remote summon request sent for sign %s"), *InSign->RemoteSignID);
	}
	else
	{
		// 本地标记
		if (!SignManager->ReportSignInteraction(SignInfo.SignID, GetPlayerDisplayName()))
		{
			UE_LOG(LogTemp, Warning, TEXT("USL_SummonSessionComponent::InteractWithSign - Failed to lock sign %s"),
				*SignInfo.SignID.ToString());
			return;
		}
		InSign->LockSign();
		TargetSummonSign = InSign;
		SetState(EOnlinePlayerState::SummoningOther);
		return;
	}

	InSign->LockSign();
	TargetSummonSign = InSign;
	SetState(EOnlinePlayerState::SummoningOther);

	UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent::InteractWithSign - %s interacting with sign %s"),
		*GetPlayerDisplayName(), *SignInfo.SignID.ToString());

	// Phase2+: 通知放置者，等待确认/拒绝
}

void USL_SummonSessionComponent::OnItemUsedCallback(AActor* InUserActor, FName InItemID)
{
	// 只处理自己使用的道具
	if (InUserActor != GetOwningCharacter()) return;

	// 查表获取道具的行为类型
	UDataTableManager* TableManager = UDataTableManager::Get(this);
	if (!TableManager) return;

	UItemDataTable* ItemTable = Cast<UItemDataTable>(
		TableManager->GetDataTable(EDataTableType::DT_ItemConfigInfo));
	if (!ItemTable) return;

	FItemDataRow ItemData;
	if (!ItemTable->GetItemData(InItemID, ItemData)) return;

	// 根据行为类型分发
	switch (ItemData.UseBehavior)
	{
	case EItemUseBehavior::PlaceSummonSign:
	{
		if (CurrentState == EOnlinePlayerState::Solo)
		{
			UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent::OnItemUsedCallback - Placing summon sign"));
			PlaceSummonSign();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("USL_SummonSessionComponent::OnItemUsedCallback - Cannot place sign in state %d"), (int32)CurrentState);
		}
		break;
	}
	case EItemUseBehavior::Default:
	default:
		// 默认行为已在 GA_UseItem 中处理（应用 GE），这里无需额外逻辑
		break;
	}
}

TArray<FSummonSignInfo> USL_SummonSessionComponent::QueryAvailableSigns() const
{
	USL_SignManager* SignManager = GetSignManager();
	if (!SignManager)
	{
		return {};
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->PlayerState)
	{
		return {};
	}

	// 获取当前关卡名
	FName CurrentLevel = NAME_None;
	if (GetWorld())
	{
		CurrentLevel = FName(*GetWorld()->GetMapName());
	}

	return SignManager->QuerySignsByLevelRange(
		CurrentLevel,
		GetPlayerLevel(),
		GetPlayerWeaponLevel(),
		MatchConfig
	);
}


/************************************************************************/
/*                        Phase 2: 远程标记管理                          */
/************************************************************************/

void USL_SummonSessionComponent::OnRemoteQueryResult(const FString& InResultJSON)
{
	// 解析 JSON
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InResultJSON);
	TSharedPtr<FJsonObject> JsonObj;
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SummonSession: Failed to parse remote query result"));
		return;
	}

	// 收集本轮查询到的 sign_id，用于增量更新
	TArray<FString> IncomingSignIDs;

	// 解析 signs 数组
	const TArray<TSharedPtr<FJsonValue>>* SignsArray;
	if (!JsonObj->TryGetArrayField(TEXT("signs"), SignsArray))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("SummonSession: Received %d remote signs"), SignsArray->Num());

	for (const TSharedPtr<FJsonValue>& Val : *SignsArray)
	{
		TSharedPtr<FJsonObject> SignObj = Val->AsObject();
		if (!SignObj.IsValid()) continue;

		FString SignID = SignObj->GetStringField(TEXT("sign_id"));
		IncomingSignIDs.Add(SignID);

		if (RemoteSignActors.Contains(SignID))
		{
			continue;
		}

		FString OwnerName = SignObj->GetStringField(TEXT("owner_name"));
		int32 OwnerLevel = SignObj->GetIntegerField(TEXT("owner_level"));
		FString TransformStr = SignObj->GetStringField(TEXT("transform"));
		FString InstanceID = SignObj->GetStringField(TEXT("instance_id"));
		SpawnRemoteSignActor(SignID, OwnerName, OwnerLevel, TransformStr, InstanceID);
	}

	// 清理已过期（不在本轮查询结果中）的标记
	TArray<FString> ToRemove;
	for (const auto& Pair : RemoteSignActors)
	{
		if (!IncomingSignIDs.Contains(Pair.Key))
		{
			ToRemove.Add(Pair.Key);
		}
	}
	for (const FString& Key : ToRemove)
	{
		if (RemoteSignActors[Key] && RemoteSignActors[Key]->IsValidLowLevel())
			RemoteSignActors[Key]->Destroy();
		RemoteSignActors.Remove(Key);
	}
}

/************************************************************************/
/*               Phase 2: 远程召唤请求/确认回调                            */
/************************************************************************/

void USL_SummonSessionComponent::OnSummonRequestReceived(const FString& InSignID,
	const FString& InRequesterName, const FString& InRequesterInstance,
	const FString& InRequesterIP, int32 InRequesterPort)
{
	// 存储待处理召唤信息
	PendingRemoteSignID = InSignID;
	PendingRequesterName = InRequesterName;
	PendingRequesterInstance = InRequesterInstance;
	PendingRequesterIP = InRequesterIP;
	PendingRequesterPort = InRequesterPort;

	// 状态转换：SignActive → BeingSummoned
	SetState(EOnlinePlayerState::BeingSummoned);

	UE_LOG(LogTemp, Log, TEXT("SummonSession: Summon request received from %s (sign=%s, ip=%s:%d)"),
		*InRequesterName, *InSignID, *InRequesterIP, InRequesterPort);

	// Phase3: 弹出确认/拒绝 UI（需调用 AcceptSummon / DeclineSummon）
	// 先默认他会直接同意
	AcceptSummon();
}

void USL_SummonSessionComponent::OnSummonAcceptedByRemote(const FString& InSignID)
{
	// 召唤者侧：放置者已接受召唤
	if (CurrentState != EOnlinePlayerState::SummoningOther)
	{
		UE_LOG(LogTemp, Warning, TEXT("SummonSession: Summon accepted but not in SummoningOther state"));
		return;
	}

	SetState(EOnlinePlayerState::HasPhantom);

	UE_LOG(LogTemp, Log, TEXT("SummonSession: Summon accepted by owner (sign=%s)"), *InSignID);

	// Phase3: 准备接收 PhantomData，生成灵体
}

void USL_SummonSessionComponent::OnSummonDeclinedByRemote(const FString& InSignID)
{
	// 召唤者侧：放置者拒绝了召唤
	UE_LOG(LogTemp, Log, TEXT("SummonSession: Summon declined by owner (sign=%s)"), *InSignID);

	// 恢复标记状态（解锁）
	if (TargetSummonSign)
	{
		// 将标记状态恢复为活跃（重置 LockSign 效果）
		TargetSummonSign->InitializeSign(TargetSummonSign->GetSignInfo());
		TargetSummonSign = nullptr;
	}

	SetState(EOnlinePlayerState::Solo);

	UE_LOG(LogTemp, Log, TEXT("SummonSession: Returned to Solo after summon declined"));
}

void USL_SummonSessionComponent::SpawnRemoteSignActor(const FString& InRemoteSignID,
	const FString& InOwnerName, int32 InLevel,
	const FString& InTransformJSON, const FString& InInstanceID)
{
	if (!SummonSignClass || !GetWorld() || !GetWorld()->IsServer()) return;

	// 解析位置
	FVector SpawnLocation(0, 0, 0);
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InTransformJSON);
	TSharedPtr<FJsonObject> JsonObj;
	if (FJsonSerializer::Deserialize(Reader, JsonObj) && JsonObj.IsValid())
	{
		TSharedPtr<FJsonObject> LocObj = JsonObj->GetObjectField(TEXT("location"));
		if (LocObj.IsValid())
		{
			SpawnLocation.X = LocObj->GetNumberField(TEXT("x"));
			SpawnLocation.Y = LocObj->GetNumberField(TEXT("y"));
			SpawnLocation.Z = LocObj->GetNumberField(TEXT("z"));
		}
	}

	FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);

	ASL_SummonSign* Sign = GetWorld()->SpawnActorDeferred<ASL_SummonSign>(SummonSignClass, SpawnTransform);
	if (Sign)
	{
		// 构造标记数据
		FSummonSignInfo SignInfo;
		SignInfo.SignID = FGuid::NewGuid();
		SignInfo.OwnerPlayerName = InOwnerName;
		SignInfo.OwnerLevel = InLevel;
		SignInfo.CurrentLevelName = FName(*GetWorld()->GetMapName());
		SignInfo.SignTransform = SpawnTransform;
		SignInfo.TimeRemaining = 300.0f;
		SignInfo.State = ESummonSignState::Active;

		// 标记为远程
		Sign->bIsRemoteSign = true;
		Sign->RemoteSignID = InRemoteSignID;
		Sign->RemoteInstanceID = InInstanceID;

		Sign->InitializeSign(SignInfo);
		Sign->FinishSpawning(SpawnTransform);

		RemoteSignActors.Add(InRemoteSignID, Sign);

		UE_LOG(LogTemp, Log, TEXT("SummonSession: Spawned remote sign %s from %s"),
			*InRemoteSignID, *InOwnerName);
	}
}

// ===== FPhantomData 序列化 =====
FString FPhantomData::ToJSON() const
{
	TSharedPtr<FJsonObject> Obj = MakeShareable(new FJsonObject());

	Obj->SetStringField(TEXT("character_mesh"), CharacterMeshPath);
	Obj->SetStringField(TEXT("anim_bp"), AnimBlueprintPath);

	TArray<TSharedPtr<FJsonValue>> MatArray;
	for (const FString& M : MaterialPaths)
	{
		MatArray.Add(MakeShareable(new FJsonValueString(M)));
	}
	Obj->SetArrayField(TEXT("materials"), MatArray);

	TArray<TSharedPtr<FJsonValue>> EquipArray;
	for (const FString& E : EquipmentPaths)
	{
		EquipArray.Add(MakeShareable(new FJsonValueString(E)));
	}
	Obj->SetArrayField(TEXT("equipment"), EquipArray);

	Obj->SetNumberField(TEXT("level"), Level);
	Obj->SetNumberField(TEXT("weapon_level"), WeaponLevel);
	Obj->SetNumberField(TEXT("health_percent"), HealthPercent);
	Obj->SetNumberField(TEXT("player_class_id"), PlayerClassID);
	Obj->SetStringField(TEXT("owner_name"), OwnerName);
	Obj->SetStringField(TEXT("session_id"), SummonSessionID.ToString());

	FString OutputJSON;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&OutputJSON);
	FJsonSerializer::Serialize(Obj.ToSharedRef(), Writer);

	return OutputJSON;
}

/************************************************************************/
/*                       Phase 3: PhantomData 打包                       */
/************************************************************************/

void USL_SummonSessionComponent::OnPhantomDataReceived(const FString& InJSONData, const FString& InPlacerInstance)
{
	// 召唤者侧：收到 PhantomData，在本地生成灵体
	if (!GetWorld() || !GetWorld()->IsServer()) return;

	// 状态守卫：只有正在等待灵体的召唤者才处理
	// PIE 模式下所有组件共享同一个 MatchClientSubsystem 委托
	if (CurrentState != EOnlinePlayerState::HasPhantom && CurrentState != EOnlinePlayerState::SummoningOther)
	{
		UE_LOG(LogTemp, Verbose, TEXT("SummonSession: Ignored PhantomData (state=%d, not summoner)"), (int32)CurrentState);
		return;
	}

	// 记录放入者的 InstanceID（用于稍后回复 phantom_ready）
	PlacerInstanceID = InPlacerInstance;

	// 解析 JSON
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InJSONData);
	TSharedPtr<FJsonObject> JsonObj;
	if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("SummonSession: Failed to parse PhantomData JSON"));
		return;
	}

	// 构造 FPhantomData
	FPhantomData Data;
	Data.CharacterMeshPath = JsonObj->GetStringField(TEXT("character_mesh"));
	Data.AnimBlueprintPath = JsonObj->GetStringField(TEXT("anim_bp"));
	Data.Level = JsonObj->GetIntegerField(TEXT("level"));
	Data.WeaponLevel = JsonObj->GetIntegerField(TEXT("weapon_level"));
	Data.HealthPercent = JsonObj->GetNumberField(TEXT("health_percent"));
	Data.PlayerClassID = JsonObj->GetIntegerField(TEXT("player_class_id"));
	Data.OwnerName = JsonObj->GetStringField(TEXT("owner_name"));

	FString SessionIDStr = JsonObj->GetStringField(TEXT("session_id"));
	FGuid::Parse(SessionIDStr, Data.SummonSessionID);

	// 保存会话 ID（统一转为 Short 格式，与 ClientTravel URL 中的一致）
	{
		FGuid TmpGuid;
		if (FGuid::Parse(SessionIDStr, TmpGuid))
		{
			CurrentPhantomSessionID = TmpGuid.ToString(EGuidFormats::Short);
		}
		else
		{
			CurrentPhantomSessionID = SessionIDStr;
		}
	}

	// 材质列表
	const TArray<TSharedPtr<FJsonValue>>* MatArray;
	if (JsonObj->TryGetArrayField(TEXT("materials"), MatArray))
	{
		for (const TSharedPtr<FJsonValue>& Val : *MatArray)
		{
			Data.MaterialPaths.Add(Val->AsString());
		}
	}

	// 装备列表
	const TArray<TSharedPtr<FJsonValue>>* EquipArray;
	if (JsonObj->TryGetArrayField(TEXT("equipment"), EquipArray))
	{
		for (const TSharedPtr<FJsonValue>& Val : *EquipArray)
		{
			Data.EquipmentPaths.Add(Val->AsString());
		}
	}

	// 在召唤者附近生成灵体
	APlayerController* PC = GetOwningPlayerController();
	ACharacter* Character = GetOwningCharacter();
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("SummonSession: No character to place phantom near"));
		return;
	}

	FTransform SpawnTransform = Character->GetActorTransform();
	SpawnTransform.SetLocation(Character->GetActorLocation() + Character->GetActorForwardVector() * 150.0f);

	// 使用可配置的灵体类（可在蓝图中替换以调整 Mesh 位置/旋转）
	UClass* PhantomClass = PhantomCharacterClass ? PhantomCharacterClass.Get() : ASL_CharacterBase::StaticClass();
	ASL_CharacterBase* Phantom = GetWorld()->SpawnActorDeferred<ASL_CharacterBase>(
		PhantomClass, SpawnTransform);
	if (Phantom)
	{
		Phantom->ApplyPhantomData(Data);
		Phantom->FinishSpawning(SpawnTransform);

		// 职业初始化（能力赋予、装备加载、属性设置）
		Phantom->InitCharacterWithClassID(Data.PlayerClassID);

		// 注册到 GameMode（等待灵体客户端连接后 Possess）
		if (ASL_GameModeBase* GM = Cast<ASL_GameModeBase>(GetWorld()->GetAuthGameMode()))
		{
			GM->RegisterPendingPhantom(CurrentPhantomSessionID, Phantom);
		}

		UE_LOG(LogTemp, Log, TEXT("SummonSession: Phantom spawned for %s (session=%s), registered with GameMode"),
			*Data.OwnerName, *SessionIDStr);
	}

	SetState(EOnlinePlayerState::HasPhantom);
}

FPhantomData USL_SummonSessionComponent::PackPhantomData() const
{
	FPhantomData Data;

	APlayerController* PC = GetOwningPlayerController();
	ACharacter* Character = GetOwningCharacter();
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("SummonSession: PackPhantomData - No character found"));
		return Data;
	}

	// 收集角色外观
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (Mesh && Mesh->SkeletalMesh)
	{
		Data.CharacterMeshPath = Mesh->SkeletalMesh->GetPathName();
	}

	// 收集动画蓝图路径
	if (Mesh && Mesh->AnimScriptInstance)
	{
		Data.AnimBlueprintPath = Mesh->AnimScriptInstance->GetClass()->GetPathName();
	}

	// 收集材质（按材质索引顺序）
	// 仅收集静态材质资源路径，跳过运行时动态实例
	if (Mesh)
	{
		for (int32 i = 0; i < Mesh->GetNumMaterials(); i++)
		{
			UMaterialInterface* Mat = Mesh->GetMaterial(i);
			if (Mat)
			{
				// 如果是动态材质实例，取其父级静态材质路径
				UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Mat);
				if (MID && MID->Parent)
				{
					Data.MaterialPaths.Add(MID->Parent->GetPathName());
				}
				else
				{
					Data.MaterialPaths.Add(Mat->GetPathName());
				}
			}
			else
			{
				Data.MaterialPaths.Add(TEXT(""));
			}
		}
	}

	// Phase3: 装备路径收集此处为占位，后续从装备系统获取
	// Data.EquipmentPaths = GetEquipmentSystem()->GetEquippedItemPaths();

	// 角色属性
	Data.Level = GetPlayerLevel();
	Data.WeaponLevel = GetPlayerWeaponLevel();

	// 获取当前血量百分比
	if (1)
	{
		// 如果将来实现了 IDamageable 接口，从接口获取
		Data.HealthPercent = 1.0f;
	}
	else
	{
		// 默认满血
		Data.HealthPercent = 1.0f;
	}

	// 获取角色职业ID
	{
		ASL_CharacterBase* Char = Cast<ASL_CharacterBase>(GetOwningCharacter());
		if (Char)
		{
			Data.PlayerClassID = Char->GetClassID();
		}
	}

	Data.OwnerName = GetPlayerDisplayName();
	Data.SummonSessionID = FGuid::NewGuid();

	UE_LOG(LogTemp, Log, TEXT("SummonSession: PhantomData packed for %s (mesh=%s, level=%d)"),
		*Data.OwnerName, *Data.CharacterMeshPath, Data.Level);

	return Data;
}

void USL_SummonSessionComponent::ClearRemoteSignActors()
{
	for (const auto& Pair : RemoteSignActors)
	{
		if (Pair.Value && Pair.Value->IsValidLowLevel())
		{
			Pair.Value->Destroy();
		}
	}
	RemoteSignActors.Empty();
}