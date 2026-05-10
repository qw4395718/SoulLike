// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CharacterBase.h"
#include "WeaponAnimNotify_IF.h"
#include "SL_AbilitySystemComponent.h"
#include <SL_StatusAttributeSet.h>
#include <Abilities/GameplayAbilityTypes.h>
#include <Components/WidgetComponent.h>
#include <UIManagerSubsystem.h>
#include <Engine/PackageMapClient.h>
#include <GameplayTagContainer.h>
#include <SL_ComboManagerComponent.h>
#include <Manager/DataTableManager.h>
#include <ClassConfigInfoTable.h>
#include <SoulLikeGameGlobal.h>
#include <Manager/GlobalDelegatesManager.h>
#include <Components/CapsuleComponent.h>

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
	InitCharacterWithClassID(1001);

	// 为ASC设置持有者和化身
	if (AbilitySystemComp)
	{
		AbilitySystemComp->InitAbilityActorInfo(this, this);
		AbilitySystemComp->SetAliveTag();
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

	UE_LOG(LogTemp, Display, TEXT("ZYF_C++_ASL_CharacterBase::BeginPlay()"));

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

UActorComponent* ASL_CharacterBase::GetComboManagerComponent()
{
	return ComboManagerCmp;
}

void ASL_CharacterBase::AnimNotifyResponse(int NotifyType)
{
	// 参数检查
	RETURN_IF_TRUE(NotifyType <= int(EAnimNotifyType::EAnimNotify_Min) || NotifyType >= int(EAnimNotifyType::EAnimNotify_Max));
	WeaponAnimProcess(CheckAnimNotifyToHand(EAnimNotifyType(NotifyType)), TranslteAnimNotifyToWeapon(EAnimNotifyType(NotifyType)));
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

	if (ASL_WeaponBase* Weapon = EquipmentCmp->GetWeaponByHand(HandType))
	{
		IWeaponAnimNotify_IF* WeaponAnimNotify = Cast<IWeaponAnimNotify_IF>(Weapon);
		if (WeaponAnimNotify == nullptr) { return; }
		WeaponAnimNotify->WeaponAnimNotifyResponse(int(WeaponAnimType));
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
		if (EquipmentCmp)
		{
			EquipmentCmp->SetOwner(this);
		}
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
	}

	if (StaminaCmp == nullptr && true)
	{
		StaminaCmp = NewObject<USL_StaminaComponent>(this);
		StaminaCmp->InitializeStaminaComponent();
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

	if (ComboManagerCmp == nullptr && true)
	{
		ComboManagerCmp = NewObject<USL_ComboManagerComponent>(this);
	}
}

UAbilitySystemComponent* ASL_CharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

void ASL_CharacterBase::SetClassID(int32 InPlayerClassID)
{
	PlayerClassID = InPlayerClassID;
	// 此处临时存档,需要等gamemodebase 重写RestartPlayer后才能走begin正常初始化的流程
	InitCharacterWithClassID(PlayerClassID);
}

void ASL_CharacterBase::InitCharacterWithClassID(int32 InPlayerClassID)
{
	UDataTableManager* TableManager = UDataTableManager::Get(this);
	if (!TableManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ASL_CharacterBase::InitializeCharacter - DataTableManager not found"));
		return;
	}
	UClassConfigInfoTable* ClassTable = Cast<UClassConfigInfoTable>(TableManager->GetDataTable(EDataTableType::DT_ClassConfigInfo));
	if (!ClassTable)
	{
		UE_LOG(LogTemp, Error, TEXT("ASL_CharacterBase::InitializeCharacter - ClassConfigInfoTable not found"));
		return;
	}

	FClassConfigInfo Config;
	if(!ClassTable->GetClassConfig(InPlayerClassID, Config))
	{
		UE_LOG(LogTemp, Error, TEXT("ASL_CharacterBase::InitializeCharacter - ClassConfig not found"));
		return;
	}

	// 保存角色的配置
	ClassConfig = Config;

	// 应用配置
	ApplyEnemyConfig(ClassConfig);

	BindGASDeathEvent();

	// Todo:切换职业的时候需要广播,改变能力值和Tag
}

void ASL_CharacterBase::ApplyEnemyConfig(const FClassConfigInfo& Config)
{
	// 1. 设置属性（通过GAS）
	if (AbilitySystemComp)
	{
		AbilitySystemComp->SetAliveTag();
		// 设置初始血量
		if (USL_StatusAttributeSet* StatusSet = const_cast<USL_StatusAttributeSet*>(AbilitySystemComp->GetSet<USL_StatusAttributeSet>()))
		{
			// 通过GAS的Attribute设置初始值
			StatusSet->InitHealth(Config.BaseHealth);
			StatusSet->InitMaxHealth(Config.BaseHealth);
			StatusSet->InitStamina(Config.BaseStamina);
			StatusSet->InitMaxStamina(Config.BaseStamina);
			// StatusSet->InitAttack(Config.BaseAttack);
			// StatusSet->InitDefense(Config.BaseDefense);
			// 设置其他属性
			// 注意：Stamina、Attack、Defense等需要额外配置GE
		}
	}
	if (EquipmentCmp)
	{
		EquipmentCmp->InitializeWithClassID(PlayerClassID);
	}

	
}

void ASL_CharacterBase::BindGASDeathEvent()
{
	// 绑定GAS的死亡委托
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		// 防止重复绑定
		if (!OnCharacterDiedHandle.IsValid())
		{
			OnCharacterDiedHandle = DelegateMgr->OnCharacterDied.AddUObject(this, &ASL_CharacterBase::OnGASCharacterDied);
		}
	}
}

void ASL_CharacterBase::OnGASCharacterDied(AActor* DiedActor, AActor* KillerActor)
{
	// 检查是否是自己的死亡事件
	if (DiedActor != this) return;

	UE_LOG(LogTemp, Log, TEXT("ASL_CharacterBase::OnGASCharacterDied - Enemy %s died"), *GetName());

	// 确保执行死亡逻辑
	if (CurrentState != EPlayerState::Dead)
	{
		Die();
	}
}

void ASL_CharacterBase::OnGASCharacterRevive(AActor* ReviveActor)
{
	// 检查是否是自己的复活事件
	if (ReviveActor != this) return;

	UE_LOG(LogTemp, Log, TEXT("ASL_CharacterBase::OnGASCharacterRevive - Enemy %s revived"), *GetName());

	// 确保执行复活逻辑
	if (CurrentState != EPlayerState::Alive)
	{
		Revive();
	}
}

void ASL_CharacterBase::Die()
{
	if (CurrentState == EPlayerState::Dead) return;
	CurrentState = EPlayerState::Dead;

    // 禁用角色
    if (ACharacter* Char = Cast<ACharacter>(this))
    {
        // 注意：UE4.26中GetMesh()可能返回null，需检查
        Char->SetActorEnableCollision(false);
        
        if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
        {
            PC->SetCinematicMode(true, false, false);
        }

		if (UCapsuleComponent* comp = Char->GetCapsuleComponent())
		{
			comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (Char->GetMesh())
		{
			Char->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			Char->GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
			Char->GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true, true);
		}
    }

}

void ASL_CharacterBase::Revive()
{
	if (CurrentState == EPlayerState::Alive) return;
	CurrentState = EPlayerState::Alive;
	// 2.移除布娃娃系统,恢复正常的碰撞
	if (ACharacter* Char = Cast<ACharacter>(this))
	{
		if (UCapsuleComponent* comp = Char->GetCapsuleComponent())
		{
			comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		if (Char->GetMesh())
		{
			Char->GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
			Char->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Char->GetMesh()->SetAllBodiesSimulatePhysics(false);
		}
	}
}

bool ASL_CharacterBase::IsAlive() const
{
	RETURN_VALUE_IF_TRUE(AbilitySystemComp == nullptr, false);
	FGameplayTagContainer currentTags;
	AbilitySystemComp->GetOwnedGameplayTags(currentTags);
	return currentTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("State.Alive")));
}

