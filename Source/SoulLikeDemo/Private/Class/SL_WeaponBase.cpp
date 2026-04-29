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
#include <Engine/PackageMapClient.h>
#include <Net/UnrealNetwork.h>

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

	// 武器中央协调组件
	WeaponComboCoordinatorComp = CreateDefaultSubobject<UWeaponComboCoordinatorComponent>(TEXT("ComboCoordinator"));

	/************************************************************************/
	/*                              变量初始化                                        */
	/************************************************************************/
	WeaponID = 0;
	SoftMeshReference = nullptr;
	CollisionBoxSize = FVector(0.0f,0.0f,0.0f);
	SoftWeaponAnimInstanceReference = nullptr;
	WeaponComponentMap.Reset();
	WeaponLoadComponentInfoMap.Reset();
	WeaponEquipInfo = EWeaponEquipState::No_Equip;

	// 开启网络复制
	bReplicates = true;
}

void ASL_WeaponBase::WeaponAnimNotifyResponse(int NotifyType)
{
	// 参数检查
	RETURN_IF_TRUE(NotifyType <= int(EWeaponAnimNotifyType::EWeaponAnimNotify_Min) || NotifyType >= int(EWeaponAnimNotifyType::EWeaponAnimNotify_Max));
	switch (EWeaponAnimNotifyType(NotifyType))
	{
	case EWeaponAnimNotifyType::EWeaponAnimNotify_Active_NormalComboWindow:
	{
		// 检查组件是否有效
		if(WeaponComboCoordinatorComp)
		{
			WeaponComboCoordinatorComp->ActiveComboWindowInputState(EWeaponModeTyoe::WEAPONMODE_Attack);
		}
	}; break;
	case EWeaponAnimNotifyType::EWeaponAnimNotify_InActive_NormalComboWindow:
	{
		// 检查组件是否有效
		if (WeaponComboCoordinatorComp)
		{
			WeaponComboCoordinatorComp->InActiveComboWindowInputState(EWeaponModeTyoe::WEAPONMODE_Attack);
		}
	}; break;
	case EWeaponAnimNotifyType::EWeaponAnimNotify_Active_SkillComboWindow:
	{
		// 检查组件是否有效
		if (WeaponComboCoordinatorComp)
		{
			WeaponComboCoordinatorComp->ActiveComboWindowInputState(EWeaponModeTyoe::WEAPONMODE_ComboSkill);
		}
	}; break;
	case EWeaponAnimNotifyType::EWeaponAnimNotify_InActive_SkillComboWindow:
	{
		if (WeaponComboCoordinatorComp)
		{
			WeaponComboCoordinatorComp->ActiveComboWindowInputState(EWeaponModeTyoe::WEAPONMODE_ComboSkill);
		}
	}; break;
	case EWeaponAnimNotifyType::EWeaponAnimNotify_EnableCollision_Melee:
	{
		// 检查组件是否有效
		if (WeaponComponentMap.Find(EWeaponComponentType::MeleeAttack) != nullptr)
		{
			UWeaponMeleeAttackComponent* Comp = Cast<UWeaponMeleeAttackComponent>(WeaponComponentMap.FindRef(EWeaponComponentType::MeleeAttack));
			if (Comp != nullptr)
			{
				Comp->EnableCollisionBoxCheck();
			}
		}
	}; break;
	case EWeaponAnimNotifyType::EWeaponAnimNotify_DisableCollision_Melee:
	{
		// 检查组件是否有效
		if (WeaponComponentMap.Find(EWeaponComponentType::MeleeAttack) != nullptr)
		{
			UWeaponMeleeAttackComponent* Comp = Cast<UWeaponMeleeAttackComponent>(WeaponComponentMap.FindRef(EWeaponComponentType::MeleeAttack));
			if (Comp != nullptr)
			{
				Comp->DisableCollisionBoxCheck();
			}
		}
	}; break;
	case EWeaponAnimNotifyType::EWeaponAnimNotify_ActiveParryWindow_Melee:
	{
		// 检查组件是否有效
		if (WeaponComponentMap.Find(EWeaponComponentType::MeleeAttack) != nullptr)
		{
			UWeaponMeleeAttackComponent* Comp = Cast<UWeaponMeleeAttackComponent>(WeaponComponentMap.FindRef(EWeaponComponentType::MeleeAttack));
			if (Comp != nullptr)
			{
				Comp->EnableParryWindowCheck(0.0f);
			}
		}
	}; break;
	case EWeaponAnimNotifyType::EWeaponAnimNotify_InActiveParryWindow_Melee:
	{
		// 检查组件是否有效
		if (WeaponComponentMap.Find(EWeaponComponentType::MeleeAttack) != nullptr)
		{
			UWeaponMeleeAttackComponent* Comp = Cast<UWeaponMeleeAttackComponent>(WeaponComponentMap.FindRef(EWeaponComponentType::MeleeAttack));
			if (Comp != nullptr)
			{
				Comp->DisableParryWindowCheck();
			}
		}
	}; break;
	case EWeaponAnimNotifyType::EWeaponAnimNotify_EnableCollision_ComboSkil_Parry:
	{
		// 检查组件是否有效
		if (WeaponComponentMap.Find(EWeaponComponentType::Parry) != nullptr)
		{
			UWeaponParryComponent* Comp = Cast<UWeaponParryComponent>(WeaponComponentMap.FindRef(EWeaponComponentType::Parry));
			if (Comp != nullptr)
			{
				Comp->EnableCollisionBoxCheck();
			}
		}
	}; break;
	case EWeaponAnimNotifyType::EWeaponAnimNotify_DisableCollision_ComboSkil_Parry:
	{
		// 检查组件是否有效
		if (WeaponComponentMap.Find(EWeaponComponentType::Parry) != nullptr)
		{
			UWeaponParryComponent* Comp = Cast<UWeaponParryComponent>(WeaponComponentMap.FindRef(EWeaponComponentType::Parry));
			if (Comp != nullptr)
			{
				Comp->DisableCollisionBoxCheck();
			}
		}
	}; break;
	default:break;
	}
}

