// GlobalDelegatesManager.cpp
#include "GlobalDelegatesManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

UGlobalDelegatesManager* UGlobalDelegatesManager::Instance = nullptr;

UGlobalDelegatesManager::UGlobalDelegatesManager()
{
	// 确保实例唯一
	check(!Instance);
	Instance = this;
}

UGlobalDelegatesManager* UGlobalDelegatesManager::Get(const UObject* WorldContextObject)
{
	if (!Instance)
	{
		// 尝试从GameInstance获取
		if (WorldContextObject)
		{
			UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
			if (GameInstance)
			{
				// 检查是否已存在
				Instance = GameInstance->GetSubsystem<UGlobalDelegatesManager>();
				if (!Instance)
				{
					// 创建新实例
					Instance = NewObject<UGlobalDelegatesManager>(GameInstance);
					Instance->AddToRoot(); // 防止被垃圾回收
				}
			}
		}
	}

	return Instance;
}

void UGlobalDelegatesManager::BroadcastDialogShow(const FString& labelText, const FString& descText)
{
	OnClickInterActBtnToDialog.Broadcast(labelText, descText);
}
