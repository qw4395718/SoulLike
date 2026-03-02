// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "WeaponParryComponent.generated.h"

class AWeaponBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API UWeaponParryComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponParryComponent();

	/************************************************************************/
	/*外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void InitalizeWeaponComponent(AActor* Onwer,FVector CBSize);
	
	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void EnableCollisionBoxCheck();

	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void DisableCollisionBoxCheck();

protected:
	/************************************************************************/
	/*内部调用                                                                     */
	/************************************************************************/
	// 碰撞重叠开始
	UFUNCTION()
		void OnCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);

	// 碰撞重叠结束
	UFUNCTION()
		void OnCollisionOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 处理碰撞重叠actor
	UFUNCTION()
		void ApplyParryToOverlappingActors();


protected:
	/************************************************************************/
	/*内部变量                                                                     */
	/************************************************************************/

	// 持有者(武器类)引用
	UPROPERTY()
		AActor* OnwerWeapon;

	// 持有者(角色类)引用
	UPROPERTY()
		AActor* OnwerActor;

	// 碰撞盒尺寸
	UPROPERTY()
		FVector CollisionBoxSize;

	// 碰撞盒-规则多边形
	UPROPERTY()
		UBoxComponent* CollisonBox;

	// 弹反定时器handle
	UPROPERTY()
		FTimerHandle ParryTimerHandle;

	// 定时器间隔
	UPROPERTY()
		float ParrtInterval;

	// 碰撞命中actor数组
	UPROPERTY()
		TArray<AActor*> CollsionOverlappingActors;

	// 已应用弹反actor集
	UPROPERTY()
		TSet<AActor*> AlreadyParryActors;

};
