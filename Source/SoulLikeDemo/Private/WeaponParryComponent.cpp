// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponParryComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SceneComponent.h"

UWeaponParryComponent::UWeaponParryComponent()
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
	OnwerCharacter = nullptr;
	ParrtInterval = 0.1f;
	ParryTimerHandle.Invalidate();
	CollsionOverlappingActors.Reset();
	AlreadyParryActors.Reset();
}

void UWeaponParryComponent::InitalizeWeaponComponent(AActor* Onwer, FVector CBSize)
{
	if (Onwer != nullptr && Cast<ACharacter>(Onwer->GetOwner()))
	{
		OnwerWeapon = Onwer;
		OnwerCharacter = Cast<ACharacter>(Onwer->GetOwner());
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("InitalizeWeaponComponent Fail"));
	}
	CollisionBoxSize = CBSize;
}

void UWeaponParryComponent::EnableCollisionBoxCheck()
{
	// 根据配置数据初始化碰撞盒大小和配置
	CollisonBox->OnComponentBeginOverlap.AddDynamic(this, &UWeaponParryComponent::OnCollisionOverlapBegin);
	CollisonBox->OnComponentEndOverlap.AddDynamic(this, &UWeaponParryComponent::OnCollisionOverlapEnd);
	// 设置碰撞
	CollisonBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisonBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisonBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisonBox->SetGenerateOverlapEvents(true);
	// 开启定时器
	GetWorld()->GetTimerManager().SetTimer(ParryTimerHandle, this,
		&UWeaponParryComponent::ApplyParryToOverlappingActors,
		ParrtInterval, true);
	// 重置已命中目标记录
	AlreadyParryActors.Reset();
}

void UWeaponParryComponent::DisableCollisionBoxCheck()
{
	// 根据配置数据初始化碰撞盒大小和配置
	CollisonBox->OnComponentBeginOverlap.RemoveDynamic(this, &UWeaponParryComponent::OnCollisionOverlapBegin);
	CollisonBox->OnComponentEndOverlap.RemoveDynamic(this, &UWeaponParryComponent::OnCollisionOverlapEnd);
	// 关闭碰撞检测
	CollisonBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollisonBox->SetGenerateOverlapEvents(false);
	// 关闭定时器
	if (ParryTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(ParryTimerHandle);
	}
	// 清空已名字目标记录
	AlreadyParryActors.Reset();
}

void UWeaponParryComponent::OnCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OnwerCharacter && OtherActor != OnwerCharacter)
	{
		CollsionOverlappingActors.AddUnique(OtherActor);
	}
}

void UWeaponParryComponent::OnCollisionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		CollsionOverlappingActors.Remove(OtherActor);
	}
}

void UWeaponParryComponent::ApplyParryToOverlappingActors()
{
	for (AActor* Actor : CollsionOverlappingActors)
	{
		if (Actor)
		{
			if (Actor && OnwerWeapon && OnwerWeapon->GetOwner() != Actor && !AlreadyParryActors.Contains(Actor))
			{
				// 调用武器类的弹反结算逻辑

			}
		}
	}
}

