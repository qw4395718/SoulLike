// Private/Class/WeaponBase.cpp

#include "SL_WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DataTableManager.h"
#include "WeaponDataTable.h"
#include <SL_CharacterBase.h>
#include <AbilitySystemInterface.h>
#include <AbilitySystemComponent.h>
#include <SL_ComboManagerComponent.h>

ASL_WeaponBase::ASL_WeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// 场景组件root
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	// 碰撞盒
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionBox->SetGenerateOverlapEvents(false);

	// 骨骼网格体
	SkeletalWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalWeaponMesh->SetupAttachment(RootComponent);

	// 静态网格体
	StaticWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticWeaponMesh->SetupAttachment(RootComponent);
}

void ASL_WeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// 获取持有者
	if (GetOwner() && Cast<ASL_CharacterBase>(GetOwner()))
	{
		OwningCharacter = Cast<ASL_CharacterBase>(GetOwner());
	}
}

void ASL_WeaponBase::SetOwner(AActor* NewOwner)
{
	RETURN_IF_TRUE(NewOwner == nullptr);
	OwningCharacter = Cast<ASL_CharacterBase>(NewOwner);
}

// ==================== 初始化 ====================

void ASL_WeaponBase::InitializeWeaponWithID(int32 WeaponID)
{
	// 从DataTableManager获取武器数据表
	if (UDataTableManager* TableManager = UDataTableManager::Get(this))
	{
		if (UWeaponDataTable* WeaponTable = Cast<UWeaponDataTable>(TableManager->GetDataTable(EDataTableType::DT_WeaponDataInfo)))
		{
			FWeaponDataInfo OutData;
			if (WeaponTable->GetWeaponData(WeaponID, OutData))
			{
				InitializeFromDataRow(OutData);
			}
		}
	}

	if (!DamageEffectClass)
	{
		DamageEffectClass = LoadClass<UGameplayEffect>(
			nullptr,
			TEXT("/Game/SoulLikeDemo/GAS/GE/GE_Damage.GE_Damage_C")
			);
	}

	UE_LOG(LogTemp, Error, TEXT("ASL_WeaponBase::InitializeWeaponWithID - Failed to load weapon data for ID: %d"), WeaponID);
}

void ASL_WeaponBase::InitializeFromDataRow(const FWeaponDataInfo& InWeaponData)
{
	WeaponData = InWeaponData;

	// 加载资源
	LoadWeaponAssets();

	// 设置位置变换
	SetupOffset();

	// 设置碰撞盒
	SetupCollisionBox();

	// 绑定碰撞委托
	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASL_WeaponBase::OnCollisionOverlapBegin);
	CollisionBox->OnComponentEndOverlap.AddDynamic(this, &ASL_WeaponBase::OnCollisionOverlapEnd);

	UE_LOG(LogTemp, Log, TEXT("ASL_WeaponBase::InitializeFromDataRow - Weapon initialized: ID=%d, Name=%s"),
		WeaponData.WeaponID, *WeaponData.WeaponName.ToString());
}

// ==================== 资源加载 ====================