void ASL_WeaponBase::Attack(AActor* OwnerActor)
{
	// 加入连段逻辑
	// PlayWeaponMentage(OwnerActor, EWeaponModeTyoe::WEAPONMODE_Attack,TEXT("Default"));
}

void ASL_WeaponBase::Defence(AActor* OwnerActor)
{
	// PlayWeaponMentage(OwnerActor, EWeaponModeTyoe::WEAPONMODE_Defence, TEXT("Default"));
}

void ASL_WeaponBase::ComboSkill(AActor* OwnerActor)
{
	// 加入连段逻辑
	// PlayWeaponMentage(OwnerActor, EWeaponModeTyoe::WEAPONMODE_ComboSkill, TEXT("Default"));
}

void ASL_WeaponBase::Execute(AActor* OwnerActor)
{
	// PlayWeaponMentage(OwnerActor, EWeaponModeTyoe::WEAPONMODE_Execute, TEXT("Default"));
}

void ASL_WeaponBase::BackStab(AActor* OwnerActor)
{
	// PlayWeaponMentage(OwnerActor, EWeaponModeTyoe::WEAPONMODE_BackStab, TEXT("Default"));
}

void ASL_WeaponBase::InitWeaponInfo(const FWeaponData& WeaponInfo,AActor* OwnerActor)
{
	// 初始化持有者信息和插槽信息
	Owning = OwnerActor;
	WeaponConfig = WeaponInfo;
	WeaponOnwerSocketName = WeaponInfo.SocketName;
	// 加载武器模型
	LoadWeaponMeshAsync(WeaponInfo.Mesh);
	CollisionBoxSize = WeaponInfo.WeaponCollisionBoxSize;
	// 加载武器动作蓝图
	LoadWeaponAnimInstanceAsync(WeaponInfo.AnimClass);

	// 加载武器蒙太奇
	LoadWeaponMentageAsync(EWeaponModeTyoe::WEAPONMODE_Attack, WeaponInfo.AttackMentageName);
	// 加载武器蒙太奇
	LoadWeaponMentageAsync(EWeaponModeTyoe::WEAPONMODE_ComboSkill, WeaponInfo.ComboSkillMentageName);
	// 加载武器蒙太奇
	LoadWeaponMentageAsync(EWeaponModeTyoe::WEAPONMODE_Execute, WeaponInfo.ExecuteMentageName);
	// 加载武器蒙太奇
	LoadWeaponMentageAsync(EWeaponModeTyoe::WEAPONMODE_BackStab, WeaponInfo.BackStabMentageName);
	// 加载武器模组
	LoadWeaponComponents(WeaponInfo.NeedLoadComponentInfoMap);

	// 初始化武器中央管理组件
	WeaponComboCoordinatorComp->InitComboCoordinatorComponet(WeaponInfo.ComboCoordinatorInfoMap);

	UE_LOG(LogTemp, Warning, TEXT("=== InitWeaponInfo ==="));
	UE_LOG(LogTemp, Warning, TEXT("Weapon: %s OwnerActorNetworkGUID: %s MulticastFunction called on %s"), *GetName(), *GetNetworkGUIDString(this), HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
	UE_LOG(LogTemp, Warning, TEXT("Weapon Owner: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Param OwnerActor: %s"), OwnerActor ? *OwnerActor->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("WeaponMentageMap.Num: %d"), WeaponMentageMap.Num());
}

void ASL_WeaponBase::ActiveWeapon()
{
	// 可视
	SetActorHiddenInGame(false);
}

void ASL_WeaponBase::InActiveWeapon()
{
	// 不可视
	SetActorHiddenInGame(true);
}

void ASL_WeaponBase::UpdateWeaponEquipState(EWeaponEquipState CurrentState)
{
	WeaponEquipInfo = CurrentState;
}

bool ASL_WeaponBase::IsLoadExecuteMod()
{
	return  bool(*WeaponLoadComponentInfoMap.Find(EWeaponComponentType::Execute));
}

bool ASL_WeaponBase::IsLoadBackStabMod()
{
	return bool(*WeaponLoadComponentInfoMap.Find(EWeaponComponentType::BackStab));
}
bool ASL_WeaponBase::PerformWeaponAction(EWeaponModeTyoe ActionType, AActor* OwnerActor)
{
	if (!OwnerActor) return false;

	// 客户端：请求服务器
	if (OwnerActor->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_PerformWeaponAction(ActionType, OwnerActor);
		return true;
	}

	// 服务器：直接执行（Authority 但没有 AutonomousProxy 时，即纯服务器或 AI）
	if (OwnerActor->HasAuthority())
	{
		ExecuteWeaponActionInternal(ActionType, OwnerActor);
	}

	return true;
}

float ASL_WeaponBase::GetWeaponBaseDamage()
{
	return 20.f;
}

// 新增：实际执行逻辑的函数（不包含 RPC 调用）
void ASL_WeaponBase::ExecuteWeaponActionInternal(EWeaponModeTyoe ActionType, AActor* OwnerActor)
{
	switch (ActionType)
	{
	case EWeaponModeTyoe::WEAPONMODE_Attack:
		Attack(OwnerActor);
		break;
	case EWeaponModeTyoe::WEAPONMODE_Defence:
		Defence(OwnerActor);
		break;
	case EWeaponModeTyoe::WEAPONMODE_ComboSkill:
		ComboSkill(OwnerActor);
		break;
	case EWeaponModeTyoe::WEAPONMODE_Execute:
		Execute(OwnerActor);
		break;
	case EWeaponModeTyoe::WEAPONMODE_BackStab:
		BackStab(OwnerActor);
		break;
	default:
		break;
	}

	// 广播动画
	Multicast_PlayWeaponMentage(OwnerActor, ActionType, TEXT("Default"));
}

void ASL_WeaponBase::Server_PerformWeaponAction_Implementation(EWeaponModeTyoe ActionType, AActor* OwnerActor)
{
	if (!OwnerActor) return;

	// 可以在这里添加反作弊验证

	ExecuteWeaponActionInternal(ActionType, OwnerActor);
}

void ASL_WeaponBase::Multicast_PlayWeaponMentage_Implementation(AActor* OwnerActor, EWeaponModeTyoe MentageType, FName MentageSectionName)
{
	UE_LOG(LogTemp, Warning, TEXT("=== Multicast Received ==="));
	UE_LOG(LogTemp, Warning, TEXT("Weapon: %s OwnerActorNetworkGUID: %s MulticastFunction called on %s"), *GetName(), *GetNetworkGUIDString(this), HasAuthority() ? TEXT("SERVER") : TEXT("CLIENT"));
	UE_LOG(LogTemp, Warning, TEXT("Weapon Owner: %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("Param OwnerActor: %s"), OwnerActor ? *OwnerActor->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("WeaponMentageMap.Num: %d"), WeaponMentageMap.Num());

	PlayWeaponMentage(OwnerActor, MentageType, MentageSectionName);
}

void ASL_WeaponBase::OnRep_WeaponConfig()
{
	if (!HasAuthority() && WeaponConfig.Mesh != "" && WeaponMentageMap.Num() == 0)
	{
		InitWeaponInfo(WeaponConfig, Owning.Get());
	}
}


void ASL_WeaponBase::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASL_WeaponBase, WeaponConfig);
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
	if (Owning != nullptr && WeaponOnwerSocketName != "")
	{
		if (ACharacter* CharacterOwner = Cast<ACharacter>(Owning))
		{
			//将武器绑定到指定虚拟骨骼
			if (CharacterOwner->GetMesh()->DoesSocketExist(FName(*WeaponOnwerSocketName)))
			{
				this->AttachToComponent(
					CharacterOwner->GetMesh(),
					FAttachmentTransformRules::SnapToTargetNotIncludingScale, // 保持相对变换
					FName(*WeaponOnwerSocketName) // Socket 名称
				);
			}
		}
	}
}

void ASL_WeaponBase::LoadWeaponMentageAsync(EWeaponModeTyoe MentageType, const FString MentagePath)
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
		WeaponMentageMap.FindOrAdd(MentageType) = Montage;
	}
	else
	{
		return;
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
			UWeaponMeleeAttackComponent* MeleeComponent = NewObject<UWeaponMeleeAttackComponent>(this,"MeleeCmp");
			if (MeleeComponent)
			{
				MeleeComponent->ReregisterComponent();
				MeleeComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				MeleeComponent->InitalizeWeaponComponent(this, CollisionBoxSize);
				NewComponent.Key = Pair.Key;
				NewComponent.Value = true;
				WeaponComponentMap.Add(TPair<EWeaponComponentType, USceneComponent*>(Pair.Key, MeleeComponent));
			}
			else
			{
				NewComponent.Key = Pair.Key;
				NewComponent.Value = false;
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
			// 目前处决模组无需单独组件
			NewComponent.Key = Pair.Key;
			NewComponent.Value = true;
		}
		else if (Pair.Key == EWeaponComponentType::BackStab)
		{
			// 目前处决模组无需单独组件
			NewComponent.Key = Pair.Key;
			NewComponent.Value = true;
		}
		else
		{continue; }

		// 更新信息
		WeaponLoadComponentInfoMap.Add(NewComponent);
	}

	return false;
}

