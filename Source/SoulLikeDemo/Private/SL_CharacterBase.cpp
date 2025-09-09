// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CharacterBase.h"

DEFINE_LOG_CATEGORY(SL_CharacterBase);

// Sets default values
ASL_CharacterBase::ASL_CharacterBase()
{

}

void ASL_CharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeCharacter();
}

void ASL_CharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	UE_LOG(SL_CharacterBase, Display,TEXT("ASL_CharacterBase::PerformAttack"));
}

void ASL_CharacterBase::PerformDefence()
{
	UE_LOG(SL_CharacterBase, Display, TEXT("ASL_CharacterBase::PerformDefence"));
}

void ASL_CharacterBase::PerformComboSkill()
{
	UE_LOG(SL_CharacterBase, Display, TEXT("ASL_CharacterBase::PerformComboSkill"));
}

void ASL_CharacterBase::PerformLockRotation()
{
	UE_LOG(SL_CharacterBase, Display, TEXT("ASL_CharacterBase::PerformLockRotation"));

}

void ASL_CharacterBase::PerformRoll()
{
	UE_LOG(SL_CharacterBase, Display, TEXT("ASL_CharacterBase::PerformRoll"));
}

void ASL_CharacterBase::PerformSwitchEquipmentUp()
{
	UE_LOG(SL_CharacterBase, Display, TEXT("ASL_CharacterBase::PerformSwitchEquipmentUp"));
}

void ASL_CharacterBase::PerformSwitchEquipmentDown()
{
	UE_LOG(SL_CharacterBase, Display, TEXT("ASL_CharacterBase::PerformSwitchEquipmentDown"));
}

void ASL_CharacterBase::PerformSwitchEquipmentLeft()
{
	UE_LOG(SL_CharacterBase, Display, TEXT("ASL_CharacterBase::PerformSwitchEquipmentLeft"));
}

void ASL_CharacterBase::PerformSwitchEquipmentRight()
{
	UE_LOG(SL_CharacterBase, Display, TEXT("ASL_CharacterBase::PerformSwitchEquipmentRight"));
}

void ASL_CharacterBase::InitializeCharacter()
{
	//拟造数据,初始化组件
	InitPartmentComponent();
}

void ASL_CharacterBase::InitPartmentComponent()
{
	if (CombatCmp == nullptr && true)
	{
		CombatCmp = NewObject<USL_CombatantComponent>(this);
		CombatCmp->InitCombatComponentInfo(this,"",0,true);
	}

	if (EquipmentCmp == nullptr && true)
	{
		EquipmentCmp = NewObject<USL_EquipmentComponent>(this);
		// 装备组件的信息初始化放在背包组件中
	}

	if (HealthCmp == nullptr && true)
	{
		HealthCmp = NewObject<USL_HealthComponent>(this);
		float HealthMax = 100.0f;
		HealthCmp->InitHealthInfo(HealthMax);
	}

	if (InventoryCmp == nullptr && true)
	{
		InventoryCmp = NewObject<USL_InventoryComponent>(this);
		TArray<int> WeaponList;
		TArray<int> ItemList;
		TMap<EArrowKeyType, int> ActiveList;
		TArray<FWeaponData> EquipmentWeaponList;

		for (int i = 0;i< EQUIPMENT_SLOT_NUM*2;i++)
		{
			WeaponList.Add(100000+i);
			ItemList.Add(200000+i);
		}
		
		ActiveList.Add(EArrowKeyType::ARROWKEY_Left,0);
		ActiveList.Add(EArrowKeyType::ARROWKEY_Right, 0);
		ActiveList.Add(EArrowKeyType::ARROWKEY_Up, 0);
		ActiveList.Add(EArrowKeyType::ARROWKEY_Down, 0);
		InventoryCmp->InitEquipmentInfo(WeaponList, ItemList, ActiveList);
		InventoryCmp->GetEquipmentInfoList(EquipmentWeaponList);
		if (EquipmentCmp != nullptr)
		{
			EquipmentCmp->InitEquipmentComponent(EquipmentWeaponList, ItemList, ActiveList,this);
		}
	}

	if (StaminaCmp == nullptr && true)
	{
		StaminaCmp = NewObject<USL_StaminaComponent>(this);
		float StaminaMax = 100.0f;
		StaminaCmp->InitStaminaInfo(StaminaMax);
	}

	if (StateCmp == nullptr && true)
	{
		StateCmp = NewObject<USL_StateComponent>(this);
	}

	if (MovementCmp == nullptr && true)
	{
		MovementCmp = NewObject<USL_MovementComponent>(this);
		MovementCmp->InitMovemenetInfo(true,"");
	}

}
