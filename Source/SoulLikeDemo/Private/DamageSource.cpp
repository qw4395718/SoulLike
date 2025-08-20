// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageSource.h"

float IDamageSource::GetBaseDamage()
{
	return 0.0f;
}

// Add default functionality here for any IDamageable functions that are not pure virtual.

AActor* IDamageSource::GetDamageInstigator()
{
	return nullptr;
}