void ASL_WeaponBase::LoadWeaponAssets()
{
	RETURN_IF_TRUE(RootComponent == nullptr);
	// 优先使用SkeletalMesh
	if (!WeaponData.SkeletalMesh.IsNull())
	{
		USkeletalMesh* Mesh = WeaponData.SkeletalMesh.LoadSynchronous();
		if (Mesh)
		{
			SkeletalWeaponMesh->SetSkeletalMesh(Mesh);
			SkeletalWeaponMesh->SetVisibility(true);
			StaticWeaponMesh->SetVisibility(false);

			if (OwningCharacter)
			{
				//将武器绑定到指定虚拟骨骼
				if (OwningCharacter->GetMesh()->DoesSocketExist(WeaponData.SocketName))
				{
					this->AttachToComponent(
						OwningCharacter->GetMesh(),
						FAttachmentTransformRules::SnapToTargetNotIncludingScale, // 保持相对变换
						WeaponData.SocketName // Socket 名称
					);
					//RootComponent->SetRelativeLocation()
				}
			}

			bIsStaticMesh = false;
			// 检查是否有动画蓝图
			SetupAnimClass();

			return;
		}
	}

	// 回退到StaticMesh
	if (!WeaponData.StaticMesh.IsNull())
	{
		UStaticMesh* Mesh = WeaponData.StaticMesh.LoadSynchronous();
		if (Mesh)
		{
			StaticWeaponMesh->SetStaticMesh(Mesh);
			StaticWeaponMesh->SetVisibility(true);
			SkeletalWeaponMesh->SetVisibility(false);
			if (OwningCharacter)
			{
				//将武器绑定到指定虚拟骨骼
				if (OwningCharacter->GetMesh()->DoesSocketExist(WeaponData.SocketName))
				{
					this->AttachToComponent(
						OwningCharacter->GetMesh(),
						FAttachmentTransformRules::SnapToTargetNotIncludingScale, // 保持相对变换
						WeaponData.SocketName // Socket 名称
					);
					//RootComponent->SetRelativeLocation()
				}
			}
			bIsStaticMesh = true;
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ASL_WeaponBase::LoadWeaponAssets - No mesh found for weapon ID: %d"), WeaponData.WeaponID);
}

void ASL_WeaponBase::SetupAnimClass()
{
	if (!WeaponData.AnimClass.IsNull())
	{
		UAnimInstance* NewAnimClass = WeaponData.AnimClass.LoadSynchronous();
		if (NewAnimClass && bIsStaticMesh == false)
		{
			SkeletalWeaponMesh->SetAnimClass(NewAnimClass->GetClass());
		}
	}
}

void ASL_WeaponBase::SetupRotator()
{
	if (bIsStaticMesh == true)
	{
		StaticWeaponMesh->SetRelativeRotation(WeaponData.Rotator);
	}
}

void ASL_WeaponBase::SetupOffset()
{
	RETURN_IF_TRUE(RootComponent == nullptr);
	RootComponent->SetRelativeLocation(WeaponData.Offset);
}

// ==================== 碰撞盒设置 ====================

void ASL_WeaponBase::SetupCollisionBox()
{
	if (!CollisionBox) return;

	// 设置碰撞盒大小
	CollisionBox->SetBoxExtent(WeaponData.CollisionBoxSize);
}

// ==================== 攻击碰撞控制 ====================

void ASL_WeaponBase::EnableAttackCollision()
{
	if (!CollisionBox) return;

	// 设置碰撞通道
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetGenerateOverlapEvents(true);

	// 开启伤害检测定时器
	GetWorld()->GetTimerManager().SetTimer(DamageTimerHandle, this,
		&ASL_WeaponBase::ApplyDamageToOverlappingActors,
		DamageInterval, true);

	// 重置已命中目标记录
	AlreadyHitActors.Reset();

	UE_LOG(LogTemp, Verbose, TEXT("ASL_WeaponBase::EnableAttackCollision"));
}

void ASL_WeaponBase::DisableAttackCollision()
{
	if (!CollisionBox) return;

	// 关闭碰撞
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisionBox->SetGenerateOverlapEvents(false);

	// 关闭伤害检测定时器
	if (DamageTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);
	}

	// 清空已命中目标记录
	AlreadyHitActors.Reset();
	AttackOverlappingActors.Empty();

	UE_LOG(LogTemp, Verbose, TEXT("ASL_WeaponBase::DisableAttackCollision"));
}

// ==================== 弹反窗口控制 ====================

void ASL_WeaponBase::EnableParryWindow(float Duration)
{
	if (!CollisionBox) return;

	bIsParryWindowActive = true;

	// 设置碰撞通道
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBox->SetGenerateOverlapEvents(true);

	// 开启弹反检测定时器
	GetWorld()->GetTimerManager().SetTimer(ParryTimerHandle, this,
		&ASL_WeaponBase::ApplyParryToOverlappingActors,
		ParryInterval, true);

	// 重置已弹反目标记录
	AlreadyParryActors.Reset();

	// 如果传入了Duration，自动关闭
	if (Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			ParryTimerHandle,
			this,
			&ASL_WeaponBase::DisableParryWindow,
			Duration,
			false
		);
	}

	UE_LOG(LogTemp, Verbose, TEXT("ASL_WeaponBase::EnableParryWindow - Duration: %.2f"), Duration);
}

