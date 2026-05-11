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
	OnItemUsed.RemoveAll(this);
	OnItemEffectTriggered.RemoveAll(this);
	OnItemCountChanged.RemoveAll(this);
}

/************************************************************************/
/*                              委托广播方法                             */
/************************************************************************/

void UGlobalDelegatesManager::BroadcastDialogShow(const FString& InLabelText, const FString& InDescText)
{
	OnClickInterActBtnToDialog.Broadcast(InLabelText, InDescText);
}

void UGlobalDelegatesManager::BroadcastOnClickMainMenuButton(const int InWidgetIndex)
{
	OnClickInterMainMenuButton.Broadcast(InWidgetIndex);
}

/************************************************************************/
/*                              道具委托广播                             */
/************************************************************************/

void UGlobalDelegatesManager::BroadcastItemUsed(AActor* InUserActor, FName InItemID)
{
	OnItemUsed.Broadcast(InUserActor, InItemID);
}

void UGlobalDelegatesManager::BroadcastItemEffectTriggered(AActor* InUserActor, FName InItemID, AActor* InTargetActor)
{
	OnItemEffectTriggered.Broadcast(InUserActor, InItemID, InTargetActor);
}

void UGlobalDelegatesManager::BroadcastItemCountChanged(AActor* InOwnerActor, FName InItemID, int32 InNewCount)
{
	OnItemCountChanged.Broadcast(InOwnerActor, InItemID, InNewCount);
}
