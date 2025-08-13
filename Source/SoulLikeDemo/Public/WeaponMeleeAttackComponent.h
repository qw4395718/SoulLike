// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/BoxComponent.h"
#include "WeaponMeleeAttackComponent.generated.h"

class AWeaponBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API UWeaponMeleeAttackComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponMeleeAttackComponent();

	/************************************************************************/
	/*外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void InitalizeWeaponComponent(AActor* Onwer,FVector CBSize);
	
	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void EnableCollisionBoxCheck();

	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void DisableCollisionBoxCheck();

	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void EnableParryWindowCheck(float Duration);

	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void DisableParryWindowCheck();

	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		bool IsActiveParryWindow();

protected:
	/************************************************************************/
	/*内部调用                                                                     */
	/************************************************************************/
	// 攻击重叠开始
	UFUNCTION()
		void OnAttackOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);

	// 攻击重叠结束
	UFUNCTION()
		void OnAttackOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 处理攻击重叠actor
	UFUNCTION()
		void ApplyDamageToOverlappingActors();


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
	UPROPERTY(VisibleAnywhere, Category = "Components")
		UBoxComponent* CollisonBox;

	// 是否激活弹反窗口
	UPROPERTY()
		bool bActiveParryWindow;

	// 伤害定时器handle
	UPROPERTY()
		FTimerHandle DamageTimerHandle;

	// 弹反定时器handle
	UPROPERTY()
		FTimerHandle ParryWindowTimer;

	// 定时器间隔
	UPROPERTY()
		float DamageInterval;

	// 伤害碰撞命中actor数组
	UPROPERTY()
		TArray<AActor*> AttackOverlappingActors;

	// 已应用伤害actor集
	UPROPERTY()
		TSet<AActor*> AlreadyHitActors;

};