void ASL_WeaponBase::DisableParryWindow()
{
	bIsParryWindowActive = false;

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CollisionBox->SetGenerateOverlapEvents(false);
	}

	if (ParryTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ParryTimerHandle);
	}

	AlreadyParryActors.Reset();
	ParryOverlappingActors.Empty();

	UE_LOG(LogTemp, Verbose, TEXT("ASL_WeaponBase::DisableParryWindow"));
}

// ==================== 碰撞回调 ====================

void ASL_WeaponBase::OnCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor != OwningCharacter)
	{
		if (bIsParryWindowActive)
		{
			ParryOverlappingActors.AddUnique(OtherActor);
		}
		else
		{
			AttackOverlappingActors.AddUnique(OtherActor);
		}
	}
}

void ASL_WeaponBase::OnCollisionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AttackOverlappingActors.Remove(OtherActor);
		ParryOverlappingActors.Remove(OtherActor);
	}
}

// ==================== 伤害处理 ====================

void ASL_WeaponBase::ApplyDamageToOverlappingActors()
{
	TArray<AActor*> ActorsToDamage;

	// 获取当前重叠的Actor
	CollisionBox->GetOverlappingActors(ActorsToDamage);

	for (AActor* Actor : ActorsToDamage)
	{
		if (!Actor || Actor == OwningCharacter || AlreadyHitActors.Contains(Actor))
			continue;

		// 1. 检查目标是否具有GAS系统（新的SL_CharacterBase）
		IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Actor);
		if (!TargetASI) continue;

		UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
		if (!TargetASC) continue;

		// 2. 计算最终伤害
		float FinalDamage = CalculateFinalDamage(Actor);

		// 3. 创建GE上下文
		FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
		EffectContext.AddInstigator(OwningCharacter, this);

		FGameplayEffectSpecHandle SpecHandle = TargetASC->MakeOutgoingSpec(
			DamageEffectClass, 1.0f, EffectContext);

		if (SpecHandle.IsValid())
		{
			// 5. 通过SetByCaller设置伤害值
			FGameplayTag DamageTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.DamageNumber"), true);
			SpecHandle.Data->SetSetByCallerMagnitude(DamageTag, FinalDamage);

			// 6. 应用GE到目标
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

			UE_LOG(LogTemp, Warning, TEXT("WeaponBase: GAS Damage - %s dealt %f damage to %s"),
				*GetName(), FinalDamage, *Actor->GetName());
		}

		// 记录已命中
		AlreadyHitActors.Add(Actor);
	}

}

float ASL_WeaponBase::CalculateFinalDamage(AActor* InTargetActor) const
{
	// 1. 检查目标是否具有GAS系统（新的SL_CharacterBase）
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(InTargetActor);
	if (!TargetASI) return 0.0f;

	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (!TargetASC) return 0.0f;

	// 2. 计算最终伤害
	float FinalDamage = WeaponData.BaseDamage;

	// 从ComboManager获取当前连击倍率
	if (OwningCharacter)
	{
		// 方式1：如果OwningCharacter本身就实现了IAbilitySystemInterface
		if (IAbilitySystemInterface* OwnerASI = Cast<IAbilitySystemInterface>(OwningCharacter))
		{
			if (UAbilitySystemComponent* OwnerASC = OwnerASI->GetAbilitySystemComponent())
			{
				// 从Owner的Actor上找ComboManager组件
				USL_ComboManagerComponent* ComboMgr =
					OwningCharacter->FindComponentByClass<USL_ComboManagerComponent>();
				if (ComboMgr)
				{
					FinalDamage *= ComboMgr->GetCurrentComboDamageMultiplier();
				}
			}
		}
	}

	return FinalDamage;
}


void ASL_WeaponBase::ApplyParryToOverlappingActors()
{
	TArray<AActor*> ActorsToParry;
	CollisionBox->GetOverlappingActors(ActorsToParry);

	for (AActor* Actor : ActorsToParry)
	{
		if (!Actor || Actor == OwningCharacter || AlreadyParryActors.Contains(Actor))
			continue;

		// 检查是否可以被弹反
		if (ASoulLikeCharacter* Enemy = Cast<ASoulLikeCharacter>(Actor))
		{
			if (Enemy->CombatComponent)
			{
				Enemy->CombatComponent->HandleParry();
				AlreadyParryActors.Add(Actor);

				// 广播弹反事件
				OnWeaponParryDelegate.Broadcast(Actor);
			}
		}
	}
}