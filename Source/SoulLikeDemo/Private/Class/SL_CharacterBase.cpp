// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CharacterBase.h"
#include "WeaponAnimNotify_IF.h"
#include "LabAbilitySystemComponent.h"
#include <LabHealthAttributeSet.h>
#include <Abilities/GameplayAbilityTypes.h>

DEFINE_LOG_CATEGORY(SL_CharacterBase);

// Sets default values
ASL_CharacterBase::ASL_CharacterBase()
{
	/************************************************************************/
	/*                                GAS组件相关                                      */
	/************************************************************************/
	 // ASC-核心功能组件
	LabAbilitySystemComp = CreateDefaultSubobject<ULabAbilitySystemComponent>(TEXT("AbilitySystem"));
	// AS(CharacterCombatState)
	HealthSet = CreateDefaultSubobject<ULabHealthAttributeSet>(TEXT("HealthSet"));

	/************************************************************************/
	/*								UI组件                                         */
	/************************************************************************/


}

void ASL_CharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeCharacter();

	// 为ASC设置持有者和化身
	LabAbilitySystemComp->InitAbilityActorInfo(this, this);


	/************************************************************************/
	/*								绑定AS与UMG                                         */
	/************************************************************************/
	 // 生命值相关AS
	if (HealthSet)
	{
		HealthSet->OnHealthChanged.AddDynamic(this ,&ASL_CharacterBase::HandleHealthChanged);
	}

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

	// ASC能力
	if (!LabAbilitySystemComp)
		return;
	FString EnumName = TEXT("EMyAbilitySlotsEnum");
	FGameplayAbilityInputBinds Binds(
		"ConfirmTargeting",     // 确认动作名
		"CancelTargeting",      // 取消动作名
		EnumName,               // 枚举名称（直接传FString）
		0,                      // 可选的起始输入ID
		true                    // 是否尝试将枚举值映射到输入ID
	);
	LabAbilitySystemComp->BindAbilityActivationToInputComponent(PlayerInputComponent, Binds);
}



UActorComponent* ASL_CharacterBase::GetCombatantComponent()
{
	return CombatCmp;
}

UActorComponent* ASL_CharacterBase::GetEquipmentComponent()
{
	return EquipmentCmp;
}

UActorComponent* ASL_CharacterBase::GetHealthComponent()
{
	return HealthCmp;
}

UActorComponent* ASL_CharacterBase::GetInventoryComponent()
{
	return InventoryCmp;
}

UActorComponent* ASL_CharacterBase::GetSpecialMovementComponent()
{
	return MovementCmp;
}

UActorComponent* ASL_CharacterBase::GetStaminaComponent()
{
	return StaminaCmp;
}

UActorComponent* ASL_CharacterBase::GetStateComponent()
{
	return StateCmp;
}

void ASL_CharacterBase::AnimNotifyResponse(int NotifyType)
{
	// 参数检查
	RETURN_IF_TRUE(NotifyType <= int(EAnimNotifyType::EAnimNotify_Min) || NotifyType >= int(EAnimNotifyType::EAnimNotify_Max));
	WeaponAnimProcess(CheckAnimNotifyToHand(EAnimNotifyType(NotifyType)), TranslteAnimNotifyToWeapon(EAnimNotifyType(NotifyType)));
}

