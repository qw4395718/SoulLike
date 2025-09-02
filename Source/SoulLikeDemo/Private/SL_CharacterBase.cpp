// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CharacterBase.h"

// Sets default values
ASL_CharacterBase::ASL_CharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ASL_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ASL_CharacterBase::PerformAttack);
	PlayerInputComponent->BindAction("CombatSkill", IE_Pressed, this, &ASL_CharacterBase::PerformCombatSkill);

}

void ASL_CharacterBase::PerformLeftMouse()
{

}

void ASL_CharacterBase::PerformRightMouse()
{

}

void ASL_CharacterBase::PerformCombatSkill()
{

}
