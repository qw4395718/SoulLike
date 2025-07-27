// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeapon.h"

void AMeleeWeapon::Initialize()
{

}

float AMeleeWeapon::GetStaminaCost(EAttackType AttackType)
{
	//后续改为读取配置返回指定的体力消耗
	return 10.0f;
}

void AMeleeWeapon::OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

}
