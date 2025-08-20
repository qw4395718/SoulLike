// Fill out your copyright notice in the Description page of Project Settings.


#include "Combatant.h"

// Add default functionality here for any IDamageable functions that are not pure virtual.

void ICombatant::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	return;
}

float ICombatant::GetCurrentHealth()
{
	return 0.0f;
}

bool ICombatant::IsAlive()
{
	return false;
}
