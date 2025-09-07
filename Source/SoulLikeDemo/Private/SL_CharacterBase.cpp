// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CharacterBase.h"

// Sets default values
ASL_CharacterBase::ASL_CharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ASL_CharacterBase::BeginPlay()
{
	
}

void ASL_CharacterBase::Tick(float DeltaTime)
{

}

void ASL_CharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ASL_CharacterBase::PerformAttack);
	PlayerInputComponent->BindAction("Defence", IE_Pressed, this, &ASL_CharacterBase::PerformDefence);
	PlayerInputComponent->BindAction("ComboSkill", IE_Pressed, this, &ASL_CharacterBase::PerformComboSkill);
	PlayerInputComponent->BindAction("LockRotation", IE_Pressed, this, &ASL_CharacterBase::PerformLockRotation);
	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &ASL_CharacterBase::PerformRoll);
	PlayerInputComponent->BindAction("SwitchEquipmentUp", IE_Pressed, this, &ASL_CharacterBase::PerformSwitchEquipmentUp);
	PlayerInputComponent->BindAction("SwitchEquipmentDown", IE_Pressed, this, &ASL_CharacterBase::PerformSwitchEquipmentDown);
	PlayerInputComponent->BindAction("EquipmentLeft", IE_Pressed, this, &ASL_CharacterBase::PerformSwitchEquipmentLeft);
	PlayerInputComponent->BindAction("EquipmentRight", IE_Pressed, this, &ASL_CharacterBase::PerformSwitchEquipmentRight);
}



void ASL_CharacterBase::PerformAttack()
{
	
}

void ASL_CharacterBase::PerformDefence()
{

}

void ASL_CharacterBase::PerformComboSkill()
{

}

void ASL_CharacterBase::PerformLockRotation()
{

}

void ASL_CharacterBase::PerformRoll()
{

}

void ASL_CharacterBase::PerformSwitchEquipmentUp()
{

}

void ASL_CharacterBase::PerformSwitchEquipmentDown()
{

}

void ASL_CharacterBase::PerformSwitchEquipmentLeft()
{

}

void ASL_CharacterBase::PerformSwitchEquipmentRight()
{

}

void ASL_CharacterBase::InitializeCharacter()
{
	//ÄâÔìÊý¾Ý
	InitPartmentComponent();
}

void ASL_CharacterBase::InitPartmentComponent()
{
	
}
