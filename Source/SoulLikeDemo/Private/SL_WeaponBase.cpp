#include "SL_WeaponBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"
#include "WeaponMeleeAttackComponent.h"
#include "WeaponParryComponent.h"

ASL_WeaponBase::ASL_WeaponBase()
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

void ASL_WeaponBase::Attack(AActor* OwnerActor)
{
	PlayWeaponMentage(OwnerActor,EWeaponMontageType::EWeaponMontag_Attack,TEXT(""));
}

void ASL_WeaponBase::Defence(AActor* OwnerActor)
{
	PlayWeaponMentage(OwnerActor, EWeaponMontageType::EWeaponMontag_Defence, TEXT(""));
}

void ASL_WeaponBase::ComboSkill(AActor* OwnerActor)
{
	PlayWeaponMentage(OwnerActor, EWeaponMontageType::EWeaponMontag_ComboSkill, TEXT(""));
}

void ASL_WeaponBase::Execute(AActor* OwnerActor)
{
	PlayWeaponMentage(OwnerActor, EWeaponMontageType::EWeaponMontag_Execute, TEXT(""));
}

void ASL_WeaponBase::BackStab(AActor* OwnerActor)
{
	PlayWeaponMentage(OwnerActor, EWeaponMontageType::EWeaponMontag_BackStab, TEXT(""));
}

void ASL_WeaponBase::InitWeaponInfo(const FWeaponData& WeaponInfo)
{
	if(OnwerActor == nullptr){return;}
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

void ASL_WeaponBase::ActiveWeapon()
{
	// 可视

}

void ASL_WeaponBase::InActiveWeapon()
{
	// 不可视
}

void ASL_WeaponBase::UpdateWeaponEquipState(EWeaponEquipState CurrentState)
{
	WeaponEquipInfo = CurrentState;
}

bool ASL_WeaponBase::IsLoadExecuteMod()
{
	return WeaponLoadComponentInfoMap.Find(EWeaponComponentType::Execute);
}

bool ASL_WeaponBase::IsLoadBackStabMod()
{
	return WeaponLoadComponentInfoMap.Find(EWeaponComponentType::BackStab);
}

void ASL_WeaponBase::LoadWeaponMeshAsync(const FString WeaponMeshName)
{
	if (WeaponMeshName == "") { return; }
	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftMeshReference = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(*WeaponMeshName));
	Streamable.RequestAsyncLoad(
		SoftMeshReference.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ASL_WeaponBase::OnLoadedWeaponMesh)
	);
}

void ASL_WeaponBase::OnLoadedWeaponMesh()
{
	if (USkeletalMesh* Mesh = SoftMeshReference.Get()) {
		SkeletalWeaponMesh->SetSkeletalMesh(Mesh);
	}
}

void ASL_WeaponBase::LoadWeaponMentageAsync(EWeaponMontageType MentageType, const FString MentagePath)
{
	if (MentagePath == "") { return; }
	// 资源异步加载
	/*FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftMentageRefrence = FSoftObjectPath(*MentagePath);
	Streamable.RequestAsyncLoad(
		SoftMentageRefrence.ToSoftObjectPath()
	);*/
	// 资源同步加载
	UAnimMontage* Montage = LoadObject<UAnimMontage>(
		nullptr,
		*MentagePath
		);
	if (Montage)
	{
		WeaponMentageMap.FindRef(MentageType) = Montage;
	}

}

void ASL_WeaponBase::LoadWeaponAnimInstanceAsync(const FString WeapinAnimName)
{
	if (WeapinAnimName == "") { return; }
	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	FSoftObjectPath AnimPathStr(WeapinAnimName + "_C");
	SoftWeaponAnimInstanceReference = AnimPathStr;
	Streamable.RequestAsyncLoad(
		SoftWeaponAnimInstanceReference.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &ASL_WeaponBase::OnLoadedWeaponAnimInstance)
	);
}

void ASL_WeaponBase::OnLoadedWeaponAnimInstance()
{
	if (SoftWeaponAnimInstanceReference.Get() != nullptr)
	{
		SkeletalWeaponMesh->SetAnimInstanceClass(SoftWeaponAnimInstanceReference.Get());
	}
}

bool ASL_WeaponBase::LoadWeaponComponents(const TMap<EWeaponComponentType, bool>& pWeaponComponentInfo)
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

void ASL_WeaponBase::PlayWeaponMentage(AActor* OwnerActor, EWeaponMontageType MentageType, FName MentageSectionName)
{
	if (OwnerActor == nullptr){return;}

	ACharacter* OwningCharacter = Cast<ACharacter>(OwnerActor);
	if (OwningCharacter == nullptr || OwningCharacter->GetMesh()->GetAnimInstance() == nullptr)
	{
		UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
		UAnimMontage* NeedPlayMentage = WeaponMentageMap.FindRef(MentageType);
		if(NeedPlayMentage == nullptr || AnimInstance == nullptr){return;}

		if (AnimInstance->Montage_IsActive(NeedPlayMentage) &&
			MentageSectionName != FName(""))
		{
			AnimInstance->Montage_JumpToSection(MentageSectionName);
		}
		else
		{
			AnimInstance->Montage_Play(NeedPlayMentage);
		}
	}
}
