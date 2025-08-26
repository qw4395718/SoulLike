#include "SL_WeaponComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"
#include "WeaponMeleeAttackComponent.h"
#include "WeaponParryComponent.h"

ASL_WeaponComponent::ASL_WeaponComponent()
{
	//是否开启tick
	PrimaryActorTick.bCanEverTick = false;

	/************************************************************************/
	/*                              组件初始化                                        */
	/************************************************************************/
	// 场景组件root
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	// 骨骼网格体-
	SkeletalWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalWeaponMesh->SetupAttachment(RootComponent);

	/************************************************************************/
	/*                              变量初始化                                        */
	/************************************************************************/
	OwnerActor = nullptr;
	WeaponID = 0;
	SoftMeshReference = nullptr;
	CollisionBoxSize = FVector(0.0f,0.0f,0.0f);
	SoftMentageRefrence = nullptr;
	SoftWeaponAnimInstanceReference = nullptr;
	WeaponComponentMap.Reset();
	WeaponLoadComponentInfoMap.Reset();
	WeaponEquipInfo = EWeaponEquipState::No_Equip;
}

void ASL_WeaponComponent::InitWeaponInfo(const FWeaponDefinition& WeaponInfo)
{
	if(WeaponInfo.WeaponID == 0){return;}
	// 变量赋值
	OwnerActor = GetOwner();
	WeaponID = WeaponInfo.WeaponID;
	// 加载武器模型
	LoadWeaponMeshAsync(WeaponInfo.Mesh);
	CollisionBoxSize = WeaponInfo.WeaponCollisionBoxSize;
	// 加载武器动作蓝图
	LoadWeaponAnimInstanceAsync(WeaponInfo.AnimClass);
	// 加载武器蒙太奇
	LoadWeaponMentageAsync(WeaponInfo.MentageName);
	// 加载武器模组
	LoadWeaponComponents(WeaponInfo.NeedLoadComponentInfoMap);

}

void ASL_WeaponComponent::UpdateWeaponEquipState(EWeaponEquipState CurrentState)
{
	WeaponEquipInfo = CurrentState;
}

void ASL_WeaponComponent::LeftMouseCallEvent()
{

}

void ASL_WeaponComponent::RightMouseCallEvent()
{

}

void ASL_WeaponComponent::CtrlMouseCallEvent()
{

}

void ASL_WeaponComponent::LoadWeaponMeshAsync(const FString WeaponMeshName)
{
	if (WeaponMeshName == "") { return; }
	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftMeshReference = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(*WeaponMeshName));
	Streamable.RequestAsyncLoad(
		SoftMeshReference.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ASL_WeaponComponent::OnLoadedWeaponMesh)
	);
}

void ASL_WeaponComponent::OnLoadedWeaponMesh()
{
	if (USkeletalMesh* Mesh = SoftMeshReference.Get()) {
		SkeletalWeaponMesh->SetSkeletalMesh(Mesh);
	}
}

void ASL_WeaponComponent::LoadWeaponMentageAsync(const FString MentagePath)
{
	if (MentagePath == ""){return;}
	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftMentageRefrence = FSoftObjectPath(*MentagePath);
	Streamable.RequestAsyncLoad(
		SoftMentageRefrence.ToSoftObjectPath()
	);
	
}

void ASL_WeaponComponent::LoadWeaponAnimInstanceAsync(const FString WeapinAnimName)
{
	if (WeapinAnimName == "") { return; }
	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	FSoftObjectPath AnimPathStr(WeapinAnimName + "_C");
	SoftWeaponAnimInstanceReference = AnimPathStr;
	Streamable.RequestAsyncLoad(
		SoftWeaponAnimInstanceReference.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ASL_WeaponComponent::OnLoadedWeaponAnimInstance)
	);
}

void ASL_WeaponComponent::OnLoadedWeaponAnimInstance()
{
	if (SoftWeaponAnimInstanceReference.Get() != nullptr)
	{
		SkeletalWeaponMesh->SetAnimInstanceClass(SoftWeaponAnimInstanceReference.Get());
	}
}

bool ASL_WeaponComponent::LoadWeaponComponents(const TMap<EWeaponComponentType, bool>& pWeaponComponentInfo)
{
	// 根据map信息创建各个模组,并将创建的实际情况更新至WeaponLoadComponentInfoMap
	WeaponLoadComponentInfoMap.Reset();
	for (const auto& Pair : pWeaponComponentInfo)
	{
		if(Pair.Value  !=  true){continue;}
		TPair<EWeaponComponentType, bool> NewComponent;
		if (Pair.Key == EWeaponComponentType::MeleeAttack)
		{
			UWeaponMeleeAttackComponent* MeleeComponent = GetWorld()->SpawnActor<UWeaponMeleeAttackComponent>(
				UWeaponMeleeAttackComponent::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			if (MeleeComponent)
			{
				MeleeComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				MeleeComponent->InitalizeWeaponComponent(this, CollisionBoxSize);
				NewComponent.Key = Pair.Key;
				NewComponent.Value = true;
				WeaponComponentMap.Add(TPair<EWeaponComponentType, USceneComponent*>(Pair.Key, MeleeComponent));
			}
			else
			{
				NewComponent.Key = Pair.Key;
				NewComponent.Value = true;
			}
		}
		else if(Pair.Key == EWeaponComponentType::Parry)
		{
			UWeaponParryComponent* ParryComponent = GetWorld()->SpawnActor<UWeaponParryComponent>(
				UWeaponParryComponent::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
			if (ParryComponent)
			{
				ParryComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				ParryComponent->InitalizeWeaponComponent(this, CollisionBoxSize);
				NewComponent.Key = Pair.Key;
				NewComponent.Value = true;
				WeaponComponentMap.Add(TPair<EWeaponComponentType, USceneComponent*>(Pair.Key,ParryComponent));
			}
			else
			{
				NewComponent.Key = Pair.Key;
				NewComponent.Value = false;
			}
		}
		else if (Pair.Key == EWeaponComponentType::Execute)
		{
			// 待后续补充
			NewComponent.Key = Pair.Key;
			NewComponent.Value = false;
		}
		else if (Pair.Key == EWeaponComponentType::BackStab)
		{
			// 待后续补充
			NewComponent.Key = Pair.Key;
			NewComponent.Value = false;
		}
		else
		{continue; }

		// 更新信息
		WeaponLoadComponentInfoMap.Add(NewComponent);
	}

	return false;
}

bool ASL_WeaponComponent::CanExecute(AActor* MasterActor, float AllowedExecuteDistance, float AllowdBackStabRange)
{
	// 完成距离,角度,是否装载组件进行判断
	return false;
}

bool ASL_WeaponComponent::CanBackStab(AActor* MasterActor, float AllowedBackStabDistance, float AllowdBackStabRange)
{
	// 完成距离,角度,是否装载组件进行判断
	return false;
}
