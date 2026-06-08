// DamageFloatingTextManagerComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoulLikeGameGlobal.h"
#include "DamageFloatingTextManagerComponent.generated.h"

class A_DamageFloatingTextActor;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SOULLIKEDEMO_API UDamageFloatingTextManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDamageFloatingTextManagerComponent();

	/************************************************************************/
	/* 继承实现                                                                     */
	/************************************************************************/
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/************************************************************************/
	/* 外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "Damage Floating")
	void SpawnFloatingText(const FDamageFloatingTextData& InData);

	UFUNCTION()
	void OnDamageFloatingTextRequest(const FDamageFloatingTextData& InData);

	UFUNCTION()
	void OnHealthChangedForFloatingText(AActor* TargetActor, float OldValue, float NewValue,
		float MinValue, float MaxValue);

protected:
	/************************************************************************/
	/* 内部调用                                                                     */
	/************************************************************************/
	A_DamageFloatingTextActor* AcquireFromPool();
	void ReleaseToPool(A_DamageFloatingTextActor* InActor);
	void PreAllocatePool();
	void ExpandPool(int32 ExpandSize);

protected:
	/************************************************************************/
	/* 内部访问                                                                     */
	/************************************************************************/
	UPROPERTY()
	TArray<A_DamageFloatingTextActor*> ActorPool;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	int32 PoolSize = 20;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	int32 MaxPoolSize = 50;

	UPROPERTY(EditDefaultsOnly, Category = "Damage Floating|Config")
	TSubclassOf<A_DamageFloatingTextActor> FloatingTextActorClass;

	FDelegateHandle DamageFloatingTextHandle;
	FDelegateHandle HealthChangedHandle;
	bool bIsDelegateBound;
};
