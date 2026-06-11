#include "SL_SummonSessionComponent.h"
#include "SL_SummonSign.h"
#include "Manager/SL_SignManager.h"
#include "Manager/Online/SL_MatchClientSubsystem.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Manager/GlobalDelegatesManager.h"
#include "Manager/DataTableManager.h"
#include "Table/ItemDataTable.h"
#include "Class/ItemDataStruct.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include <GameFramework/PlayerState.h>

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
	}

	// Phase 2: 定时查询远程标记（每 5 秒刷新一次）
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FTimerHandle H;
		GetWorld()->GetTimerManager().SetTimer(H, FTimerDelegate::CreateLambda([this]()
		{
			USL_MatchClientSubsystem* MC = GetWorld()->GetGameInstance()->GetSubsystem<USL_MatchClientSubsystem>();
			if (MC && MC->IsConnected())
			{
				MC->QuerySigns(GetWorld()->GetMapName(), GetPlayerLevel(), GetPlayerWeaponLevel());
			}
		}), 3.0f, true);  // true = 循环执行
	}
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

	UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent::AcceptSummon - Summon accepted"));

	// Phase2+: 触发世界穿梭流程，保存当前世界状态，切换到召唤者世界
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
	TArray<FSummonSignInfo> AvailableSigns = QueryAvailableSigns();
	bool bIsValid = false;
	for (const FSummonSignInfo& Info : AvailableSigns)
	{
		if (Info.SignID == SignInfo.SignID)
		{
			bIsValid = true;
			break;
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