#include "SL_MovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

USL_MovementComponent::USL_MovementComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USL_MovementComponent::InitMovemenetInfo()
{
	// 由外部信息进行初始化

}

void USL_MovementComponent::ExeRoll()
{
	// 检查资源是否加载
	if (CanExeRoll() == true)
	{

	}
	else
	{
		// 进行同步加载
	}
	// 执行翻滚

}

bool USL_MovementComponent::CanExeRoll()
{
	// 检查蒙太奇是否已加载成功
	return false;
}

void USL_MovementComponent::LoadMovementMentageAsync(const FString MentagePath)
{
	if (MentagePath == "") { return; }
	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftMentageRefrence = FSoftObjectPath(*MentagePath);
	Streamable.RequestAsyncLoad(
		SoftMentageRefrence.ToSoftObjectPath()
	);
}
