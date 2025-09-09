#include "SL_HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

USL_HealthComponent::USL_HealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 默认初始化
	CurrentHealthValue = 100.0f;
	MaxHealthValue = 100.0f;
	bIsAlive = true;
}

void USL_HealthComponent::InitHealthInfo(float MaxHealthSettings)
{
	// 外部提供信息初始化
	MaxHealthValue = MaxHealthSettings;
	CurrentHealthValue = MaxHealthSettings;
	bIsAlive = true;
}

bool USL_HealthComponent::IsAlive()
{
	return bIsAlive;
}

float USL_HealthComponent::GetCurrentHealthValue()
{
	return CurrentHealthValue;
}

float USL_HealthComponent::GetMaxHealthValue()
{
	return MaxHealthValue; 
}

void USL_HealthComponent::ReduceCurrentHealth(float ReduceValue)
{
	if (CurrentHealthValue + ReduceValue <= 0)
	{
		OnHealthEqualZero();
		CurrentHealthValue = 0;
	}
	else
	{
		CurrentHealthValue += ReduceValue;
	}
}

void USL_HealthComponent::OnHealthEqualZero()
{
	//广播委托-通知UI组件,音效,动画

	bIsAlive = false;
}

void USL_HealthComponent::ReviveCurrentHealth(float ReviveValue)
{
	if (CurrentHealthValue + ReviveValue >= MaxHealthValue)
	{
		CurrentHealthValue = MaxHealthValue;
	}
	else
	{
		CurrentHealthValue += ReviveValue;
	}
}
