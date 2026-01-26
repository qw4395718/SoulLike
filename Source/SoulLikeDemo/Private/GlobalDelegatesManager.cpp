// GlobalDelegatesManager.cpp
#include "GlobalDelegatesManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

void UGlobalDelegatesManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGlobalDelegatesManager::Deinitialize()
{
	// 解除绑定
	ClearAllBinding();

	Super::Deinitialize();
}

UGlobalDelegatesManager* UGlobalDelegatesManager::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("UGlobalDelegatesManager::Get: WorldContextObject is null"));
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("UGlobalDelegatesManager::Get: Cannot get World"));
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("UGlobalDelegatesManager::Get: Cannot get GameInstance"));
		return nullptr;
	}

	// ✅ 正确方式：通过Subsystem系统获取
	return GameInstance->GetSubsystem<UGlobalDelegatesManager>();
}

void UGlobalDelegatesManager::ClearAllBinding()
{
	OnClickInterActBtnToDialog.RemoveAll(this);
	OnClickInterMainMenuButton.RemoveAll(this);
}

void UGlobalDelegatesManager::BroadcastDialogShow(const FString& labelText, const FString& descText)
{
	OnClickInterActBtnToDialog.Broadcast(labelText, descText);
}

void UGlobalDelegatesManager::BroadcastOnClickMainMenuButton(const int widgetIndex)
{
	OnClickInterMainMenuButton.Broadcast(widgetIndex);
}