void ASL_CharacterBase::PerformAttack()
{	
	if (CombatCmp)
	{
		CombatCmp->PerformAttack();
	}
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

EWeaponAnimNotifyType ASL_CharacterBase::TranslteAnimNotifyToWeapon(EAnimNotifyType NotifyType)
{
	// 参数检查
	RETURN_VALUE_IF_FALSE(NotifyType <= EAnimNotifyType::EAnimNotify_Min || NotifyType <= EAnimNotifyType::EAnimNotify_Max, EWeaponAnimNotifyType::EWeaponAnimNotify_Min);
	
	EWeaponAnimNotifyType WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_Min;
	
	switch (EAnimNotifyType(NotifyType))
	{
	case EAnimNotifyType::EAnimNotify_LH_Active_NormalComboWindow:
	case EAnimNotifyType::EAnimNotify_RH_Active_NormalComboWindow:
	{
		WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_Active_NormalComboWindow;
	}; break;
	
	case EAnimNotifyType::EAnimNotify_LH_InActive_NormalComboWindow:
	case EAnimNotifyType::EAnimNotify_RH_InActive_NormalComboWindow:
	{
		WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_InActive_NormalComboWindow;
	}; break;

	case EAnimNotifyType::EAnimNotify_LH_Active_SkillComboWindow:
	case EAnimNotifyType::EAnimNotify_RH_Active_SkillComboWindow:
	{
		WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_Active_SkillComboWindow;
	}; break;

	case EAnimNotifyType::EAnimNotify_LH_InActive_SkillComboWindow:
	case EAnimNotifyType::EAnimNotify_RH_InActive_SkillComboWindow:
	{
		WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_InActive_SkillComboWindow;
	}; break;

	case EAnimNotifyType::EAnimNotify_LH_EnableCollision_Melee:
	case EAnimNotifyType::EAnimNotify_RH_EnableCollision_Melee:
	{
		WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_EnableCollision_Melee;
	}; break;

	case EAnimNotifyType::EAnimNotify_LH_DisableCollision_Melee:
	case EAnimNotifyType::EAnimNotify_RH_DisableCollision_Melee:
	{
		WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_DisableCollision_Melee;
	}; break;

	case EAnimNotifyType::EAnimNotify_LH_AcitiveParryWindow_Melee:
	case EAnimNotifyType::EAnimNotify_RH_AcitiveParryWindow_Melee:
	{
		WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_ActiveParryWindow_Melee;
	}; break;

	case EAnimNotifyType::EAnimNotify_LH_InAcitiveParryWindow_Melee:
	case EAnimNotifyType::EAnimNotify_RH_InAcitiveParryWindow_Melee:
	{
		WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_InActiveParryWindow_Melee;
	}; break;

	case EAnimNotifyType::EAnimNotify_LH_EnableCollision_ComboSkil_Parry:
	case EAnimNotifyType::EAnimNotify_RH_EnableCollision_ComboSkil_Parry:
	{
		WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_EnableCollision_ComboSkil_Parry;
	}; break;

	case EAnimNotifyType::EAnimNotify_LH_DisableCollision_ComboSkil_Parry:
	case EAnimNotifyType::EAnimNotify_RH_DisableCollision_ComboSkil_Parry:
	{
		WeaponAnimType = EWeaponAnimNotifyType::EWeaponAnimNotify_DisableCollision_ComboSkil_Parry;
	}; break;

	default:break;
	}
	return WeaponAnimType;
}

int ASL_CharacterBase::CheckAnimNotifyToHand(EAnimNotifyType NotifyType)
{
	// 参数检查
	RETURN_VALUE_IF_FALSE(NotifyType <= EAnimNotifyType::EAnimNotify_Min || NotifyType <= EAnimNotifyType::EAnimNotify_Max, int(EWeaponAnimNotifyType::EWeaponAnimNotify_Min));

	int HandType = -1;

	switch (EAnimNotifyType(NotifyType))
	{
	case EAnimNotifyType::EAnimNotify_LH_Active_NormalComboWindow:
	case EAnimNotifyType::EAnimNotify_LH_InActive_NormalComboWindow:
	case EAnimNotifyType::EAnimNotify_LH_Active_SkillComboWindow:
	case EAnimNotifyType::EAnimNotify_LH_InActive_SkillComboWindow:
	case EAnimNotifyType::EAnimNotify_LH_EnableCollision_Melee:
	case EAnimNotifyType::EAnimNotify_LH_DisableCollision_Melee:
	case EAnimNotifyType::EAnimNotify_LH_AcitiveParryWindow_Melee:
	case EAnimNotifyType::EAnimNotify_LH_InAcitiveParryWindow_Melee:
	case EAnimNotifyType::EAnimNotify_LH_EnableCollision_ComboSkil_Parry:
	case EAnimNotifyType::EAnimNotify_LH_DisableCollision_ComboSkil_Parry:
	{HandType = 0;}break;

	case EAnimNotifyType::EAnimNotify_RH_Active_NormalComboWindow:
	case EAnimNotifyType::EAnimNotify_RH_InActive_NormalComboWindow:
	case EAnimNotifyType::EAnimNotify_RH_Active_SkillComboWindow:
	case EAnimNotifyType::EAnimNotify_RH_InActive_SkillComboWindow:
	case EAnimNotifyType::EAnimNotify_RH_EnableCollision_Melee:
	case EAnimNotifyType::EAnimNotify_RH_DisableCollision_Melee:
	case EAnimNotifyType::EAnimNotify_RH_AcitiveParryWindow_Melee:
	case EAnimNotifyType::EAnimNotify_RH_InAcitiveParryWindow_Melee:
	case EAnimNotifyType::EAnimNotify_RH_EnableCollision_ComboSkil_Parry:
	case EAnimNotifyType::EAnimNotify_RH_DisableCollision_ComboSkil_Parry:
	{HandType = 1; }break;
	default:break;
	}

	return HandType;
}

void ASL_CharacterBase::WeaponAnimProcess(int HandType, EWeaponAnimNotifyType WeaponAnimType)
{
	// 检查组件是否有效
	RETURN_IF_TRUE(EquipmentCmp == nullptr);
	RETURN_IF_TRUE(HandType != 0 && HandType != 1);

	if (HandType == false)
	{
		if (EquipmentCmp->GetCurrentLHWeapon())
		{
			ASL_WeaponBase* Weapon = EquipmentCmp->GetCurrentLHWeapon();
			IWeaponAnimNotify_IF* WeaponAnimNotify = Cast<IWeaponAnimNotify_IF>(Weapon);
			if (WeaponAnimNotify == nullptr) { return; }
			WeaponAnimNotify->WeaponAnimNotifyResponse(int(WeaponAnimType));
		}
	}
	else
	{
		if (EquipmentCmp->GetCurrentRHWeapon())
		{
			ASL_WeaponBase* Weapon = EquipmentCmp->GetCurrentLHWeapon();
			IWeaponAnimNotify_IF* WeaponAnimNotify = Cast<IWeaponAnimNotify_IF>(Weapon);
			if (WeaponAnimNotify == nullptr) { return; }
			WeaponAnimNotify->WeaponAnimNotifyResponse(int(WeaponAnimType));

		}
	}
}

void ASL_CharacterBase::HandleHealthChanged(float OldGHealth, float CurrentHealth)
{
	if(OldGHealth == CurrentHealth){return;}

	// 通知UI组件
	// 通知音效组件
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
		CombatCmp->InitCombatComponentInfo(this, TEXT("/Game/SoulLikeDemo/Anim/AM_Character_Hit.AM_Character_Hit") ,0,true);
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

UAbilitySystemComponent* ASL_CharacterBase::GetAbilitySystemComponent() const
{
	return LabAbilitySystemComp;
}

void ASL_CharacterBase::GiveAbilityToSelf(TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass || !LabAbilitySystemComp) return;

	// 检查是否有权限（在单人游戏中永远为真）
	if (HasAuthority())
	{
		LabAbilitySystemComp->GiveAbility(
			FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this)
		);
	}
}