void ASL_WeaponBase::PlayWeaponMentage(AActor* OwnerActor, EWeaponModeTyoe MentageType, FName MentageSectionName)
{
	if (OwnerActor == nullptr){return;}

	ACharacter* OwningCharacter = Cast<ACharacter>(OwnerActor);
	if (OwningCharacter != nullptr && OwningCharacter->GetMesh()->GetAnimInstance() != nullptr)
	{
		UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
		UAnimMontage* NeedPlayMentage = WeaponMentageMap.FindRef(MentageType);
		if(NeedPlayMentage == nullptr || AnimInstance == nullptr){return;}

		if (AnimInstance->Montage_IsActive(NeedPlayMentage) &&
			NeedPlayMentage->IsValidSectionName(MentageSectionName))
		{
			AnimInstance->Montage_JumpToSection(MentageSectionName);
		}
		else if (NeedPlayMentage->IsValidSectionName(MentageSectionName))
		{
			AnimInstance->Montage_Play(NeedPlayMentage);
			AnimInstance->Montage_JumpToSection(MentageSectionName, NeedPlayMentage);
		}
		else
		{
			// 不展示动画
			if (MentageType == EWeaponModeTyoe::WEAPONMODE_Attack && MentageSectionName == "Default")
			{
				AnimInstance->Montage_Play(NeedPlayMentage);
			}
		}
	}
}

FString ASL_WeaponBase::GetNetworkGUIDString(AActor* InActor)
{
	if (InActor == nullptr) { return TEXT("InActor InValid"); }
	UNetDriver* NetDriver = InActor->GetWorld()->GetNetDriver();
	if (NetDriver == nullptr) { return TEXT("InActor NetDriver"); }
	if (InActor->GetLocalRole() == ROLE_Authority)
	{
		// 服务器端：遍历 ClientConnections
		for (UNetConnection* NetConnection : NetDriver->ClientConnections)
		{
			// 找到拥有此 Actor 的那个连接
			if (UPackageMapClient* PackageMapClient = Cast<UPackageMapClient>(NetConnection->PackageMap))
			{
				FNetworkGUID NetGUID = PackageMapClient->GetNetGUIDFromObject(InActor);
				if (NetGUID.IsValid())
				{
					return NetGUID.ToString();
				}
			}
		}
	}
	else
	{
		// 客户端端：使用 ServerConnection
		UPackageMapClient* PackageMap = Cast<UPackageMapClient>(NetDriver->ServerConnection->PackageMap);

		FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(InActor);
		if (NetGUID.IsValid())
		{
			return NetGUID.ToString();
		}

	}

	return TEXT("Invalid GUID");
}
