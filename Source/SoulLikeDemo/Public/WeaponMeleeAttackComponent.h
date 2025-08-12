// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponBase.h"
#include "WeaponMeleeAttackComponent.generated.h"

class AWeaponBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOULLIKEDEMO_API UWeaponMeleeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponMeleeAttackComponent();

	/************************************************************************/
	/*外部调用                                                                     */
	/************************************************************************/
	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void InitalizeWeaponComponent(AWeaponBase* Onwer,FVector CBSize);
	
	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void EnableCollisionBoxCheck();

	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void DisableCollisionBoxCheck();

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
	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void ApplyDamageToOverlappingActors();

	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void EnableParryWindowCheck();

	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		void DisableParryWindowCheck();

	UFUNCTION(BlueprintCallable, Category = "WeaponComponent")
		bool IsActiveParryWindow();

protected:
	/************************************************************************/
	/*内部调用                                                                     */
	/************************************************************************/



protected:
	/************************************************************************/
	/*内部变量                                                                     */
	/************************************************************************/

	// 持有者(武器类)引用
	UPROPERTY()
		AWeaponBase* OnwerWeapon;

	// 碰撞盒尺寸
	UPROPERTY()
		FVector CollisionBoxSize;

	// 是否激活碰撞盒子
	UPROPERTY()
		bool bEnableCollisionBox;

	// 是否激活弹反窗口
	UPROPERTY()
		bool bActiveParryWindow;

	// 伤害碰撞命中actor数组
	UPROPERTY()
		TArray<AActor*> AttackOverlappingActors;

	// 已应用伤害actor集
	UPROPERTY()
		TSet<AActor*> AlreadyHitActors;

};
