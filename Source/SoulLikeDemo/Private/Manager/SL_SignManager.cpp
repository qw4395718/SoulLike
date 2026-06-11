#include "SL_SignManager.h"
#include "Manager/Online/SL_MatchClientSubsystem.h"

void USL_SignManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("USL_SignManager::Initialize - SignManager ready"));
}

void USL_SignManager::Deinitialize()
{
	ActiveSigns.Empty();
	OnSignRegistered.Clear();
	OnSignRemoved.Clear();
	OnSignInteracted.Clear();

	Super::Deinitialize();
}

FGuid USL_SignManager::RegisterSign(const FSummonSignInfo& InSignInfo)
{
	if (!InSignInfo.SignID.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_SignManager::RegisterSign - Invalid SignID"));
		return FGuid();
	}

	// 检查是否已存在
	if (ActiveSigns.Contains(InSignInfo.SignID))
	{
		UE_LOG(LogTemp, Warning, TEXT("USL_SignManager::RegisterSign - Sign %s already exists"), *InSignInfo.SignID.ToString());
		return FGuid();
	}

	// 注册标记
	FSummonSignInfo NewInfo = InSignInfo;
	NewInfo.State = ESummonSignState::Active;
	ActiveSigns.Add(NewInfo.SignID, NewInfo);

	UE_LOG(LogTemp, Log, TEXT("USL_SignManager::RegisterSign - Sign %s registered by %s at level %s"),
		*NewInfo.SignID.ToString(), *NewInfo.OwnerPlayerName, *NewInfo.CurrentLevelName.ToString());

	// 广播事件
	OnSignRegistered.Broadcast(NewInfo);

	USL_MatchClientSubsystem* MC = GetGameInstance()->GetSubsystem<USL_MatchClientSubsystem>();
	if (MC && MC->IsConnected())
	{
		FString TJSON = FString::Printf(TEXT("{\"location\":{\"x\":%.1f,\"y\":%.1f,\"z\":%.1f}}"),
			NewInfo.SignTransform.GetLocation().X,
			NewInfo.SignTransform.GetLocation().Y,
			NewInfo.SignTransform.GetLocation().Z);
		MC->RegisterSign(NewInfo.OwnerPlayerName, NewInfo.OwnerLevel,
			NewInfo.OwnerWeaponLevel, NewInfo.CurrentLevelName.ToString(),
			TJSON, NewInfo.TimeRemaining);
	}

	return NewInfo.SignID;
}

bool USL_SignManager::UnregisterSign(const FGuid& InSignID)
{
	if (!ActiveSigns.Contains(InSignID))
	{
		return false;
	}

	ActiveSigns.Remove(InSignID);

	USL_MatchClientSubsystem* MC = GetGameInstance()->GetSubsystem<USL_MatchClientSubsystem>();
	if (MC && MC->IsConnected()) MC->UnregisterSign(InSignID.ToString());

	UE_LOG(LogTemp, Log, TEXT("USL_SignManager::UnregisterSign - Sign %s removed"), *InSignID.ToString());

	OnSignRemoved.Broadcast(InSignID);
	return true;
}

TArray<FSummonSignInfo> USL_SignManager::QuerySignsByLevel(FName InLevelName) const
{
	TArray<FSummonSignInfo> Results;

	for (const auto& Pair : ActiveSigns)
	{
		if (Pair.Value.State == ESummonSignState::Active && Pair.Value.CurrentLevelName == InLevelName)
		{
			Results.Add(Pair.Value);
		}
	}

	return Results;
}

TArray<FSummonSignInfo> USL_SignManager::QuerySignsByLevelRange(FName InLevelName, int32 InPlayerLevel, int32 InWeaponLevel, const FSummonMatchConfig& InMatchConfig) const
{
	TArray<FSummonSignInfo> Results;

	for (const auto& Pair : ActiveSigns)
	{
		const FSummonSignInfo& Info = Pair.Value;

		// 只返回活跃标记
		if (Info.State != ESummonSignState::Active)
			continue;

		// 同关卡
		if (Info.CurrentLevelName != InLevelName)
			continue;

		// 等级范围校验
		int32 LevelDiff = FMath::Abs(Info.OwnerLevel - InPlayerLevel);
		if (LevelDiff > InMatchConfig.LevelRange)
			continue;

		// 武器等级范围校验
		int32 WeaponDiff = FMath::Abs(Info.OwnerWeaponLevel - InWeaponLevel);
		if (WeaponDiff > InMatchConfig.WeaponLevelRange)
			continue;

		// 密码匹配
		if (InMatchConfig.bEnablePassword && !InMatchConfig.SummonPassword.IsEmpty())
		{
			// Phase1 暂不实现密码匹配详细逻辑，预留
		}

		Results.Add(Info);
	}

	return Results;
}

bool USL_SignManager::GetSignInfo(const FGuid& InSignID, FSummonSignInfo& OutSignInfo) const
{
	const FSummonSignInfo* Found = ActiveSigns.Find(InSignID);
	if (!Found)
	{
		return false;
	}

	OutSignInfo = *Found;
	return true;
}

bool USL_SignManager::ReportSignInteraction(const FGuid& InSignID, const FString& InInteractorName)
{
	FSummonSignInfo* Found = ActiveSigns.Find(InSignID);
	if (!Found || Found->State != ESummonSignState::Active)
	{
		return false;
	}

	// 锁定标记，防止多人同时召唤同一个
	Found->State = ESummonSignState::BeingSummoned;

	UE_LOG(LogTemp, Log, TEXT("USL_SignManager::ReportSignInteraction - Sign %s locked by %s"),
		*InSignID.ToString(), *InInteractorName);

	OnSignInteracted.Broadcast(InSignID, InInteractorName);
	return true;
}

void USL_SignManager::TickSigns(float DeltaTime)
{
	TArray<FGuid> ExpiredSigns;

	for (auto& Pair : ActiveSigns)
	{
		Pair.Value.TimeRemaining -= DeltaTime;
		if (Pair.Value.TimeRemaining <= 0.0f && Pair.Value.State == ESummonSignState::Active)
		{
			Pair.Value.State = ESummonSignState::Expired;
			ExpiredSigns.Add(Pair.Key);
		}
	}

	// 移除过期标记
	for (const FGuid& SignID : ExpiredSigns)
	{
		UE_LOG(LogTemp, Verbose, TEXT("USL_SignManager::TickSigns - Sign %s expired"), *SignID.ToString());
		ActiveSigns.Remove(SignID);
		OnSignRemoved.Broadcast(SignID);
	}
}

void USL_SignManager::CleanExpiredSigns()
{
	TArray<FGuid> ToRemove;

	for (const auto& Pair : ActiveSigns)
	{
		if (Pair.Value.State == ESummonSignState::Expired || Pair.Value.State == ESummonSignState::Removed)
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (const FGuid& SignID : ToRemove)
	{
		ActiveSigns.Remove(SignID);
		OnSignRemoved.Broadcast(SignID);
	}
}
