#include "SL_StaminaComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_StaminaComponent::USL_StaminaComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// 默认初始化
	CurrentStaminaValue = 100.0f;
	MaxStaminaValue = 100.0f;
	IsStaminaZero = false;
	StaminaReviveBlock = 0.25f;
	StaminaZeroReviveBlock = 0.5f;
	StaminaReviveTimerBlock = 0.1f;
	StaminaReviveSingleValue = 20.0f;
}

void USL_StaminaComponent::InitStaminaInfo()
{
	// 外部提供信息初始化

}

bool USL_StaminaComponent::GetIsStaminaZero()
{
	return IsStaminaZero;
}

float USL_StaminaComponent::GetCurrentStaminaValue()
{
	return CurrentStaminaValue;
}

float USL_StaminaComponent::GetMaxStaminaValue()
{
	return MaxStaminaValue;
}

void USL_StaminaComponent::ReduceStaminaValue(float ReduceValue)
{
	if (CurrentStaminaValue - ReduceValue < 0)
	{
		// 进入力竭状态
		IsStaminaZero = true;
		OnStaminaValueZero();
	}

	// 检查精力恢复定时器是否启动
	if (StaminaReviveTimerHandle.IsValid())
	{
		// 若已启动则关闭定时器
		GetWorld()->GetTimerManager().ClearTimer(StaminaReviveTimerHandle);
	}


	if (IsStaminaZero == true)
	{
		// 启动精力启动恢复定时器
		GetWorld()->GetTimerManager().SetTimer(
			StaminaReviveTimerHandle,
			this,
			&USL_StaminaComponent::ReviveStaminaValue,
			StaminaReviveTimerBlock,
			true,
			StaminaZeroReviveBlock);
	}
	else
	{
		// 启动精力启动恢复定时器
		GetWorld()->GetTimerManager().SetTimer(
			StaminaReviveTimerHandle,
			this,
			&USL_StaminaComponent::ReviveStaminaValue,
			StaminaReviveTimerBlock,
			true,
			StaminaReviveBlock);
	}




}

void USL_StaminaComponent::OnStaminaValueZero()
{
	
}

void USL_StaminaComponent::ReviveStaminaValue()
{
	if (CurrentStaminaValue + StaminaReviveSingleValue > MaxStaminaValue)
	{
		// 体力恢复满则清空精力恢复定时器
		CurrentStaminaValue = MaxStaminaValue;
		if (StaminaReviveTimerHandle.IsValid())
		{
			// 若已启动则关闭定时器
			GetWorld()->GetTimerManager().ClearTimer(StaminaReviveTimerHandle);
		}
	}
	else
	{
		CurrentStaminaValue += StaminaReviveSingleValue;
	}

	// 解除力竭状态
	if(IsStaminaZero == true){ IsStaminaZero = false; }
	
}