void ASL_CharacterBase::GiveAbility(TSubclassOf<UGameplayAbility> AbilityClass, int32 InLevel, int32 InInputID)
{
	if (!AbilityClass || !LabAbilitySystemComp) return;

	// 检查是否有权限（在单人游戏中永远为真）
	if (HasAuthority())
	{
		LabAbilitySystemComp->GiveAbility(
			FGameplayAbilitySpec(AbilityClass, InLevel, InInputID, this)
		);
	}
}

FGameplayAbilitySpecHandle ASL_CharacterBase::GiveAbilityAndActivateOnce(TSubclassOf<UGameplayAbility> AbilityClass, int32 InLevel, int32 InInputID)
{
	if (!AbilityClass || !LabAbilitySystemComp) return FGameplayAbilitySpecHandle();

	// 检查是否有权限（在单人游戏中永远为真）
	if (HasAuthority())
	{
		FGameplayAbilitySpec temp = FGameplayAbilitySpec(AbilityClass, InLevel, InInputID, this);

		return LabAbilitySystemComp->GiveAbilityAndActivateOnce(
			temp
		);
	}
	return FGameplayAbilitySpecHandle();
}

void ASL_CharacterBase::GiveAbilitiesToSelf(const TArray<TSubclassOf<UGameplayAbility>>& AbilityClasses)
{
	if (!LabAbilitySystemComp) return;

	for (auto AbilityClass : AbilityClasses)
	{
		GiveAbilityToSelf(AbilityClass);
	}
}
