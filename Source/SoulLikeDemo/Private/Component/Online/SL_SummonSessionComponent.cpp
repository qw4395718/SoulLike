#include "SL_SummonSessionComponent.h"
#include "SL_SummonSign.h"
#include "Manager/SL_SignManager.h"
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

	// 默认召唤标记类
	SummonSignClass = ASL_SummonSign::StaticClass();
}

void USL_SummonSessionComponent::BeginPlay()
{
	Super::BeginPlay();

	// 监听道具使用事件（召唤符道具通过此回调触发放置标记）
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		DelegateMgr->OnItemUsed.AddUObject(this, &USL_SummonSessionComponent::OnItemUsedCallback);
		UE_LOG(LogTemp, Log, TEXT("USL_SummonSessionComponent::BeginPlay - Bound to OnItemUsed"));
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

	// 锁定标记
	if (!SignManager->ReportSignInteraction(SignInfo.SignID, GetPlayerDisplayName()))
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_SummonSessionComponent::InteractWithSign - Failed to lock sign %s"),
			*SignInfo.SignID.ToString());
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
	if (InUserActor != GetOwner()) return;

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
