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
	bIsParryWindowActive = false;
	//APCostMap.Reset();
	//APCostMap.Add(TTuple<EAttackType, float>{EAttackType::Normal_Combo_Phase_1,-20.0f});
	//APCostMap.Add(TTuple<EAttackType, float>{EAttackType::Normal_Combo_Phase_2, -20.0f});
	//APCostMap.Add(TTuple<EAttackType, float>{EAttackType::Normal_Combo_Phase_3, -20.0f});
	//APCostMap.Add(TTuple<EAttackType, float>{EAttackType::Skill_Combo_Phase_1, -40.0f});

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

	// 临时测试添加盾牌战技动画
	Montage = LoadObject<UAnimMontage>(
		nullptr,
		TEXT("/Game/SoulLikeDemo/Anim/AM_CombatSkill_Sheild.AM_CombatSkill_Sheild")
		);
	if (Montage)
	{
		CombatSkillMontage = Montage;
	}

	// 临时测试添加背刺动画
	Montage = LoadObject<UAnimMontage>(
		nullptr,
		TEXT("/Game/SoulLikeDemo/Anim/AM_BackStab_Sword.AM_BackStab_Sword")
		);
	if (Montage)
	{
		BackStabbMontage = Montage;
	}

	// 临时测试添加处决动画
	Montage = LoadObject<UAnimMontage>(
		nullptr,
		TEXT("/Game/SoulLikeDemo/Anim/AM_Execute_Sword.AM_Execute_Sword")
		);
	if (Montage)
	{
		ExecutionMontage = Montage;
	}
}

FDamageData AWeaponBase::GetDamageData_Implementation() const
{
	return FDamageData();
}



void AWeaponBase::PerformAttack()
{
	// 根据当前播放状态进行判断是否允许切换
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

void AWeaponBase::PerformCombatSkill()
{
	// 根据通知来确定播放的montage片段
	if (AttackSection_NS != nullptr && EnableComboContinue == true)
	{

		PlayCombatSkillMontage(AttackSection_NS->JumpSectionName);
	}
	else
	{
		PlayCombatSkillMontage(FName(""));
	}
}

void AWeaponBase::PerformBackstab()
{
	// 直接播放蒙太奇动画，后续补充状态判断
	UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(BackStabbMontage);
	}
}

void AWeaponBase::PerformExecute()
{
	// 直接播放蒙太奇动画，后续补充状态判断
	UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(ExecutionMontage);
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

void AWeaponBase::PlayCombatSkillMontage(FName MontageSectionName)
{
	// 检测是否有效
	if (OwningCharacter == nullptr || OwningCharacter->GetMesh()->GetAnimInstance() == nullptr)
		return;

	UAnimInstance* AnimInstance = OwningCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance->Montage_IsActive(CombatSkillMontage) &&
		MontageSectionName != FName(""))
	{
		AnimInstance->Montage_JumpToSection(MontageSectionName);
	}
	else
	{
		AnimInstance->Montage_Play(CombatSkillMontage);
	}
}

void AWeaponBase::OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}

float AWeaponBase::GetStaminaCost(EAttackType AttackType)
{
	return (*APCostMap.Find(AttackType));
}

void AWeaponBase::EnableAttackCollisonCheck()
{
	// 根据配置数据初始化碰撞盒大小和配置
	CollisonBox->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnAttackOverlapBegin);
	CollisonBox->OnComponentEndOverlap.AddDynamic(this, &AWeaponBase::OnAttackOverlapEnd);
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
	// 根据配置数据初始化碰撞盒大小和配置
	CollisonBox->OnComponentBeginOverlap.RemoveDynamic(this, &AWeaponBase::OnAttackOverlapBegin);
	CollisonBox->OnComponentEndOverlap.RemoveDynamic(this, &AWeaponBase::OnAttackOverlapEnd);
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

void AWeaponBase::EnableParryCollisonCheck()
{
	// 根据配置数据初始化碰撞盒大小和配置
	CollisonBox->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnParryOverlapBegin);
	CollisonBox->OnComponentEndOverlap.AddDynamic(this, &AWeaponBase::OnParryOverlapEnd);
	// 设置碰撞
	CollisonBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisonBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisonBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisonBox->SetGenerateOverlapEvents(true);
	// 开启定时器
	GetWorld()->GetTimerManager().SetTimer(ParryWindowTimer, this,
		&AWeaponBase::ApplyParryToOverlappingActors,
		ParryInterval, true);
	// 重置已命中目标记录
	AlreadyParryActors.Reset();
	UE_LOG(LogTemp, Display, TEXT("EnableParryCollisonCheck"));
}

void AWeaponBase::DisableParryCollisonCheck()
{
	// 根据配置数据初始化碰撞盒大小和配置
	CollisonBox->OnComponentBeginOverlap.RemoveDynamic(this, &AWeaponBase::OnParryOverlapBegin);
	CollisonBox->OnComponentEndOverlap.RemoveDynamic(this, &AWeaponBase::OnParryOverlapEnd);
	// 关闭碰撞检测
	CollisonBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisonBox->SetGenerateOverlapEvents(false);
	// 关闭定时器
	if (ParryWindowTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ParryWindowTimer);
	}
	// 清空已名字目标记录
	AlreadyParryActors.Reset();
	UE_LOG(LogTemp, Display, TEXT("DisableParryCollisonCheck"));
}

void AWeaponBase::ActivateParryWindow(float Duration)
{
	// 根据配置数据初始化碰撞盒大小和配置

	bIsParryWindowActive = true;

	// 设置定时器自动关闭弹反窗口,当弹反成功时需要通过定时器关闭
	GetWorld()->GetTimerManager().SetTimer(
		ParryWindowTimer,
		this,
		&AWeaponBase::DeactivateParryWindow,
		Duration,
		false
	);
}

void AWeaponBase::DeactivateParryWindow()
{
	bIsParryWindowActive = false;
}

bool AWeaponBase::IsParryWindowActive()
{
	return bIsParryWindowActive;
}

void AWeaponBase::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor && OtherActor != this)
	{
		AttackOverlappingActors.AddUnique(OtherActor);
	}

}

void AWeaponBase::OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AttackOverlappingActors.Remove(OtherActor);
	}
}

void AWeaponBase::OnParryOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if (OtherActor && OtherActor != this)
	{
		ParryOverlappingActors.AddUnique(OtherActor);
	}

}

void AWeaponBase::OnParryOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		ParryOverlappingActors.Remove(OtherActor);
	}
}

void AWeaponBase::ApplyDamageToOverlappingActors()
{
	for (AActor* Actor : AttackOverlappingActors)
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

				Enemy->CombatComponent->HandleDamage(DamageEventData);
				AlreadyHitActors.Add(Enemy);
			}
		}
	}
}

void AWeaponBase::ApplyParryToOverlappingActors()
{
	for (AActor* Actor : ParryOverlappingActors)
	{
		if (Actor)
		{
			// 检查是否是特定类
			ASoulLikeCharacter* Enemy = Cast<ASoulLikeCharacter>(Actor);
			if (Enemy && OwningCharacter != Enemy && !AlreadyParryActors.Contains(Enemy))
			{
				Enemy->CombatComponent->HandleParry();
				AlreadyParryActors.Add(Enemy);
			}
		}
	}
}

void AWeaponBase::SetComboContinueState(bool Enable)
{
	EnableComboContinue = Enable;
}

void AWeaponBase::SetJumpSection_NS(USL_Attack_JumpSection_NS* NS)
{
	if(NS != nullptr)
		AttackSection_NS = NS;
}
