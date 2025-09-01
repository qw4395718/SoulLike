// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CombatantComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include <Engine/StreamableManager.h>
#include <Engine/AssetManager.h>
#include "GameFramework/Character.h"

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
	// 检查持有者是否有效
	AActor* OwnActor = GetOwner();
	if (OwnActor == nullptr) { return false; }

	ACharacter* OwnCharacter = Cast<ACharacter>(GetOwner());
	if (bWaitingForExecuted && OwnCharacter)
	{
		// 检查资源是否已经加载
		if (UAnimMontage* LoadedMontage = SoftExecuteMentageRefrence.Get())
		{
			// 资源已成功加载
			if (UAnimInstance* AnimInstance = OwnCharacter->GetMesh()->GetAnimInstance())
			{
				if (LoadedMontage->IsValidSectionName(WeaponName))
				{
					AnimInstance->Montage_JumpToSection(WeaponName);
				}
				else
				{
					AnimInstance->Montage_Play(LoadedMontage);
				}
				return true;
			}
		}
	}
	return false;
}

bool USL_CombatantComponent::PerformBackStabbed(FName WeaponName)
{
	// 检查持有者是否有效
	AActor* OwnActor = GetOwner();
	if(OwnActor == nullptr){return false;}

	ACharacter* OwnCharacter = Cast<ACharacter>(OwnActor);
	if (bWaitingForExecuted && OwnCharacter)
	{
		// 检查资源是否已经加载
		if (UAnimMontage* LoadedMontage = SoftBackStabsMentageRefrence.Get())
		{
			// 资源已成功加载
			if (UAnimInstance* AnimInstance = OwnCharacter->GetMesh()->GetAnimInstance())
			{
				if (LoadedMontage->IsValidSectionName(WeaponName))
				{
					AnimInstance->Montage_JumpToSection(WeaponName);
				}
				else
				{
					AnimInstance->Montage_Play(LoadedMontage);
				}
				return true;
			}
		}
		else
		{
			// 如果未加载成功

		}
	}
	return false;
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
	SoftExecuteMentageRefrence = FSoftObjectPath(*MentagePath);
	Streamable.RequestAsyncLoad(
		SoftExecuteMentageRefrence.ToSoftObjectPath()
	);
}

