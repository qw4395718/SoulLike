// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"

AWeaponBase::AWeaponBase()
{
	//是否开启tick
	PrimaryActorTick.bCanEverTick = false;

	/************************************************************************/
	/*                              组件初始化                                        */
	/************************************************************************/
	// 场景组件root
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	// 碰撞盒-默认不开启检测
	CollisonBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisonBox"));
	CollisonBox->SetupAttachment(RootComponent);
	CollisonBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisonBox->SetGenerateOverlapEvents(false);

	// 骨骼网格体-
	SkeletalWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalWeaponMesh->SetupAttachment(RootComponent);
	// 静态网格体-
	StaticWeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticWeaponMesh->SetupAttachment(RootComponent);

	/************************************************************************/
	/*                              变量初始化                                        */
	/************************************************************************/

	Initialize();
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	// 获取持有者
	if (GetOwner() && Cast<ASoulLikeCharacter>(GetOwner()))
	{
		OwningCharacter = Cast<ASoulLikeCharacter>(GetOwner());
	}

	// 根据配置数据初始化碰撞盒大小和配置
	CollisonBox->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnOverlapBegin);
	CollisonBox->OnComponentEndOverlap.AddDynamic(this, &AWeaponBase::OnOverlapEnd);
}

void AWeaponBase::Initialize()
{
	// 变量初始化
	bEnableCapsuleCheck = false;
	IsStaticMesh = true;
	WeaponData.WeaponCollisonBoxLength = 100.0f;
	WeaponData.WeaponCollisonBoxWidth = 5.0f;
	WeaponData.WeaponCollisonBoxHeight = 5.0f;
	EnableComboContinue = false;

	// 资源初始化
	USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(
		nullptr, // Outer对象（通常为null或GetTransientPackage）
		TEXT("/Game/InfinityBladeWeapons/Weapons/Blade/Swords/Blade_BlackKnight/SK_Blade_BlackKnight.SK_Blade_BlackKnight") // 资源路径
		);
	if (Mesh)
	{
		SkeletalWeaponMesh->SetSkeletalMesh(Mesh);
	}

	// 加载武器动画资源
	UAnimMontage* Montage = LoadObject<UAnimMontage>(
		nullptr,
		TEXT("/Game/SoulLikeDemo/Anim/AM_Attack_Sword.AM_Attack_Sword")
		);
	if (Montage)
	{
		AttackMontage = Montage;
	}

}

FDamageData AWeaponBase::GetDamageData_Implementation() const
{
	return FDamageData();
}



void AWeaponBase::PerformAttack()
{
	// 根据通知来确定播放的montage片段
	if (AttackSection_NS != nullptr && EnableComboContinue == true)
	{

		PlayAttackMontage(AttackSection_NS->JumpSectionName);
	}
	else
	{
		PlayAttackMontage(FName(""));
	}

}

void AWeaponBase::PlayAttackMontage(FName MontageSectionName)
{
	// 检测是否有效
	if (OwningCharacter == nullptr || OwningCharacter->GetMesh()->GetAnimInstance() == nullptr)
		return;

	UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance->Montage_IsActive(AttackMontage) &&
		MontageSectionName != FName(""))
	{
		AnimInstance->Montage_JumpToSection(MontageSectionName);
	}
	else
	{
		AnimInstance->Montage_Play(AttackMontage);
	}

	
	
}

void AWeaponBase::OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}

float AWeaponBase::GetStaminaCost(EAttackType AttackType)
{
	return 0.0f;
}

void AWeaponBase::EnableAttackCollisonCheck()
{
	// 设置碰撞
	CollisonBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisonBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisonBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisonBox->SetGenerateOverlapEvents(true);
	// 开启定时器
	GetWorld()->GetTimerManager().SetTimer(DamageTimerHandle, this,
		&AWeaponBase::ApplyDamageToOverlappingActors,
		DamageInterval, true);
	// 重置已命中目标记录
	AlreadyHitActors.Reset();
}

void AWeaponBase::DisableAttackCollisonCheck()
{
	// 关闭碰撞检测
	CollisonBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisonBox->SetGenerateOverlapEvents(false);
	// 关闭定时器
	if (DamageTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(DamageTimerHandle);
	}
	// 清空已名字目标记录
	AlreadyHitActors.Reset();
}

void AWeaponBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor && OtherActor != this)
	{
		OverlappingActors.AddUnique(OtherActor);
	}

}

void AWeaponBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		OverlappingActors.Remove(OtherActor);
	}
}

void AWeaponBase::ApplyDamageToOverlappingActors()
{
	for (AActor* Actor : OverlappingActors)
	{
		if (Actor)
		{
			// 检查是否是特定类
			ASoulLikeCharacter* Enemy = Cast<ASoulLikeCharacter>(Actor);
			if (Enemy && OwningCharacter != Enemy && !AlreadyHitActors.Contains(Enemy))
			{
				// 处理敌人命中逻辑
				FDamageEventData DamageEventData;
				DamageEventData.BaseDamage = WeaponData.BaseDamage;
				//DamageEventData.HitLocation = HitResult.Location;
				//DamageEventData.HitNormal = HitResult.Normal;
				DamageEventData.bIsCriticalHit = false;
				DamageEventData.AttackType = EDamageType::SLASH;
				DamageEventData.DamageCauser = Enemy;

				OwningCharacter->CombatComponent->HandleDamage(DamageEventData);
				AlreadyHitActors.Add(Enemy);
			}
		}
	}
}

void AWeaponBase::SetComboContinueState(bool Enable)
{
	EnableComboContinue = Enable;
}

void AWeaponBase::SetJumpSection_NS(USoulLike_JumpSection_NS* NS)
{
	if(NS != nullptr)
		AttackSection_NS = NS;
}
