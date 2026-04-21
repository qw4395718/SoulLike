// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CharacterBase.h"
#include "WeaponAnimNotify_IF.h"
#include "SL_AbilitySystemComponent.h"
#include <SL_StatusAttributeSet.h>
#include <Abilities/GameplayAbilityTypes.h>
#include <Components/WidgetComponent.h>
#include <UIManagerSubsystem.h>
#include <Engine/PackageMapClient.h>

DEFINE_LOG_CATEGORY(SL_CharacterBase);

ASL_CharacterBase::ASL_CharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	/************************************************************************/
	/*                                GAS组件相关                                      */
	/************************************************************************/
	 // ASC-核心功能组件
	AbilitySystemComp = CreateDefaultSubobject<USL_AbilitySystemComponent>(TEXT("AbilitySystem"));
	// AS(CharacterCombatState)
	StatusAttributeSet = CreateDefaultSubobject<USL_StatusAttributeSet>(TEXT("StatusSet"));
	// 
	ScreenWidgetCmp = CreateDefaultSubobject<UWidgetComponent>(TEXT("ScreenWidgetCmp"));
	ScreenWidgetCmp->SetupAttachment(GetMesh()); // 挂在骨骼上
	ScreenWidgetCmp->SetWidgetSpace(EWidgetSpace::Screen);
	ScreenWidgetCmp->SetDrawSize(FVector2D(200, 30));
	ScreenWidgetCmp->SetRelativeLocation(FVector(0, 0, 180)); // 头部偏移

	// 设置组件标签（重要！用于查找）
	ScreenWidgetCmp->ComponentTags.Add(FName("HeadUI"));

	// 默认不激活Widget，等UIManager来设置
	ScreenWidgetCmp->SetWidget(nullptr);
	ScreenWidgetCmp->SetVisibility(false);


}

void ASL_CharacterBase::BeginPlay()
{
	Super::BeginPlay();
	// 初始化角色(包含组件)
	InitializeCharacter();

	// 为ASC设置持有者和化身
	if (AbilitySystemComp)
	{
		AbilitySystemComp->InitAbilityActorInfo(this, this);
	}

	if (StatusAttributeSet)
	{
		StatusAttributeSet->SetOwningActor(this);
		StatusAttributeSet->InitStatusAS();
	}
	
	if (!IsPlayerControlled())
	{// 非自身SL_CharacterBase实例
		if (UUIManagerSubsystem* pUIManagerSystem = UUIManagerSubsystem::Get(this))
		{
			// 创建结构体
			FUICreateParams createParam;
			createParam.Type = EWidgetType::EWIDGET_PawnStatusInScreen;
			createParam.TargetActor = this;

			pUIManagerSystem->OpenWidget(createParam);
		}
	}

	UE_LOG(SL_CharacterBase, Display, TEXT("ZYF_C++_ASL_CharacterBase::BeginPlay()"));

	UE_LOG(LogTemp, Warning, TEXT("Character %s BeginPlay: Role=%d, HasAuthority=%d, GUID=%s"),
		*GetName(),
		(int)GetLocalRole(),
		HasAuthority(),
		*GetNetworkGUIDString(this));
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
	if (!AbilitySystemComp)
		return;
	FString EnumName = TEXT("EMyAbilitySlotsEnum");
	FGameplayAbilityInputBinds Binds(
		"ConfirmTargeting",     // 确认动作名
		"CancelTargeting",      // 取消动作名
		EnumName,               // 枚举名称（直接传FString）
		0,                      // 可选的起始输入ID
		true                    // 是否尝试将枚举值映射到输入ID
	);
	AbilitySystemComp->BindAbilityActivationToInputComponent(PlayerInputComponent, Binds);
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
	UE_LOG(LogTemp, Warning, TEXT("Character %s BeginPlay: Role=%d, HasAuthority=%d NetworkGUID: %s"),
		*GetName(),
		(int)GetLocalRole(),
		HasAuthority(),
		*GetNetworkGUIDString(this));
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

		for (int i = 0;i< 1/*EQUIPMENT_SLOT_NUM*2*/;i++)
		{ 
			WeaponList.Add(100000+i);
			ItemList.Add(200000+i);
		}
		
		//ActiveList.Add(EArrowKeyType::ARROWKEY_Left,0);
		ActiveList.Add(EArrowKeyType::ARROWKEY_Right, 0);
		//ActiveList.Add(EArrowKeyType::ARROWKEY_Up, 0);
		//ActiveList.Add(EArrowKeyType::ARROWKEY_Down, 0);
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
	return AbilitySystemComp;
}

FString ASL_CharacterBase::GetNetworkGUIDString(AActor* InActor)
{
	if(InActor == nullptr){return TEXT("InActor InValid"); }
	UNetDriver* NetDriver = InActor->GetWorld()->GetNetDriver();
	if (NetDriver == nullptr) { return TEXT("NetDriver InValid"); }
	if (InActor->GetLocalRole() == ROLE_Authority)
	{
		// 服务器端：遍历 ClientConnections
		for (UNetConnection* NetConnection : NetDriver->ClientConnections)
		{
			// 找到拥有此 Actor 的那个连接
			if (UPackageMapClient* PackageMapClient = Cast<UPackageMapClient>(NetConnection->PackageMap))
			{
				FNetworkGUID NetGUID = PackageMapClient->GetNetGUIDFromObject(InActor);
				if (NetGUID.IsValid())
				{
					return NetGUID.ToString();
				}
			}
		}
	}
	else
	{
		// 客户端端：使用 ServerConnection
		UPackageMapClient* PackageMap = Cast<UPackageMapClient>(NetDriver->ServerConnection->PackageMap);

		FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(InActor);
		if (NetGUID.IsValid())
		{
			return NetGUID.ToString();
		}
		
	}

	return TEXT("Invalid GUID");
}




