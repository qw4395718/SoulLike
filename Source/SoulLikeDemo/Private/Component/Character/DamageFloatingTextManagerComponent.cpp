// DamageFloatingTextManagerComponent.cpp
#include "DamageFloatingTextManagerComponent.h"
#include "A_DamageFloatingTextActor.h"
#include "GlobalDelegatesManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UDamageFloatingTextManagerComponent::UDamageFloatingTextManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UDamageFloatingTextManagerComponent::PostInitProperties()
{
	Super::PostInitProperties();

	bIsDelegateBound = false;
}


void UDamageFloatingTextManagerComponent::OnRegister()
{
	Super::OnRegister();

	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		DamageFloatingTextHandle = DelegateMgr->OnDamageFloatingText.AddUObject(
			this, &UDamageFloatingTextManagerComponent::OnDamageFloatingTextRequest);
		bIsDelegateBound = true;
	}
}


void UDamageFloatingTextManagerComponent::OnUnregister()
{
	if (bIsDelegateBound)
	{
		if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
		{
			if (DamageFloatingTextHandle.IsValid())
			{
				DelegateMgr->OnDamageFloatingText.Remove(DamageFloatingTextHandle);
				DamageFloatingTextHandle.Reset();
			}
			if (HealthChangedHandle.IsValid())
			{
				DelegateMgr->OnAttributeHealthChanged.Remove(HealthChangedHandle);
				HealthChangedHandle.Reset();
			}
		}
		bIsDelegateBound = false;
	}

	Super::OnUnregister();
}

/************************************************************************/
/* 继承实现                                                                     */
/************************************************************************/

void UDamageFloatingTextManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	PreAllocatePool();

}

void UDamageFloatingTextManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (A_DamageFloatingTextActor* Actor : ActorPool)
	{
		if (Actor && Actor->IsValidLowLevel())
		{
			Actor->SetReleaseCallback(nullptr);
			Actor->Destroy();
		}
	}
	ActorPool.Empty();

	Super::EndPlay(EndPlayReason);
}

/************************************************************************/
/* 外部调用                                                                     */
/************************************************************************/

void UDamageFloatingTextManagerComponent::SpawnFloatingText(const FDamageFloatingTextData& InData)
{
	A_DamageFloatingTextActor* Actor = AcquireFromPool();
	if (Actor)
	{
		Actor->ShowText(InData);
	}
}

void UDamageFloatingTextManagerComponent::OnDamageFloatingTextRequest(const FDamageFloatingTextData& InData)
{
	SpawnFloatingText(InData);
}

void UDamageFloatingTextManagerComponent::OnHealthChangedForFloatingText(
	AActor* TargetActor, float OldValue, float NewValue, float MinValue, float MaxValue)
{
	const float DamageValue = OldValue - NewValue;
	if (DamageValue <= 0.0f || !TargetActor) return;

	FDamageFloatingTextData TextData;
	TextData.DamageValue = DamageValue;
	TextData.HitWorldLocation = TargetActor->GetActorLocation();
	TextData.bIsCriticalHit = false;
	TextData.TargetActor = TargetActor;

	SpawnFloatingText(TextData);
}

/************************************************************************/
/* 内部调用                                                                     */
/************************************************************************/

void UDamageFloatingTextManagerComponent::PreAllocatePool()
{
	if (!FloatingTextActorClass)
	{
		// 目前先使用硬编码的形式
		FloatingTextActorClass = LoadClass<A_DamageFloatingTextActor>(
			nullptr,
			TEXT("/Game/SoulLikeDemo/Blueprints/Actor/BP_DamageFloatingTextActor.BP_DamageFloatingTextActor_C")
			);
	}

	const int32 ActualPoolSize = FMath::Min(PoolSize, MaxPoolSize);
	ActorPool.Reserve(MaxPoolSize);

	for (int32 i = 0; i < ActualPoolSize; ++i)
	{
		A_DamageFloatingTextActor* NewActor = GetWorld()->SpawnActor<A_DamageFloatingTextActor>(FloatingTextActorClass);
		if (NewActor)
		{
			NewActor->SetReleaseCallback([this](A_DamageFloatingTextActor* ReleasedActor)
			{
				ReleaseToPool(ReleasedActor);
			});
			NewActor->SetActorHiddenInGame(true);
			NewActor->SetActorTickEnabled(false);
			ActorPool.Add(NewActor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DamageFloatingTextManagerComponent: 预生成 %d 个飘字 Actor"), ActualPoolSize);
}

A_DamageFloatingTextActor* UDamageFloatingTextManagerComponent::AcquireFromPool()
{
	for (A_DamageFloatingTextActor* Actor : ActorPool)
	{
		if (Actor && !Actor->IsTextActive())
		{
			return Actor;
		}
	}

	if (ActorPool.Num() < MaxPoolSize)
	{
		const int32 ExpandCount = FMath::Min(5, MaxPoolSize - ActorPool.Num());
		ExpandPool(ExpandCount);
		if (ActorPool.Num() > 0)
		{
			A_DamageFloatingTextActor* NewActor = ActorPool.Last();
			if (NewActor && !NewActor->IsTextActive())
			{
				return NewActor;
			}
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("DamageFloatingTextManagerComponent: 池已满（%d/%d），丢弃飘字请求"),
		ActorPool.Num(), MaxPoolSize);
	return nullptr;
}

void UDamageFloatingTextManagerComponent::ReleaseToPool(A_DamageFloatingTextActor* InActor)
{
	if (!InActor) return;
	InActor->SetActorHiddenInGame(true);
	InActor->SetActorTickEnabled(false);
}

void UDamageFloatingTextManagerComponent::ExpandPool(int32 ExpandSize)
{
	if (!FloatingTextActorClass) return;

	const int32 MaxCanAdd = MaxPoolSize - ActorPool.Num();
	const int32 ActualExpand = FMath::Min(ExpandSize, MaxCanAdd);

	for (int32 i = 0; i < ActualExpand; ++i)
	{
		A_DamageFloatingTextActor* NewActor = GetWorld()->SpawnActor<A_DamageFloatingTextActor>(FloatingTextActorClass);
		if (NewActor)
		{
			NewActor->SetReleaseCallback([this](A_DamageFloatingTextActor* ReleasedActor)
			{
				ReleaseToPool(ReleasedActor);
			});
			NewActor->SetActorHiddenInGame(true);
			NewActor->SetActorTickEnabled(false);
			ActorPool.Add(NewActor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("DamageFloatingTextManagerComponent: 池扩容 %d，当前大小 %d"), ActualExpand, ActorPool.Num());
}
