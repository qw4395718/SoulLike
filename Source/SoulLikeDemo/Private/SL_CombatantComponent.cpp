// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CombatantComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include <Engine/StreamableManager.h>
#include <Engine/AssetManager.h>

USL_CombatantComponent::USL_CombatantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USL_CombatantComponent::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{

}

int USL_CombatantComponent::GetCurrentCombatState()
{
	return 0;
}

int USL_CombatantComponent::GetTeamID()
{
	return 0;
}

void USL_CombatantComponent::OnAttackEventCall()
{

}

bool USL_CombatantComponent::CanExecute()
{
	return bWaitingForExecuted;
}

bool USL_CombatantComponent::CanBackStabs()
{
	return bAllowedBackStabsed;
}

bool USL_CombatantComponent::PerformExecuted(FName WeaponName)
{
	if (bWaitingForExecuted)
	{
		// 检查蒙太奇中是否有指定武器名字的蒙太奇动画，若无则执行默认处决动画

		UAnimInstance* AnimInstance = GetOwner()->GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(SoftMentageRefrence->Get());
		}
		return true;
	}
	else
	{
		return false;
	}

}

bool USL_CombatantComponent::PerformBackStabbed()
{
	if (bAllowedBackStabsed)
	{
		// 检查是否有蒙太奇动画是否完成异步加载，若未完成则执行同步加载


		return true;
	}
	else
	{
		return false;
	}
}

void USL_CombatantComponent::InitCombatComponentInfo()
{
	// 一堆信息
	// 蒙太奇信息
	FString MentagePath = "";
	LoadActorMentageAsync(MentagePath);
}

void USL_CombatantComponent::LoadActorMentageAsync(const FString MentagePath)
{
	if (MentagePath == "") { return; }
	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftMentageRefrence = FSoftObjectPath(*MentagePath);
	Streamable.RequestAsyncLoad(
		SoftMentageRefrence.ToSoftObjectPath()
	);
}