bool ASL_CharacterBase::IsDie() const
{
	RETURN_VALUE_IF_TRUE(AbilitySystemComp == nullptr, false);
	FGameplayTagContainer currentTags;
	AbilitySystemComp->GetOwnedGameplayTags(currentTags);
	return currentTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("State.Dead")));

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

/************************************************************************/
/*                    IWeaponAccessory_IF 接口实现                        */
/************************************************************************/
ASL_WeaponBase* ASL_CharacterBase::GetLeftHandWeapon() const
{
    if (!EquipmentCmp)
    {
        UE_LOG(LogTemp, Warning, TEXT("ASL_CharacterBase::GetLeftHandWeapon - EquipmentCmp is null"));
        return nullptr;
    }
    return EquipmentCmp->GetCurrentLeftHandWeapon();
}

ASL_WeaponBase* ASL_CharacterBase::GetRightHandWeapon() const
{
    if (!EquipmentCmp)
    {
        UE_LOG(LogTemp, Warning, TEXT("ASL_CharacterBase::GetRightHandWeapon - EquipmentCmp is null"));
        return nullptr;
    }
    return EquipmentCmp->GetCurrentRightHandWeapon();
}

ASL_WeaponBase* ASL_CharacterBase::GetWeaponByHand(int32 HandIndex) const
{
    if (!EquipmentCmp)
    {
        UE_LOG(LogTemp, Warning, TEXT("ASL_CharacterBase::GetWeaponByHand - EquipmentCmp is null"));
        return nullptr;
    }
    return EquipmentCmp->GetWeaponByHand(HandIndex);
}




