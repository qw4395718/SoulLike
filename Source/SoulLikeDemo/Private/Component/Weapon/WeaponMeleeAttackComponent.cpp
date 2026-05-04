// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponMeleeAttackComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SceneComponent.h"
#include "SoulLikeGameGlobal.h"
#include <AbilitySystemInterface.h>
#include <GameplayEffectTypes.h>
#include <AbilitySystemComponent.h>
#include <GameplayTagContainer.h>
#include <SL_ComboManagerComponent.h>

UWeaponMeleeAttackComponent::UWeaponMeleeAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	/************************************************************************/
	/*                              组件初始化                                        */
	/************************************************************************/
	// 碰撞盒-默认不开启检测
	CollisonBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisonBox"));
	CollisonBox->SetupAttachment(this);
	CollisonBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisonBox->SetGenerateOverlapEvents(false);

	/************************************************************************/
	/*                              变量初始化                                        */
	/************************************************************************/
	OnwerWeapon = nullptr;
	OnwerActor = nullptr;
	DamageInterval = 0.1f;
	bActiveParryWindow = false;
	DamageTimerHandle.Invalidate();
	ParryWindowTimer.Invalidate();
	AttackOverlappingActors.Reset();
	AlreadyHitActors.Reset();
}

void UWeaponMeleeAttackComponent::InitalizeWeaponComponent(AActor* Onwer, FVector CBSize)
{
	if (Onwer != nullptr && Onwer->GetOwner() != nullptr)
	{
		OnwerWeapon = Onwer;
		OnwerActor = Onwer->GetOwner();
		CollisionBoxSize = CBSize;
		CollisonBox->SetBoxExtent(FVector(CollisionBoxSize));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("InitalizeWeaponComponent Fail"));
	}

}

void UWeaponMeleeAttackComponent::EnableCollisionBoxCheck()
{
	// 根据配置数据初始化碰撞盒大小和配置
	RETURN_IF_FALSE(CollisonBox);
	CollisonBox->OnComponentBeginOverlap.AddDynamic(this, &UWeaponMeleeAttackComponent::OnAttackOverlapBegin);
	CollisonBox->OnComponentEndOverlap.AddDynamic(this, &UWeaponMeleeAttackComponent::OnAttackOverlapEnd);
	// 设置碰撞
	CollisonBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisonBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisonBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisonBox->SetGenerateOverlapEvents(true);
	// 开启定时器
	GetWorld()->GetTimerManager().SetTimer(DamageTimerHandle, this,
		&UWeaponMeleeAttackComponent::ApplyDamageToOverlappingActors,
		DamageInterval, true);
	// 重置已命中目标记录
	AlreadyHitActors.Reset();
}

void UWeaponMeleeAttackComponent::DisableCollisionBoxCheck()
{
	// 根据配置数据初始化碰撞盒大小和配置
	CollisonBox->OnComponentBeginOverlap.RemoveDynamic(this, &UWeaponMeleeAttackComponent::OnAttackOverlapBegin);
	CollisonBox->OnComponentEndOverlap.RemoveDynamic(this, &UWeaponMeleeAttackComponent::OnAttackOverlapEnd);
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

void UWeaponMeleeAttackComponent::OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OnwerActor && OtherActor != OnwerActor)
	{
		AttackOverlappingActors.AddUnique(OtherActor);
	}
}

void UWeaponMeleeAttackComponent::OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AttackOverlappingActors.Remove(OtherActor);
	}
}

void UWeaponMeleeAttackComponent::ApplyDamageToOverlappingActors()
{
	for (AActor* Actor : AttackOverlappingActors)
	{
		if (Actor)
		{
			if (Actor && OnwerActor && OnwerActor != Actor && !AlreadyHitActors.Contains(Actor))
			{
				AlreadyHitActors.Add(Actor);

				// 1. 检查目标是否具有GAS系统（新的SL_CharacterBase）
				IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Actor);
				if (!TargetASI) continue;

				UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
				if (!TargetASC) continue;

				// 2. 计算最终伤害
				float FinalDamage = CalculateFinalDamage(Actor);

				// 3. 创建GE上下文
				FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
				EffectContext.AddInstigator(OnwerActor, OnwerWeapon);

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
			}
		}
	}
}

float UWeaponMeleeAttackComponent::CalculateFinalDamage_Implementation(AActor* InTargetActor) const
{
	// 1. 检查目标是否具有GAS系统（新的SL_CharacterBase）
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(InTargetActor);
	if (!TargetASI) return 0.0f;

	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (!TargetASC) return 0.0f;

	// 2. 计算最终伤害
	float FinalDamage = 20.0f;

	// 从ComboManager获取当前连击倍率
	if (OnwerActor)
	{
		// 方式1：如果OwningCharacter本身就实现了IAbilitySystemInterface
		if (IAbilitySystemInterface* OwnerASI = Cast<IAbilitySystemInterface>(OnwerActor))
		{
			if (UAbilitySystemComponent* OwnerASC = OwnerASI->GetAbilitySystemComponent())
			{
				// 从Owner的Actor上找ComboManager组件
				USL_ComboManagerComponent* ComboMgr =
					OnwerActor->FindComponentByClass<USL_ComboManagerComponent>();
				if (ComboMgr)
				{
					FinalDamage *= ComboMgr->GetCurrentComboDamageMultiplier();
				}
			}
		}
	}

	return FinalDamage;
}


void UWeaponMeleeAttackComponent::EnableParryWindowCheck(float Duration)
{
	// 更新标志位
	bActiveParryWindow = true;

//	// 设置定时器自动关闭弹反窗口,当弹反成功时需要通过定时器关闭
//	GetWorld()->GetTimerManager().SetTimer(
//		ParryWindowTimer,
//		this,
//		&UWeaponMeleeAttackComponent::DisableParryWindowCheck,
//		Duration,
//		false
//	);
}

void UWeaponMeleeAttackComponent::DisableParryWindowCheck()
{
	bActiveParryWindow = false;
//	ParryWindowTimer.Invalidate();
}

bool UWeaponMeleeAttackComponent::IsActiveParryWindow()
{
	return bActiveParryWindow;
}
