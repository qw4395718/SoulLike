// Fill out your copyright notice in the Description page of Project Settings.


#include "SL_CharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Components/CapsuleComponent.h"
#include "WeaponAnimNotify_IF.h"
#include "SL_AbilitySystemComponent.h"
#include <SL_StatusAttributeSet.h>
#include <Abilities/GameplayAbilityTypes.h>
#include <Components/WidgetComponent.h>
#include <UIManagerSubsystem.h>
#include <SL_PlayerStateBase.h>
#include <Engine/PackageMapClient.h>
#include <GameplayTagContainer.h>
#include <SL_ComboManagerComponent.h>
#include <Manager/DataTableManager.h>
#include <ClassConfigInfoTable.h>
#include <SoulLikeGameGlobal.h>
#include <Manager/GlobalDelegatesManager.h>
#include <Components/CapsuleComponent.h>
#include <GameFramework/CharacterMovementComponent.h>
#include "Components/SkeletalMeshComponent.h"
#include <Engine/NetConnection.h>
#include "Materials/MaterialInstanceDynamic.h"

DEFINE_LOG_CATEGORY(SL_CharacterBase);


void ASL_CharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASL_CharacterBase, CurrentIdentity);
	DOREPLIFETIME(ASL_CharacterBase, PhantomData);
	DOREPLIFETIME(ASL_CharacterBase, bCanInteractWithWorld);
	DOREPLIFETIME(ASL_CharacterBase, bCanBeDamagedByWorld);
}

ASL_CharacterBase::ASL_CharacterBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;
	CurrentIdentity = ECharacterIdentity::Normal;
	bCanInteractWithWorld = true;
	bCanBeDamagedByWorld = true;
	PhantomMaterialOverride = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/SoulLikeDemo/Materials/MI_Phantom.MI_Phantom")));


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

void ASL_CharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// 初始化角色(包含组件)
	InitializeCharacter();
}

void ASL_CharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// 为ASC设置持有者和化身
	if (AbilitySystemComp)
	{
		AbilitySystemComp->InitAbilityActorInfo(this, this);
	}

	if (StatusAttributeSet)
	{
		StatusAttributeSet->SetOwningActor(this);
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

	// 锁定旋转控制
	if (LockOnCmp && LockOnCmp->IsLocked() && Controller)
	{
		FVector LockDir = LockOnCmp->GetLockDirection();
		if (!LockDir.IsNearlyZero())
		{
			FRotator TargetRotation = LockDir.Rotation();
			FRotator CurrentControlRot = Controller->GetControlRotation();
			FRotator NewControlRot(CurrentControlRot.Pitch, TargetRotation.Yaw, CurrentControlRot.Roll);
			Controller->SetControlRotation(NewControlRot);
		}
	}
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



void ASL_CharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (ASL_PlayerStateBase* PS = GetPlayerState<ASL_PlayerStateBase>())
	{
		int32 ClassID = PS->GetClassID();
		if (ClassID > 0)
		{
			ApplyClassAppearance(ClassID);
		}
	}
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
		CombatCmp->RegisterComponent();
		CombatCmp->InitCombatComponentInfo(this, TEXT("/Game/SoulLikeDemo/Anim/AM_Character_Hit.AM_Character_Hit") ,0,true);
	}

	if (EquipmentCmp == nullptr && true)
	{
		EquipmentCmp = NewObject<USL_EquipmentComponent>(this);
		EquipmentCmp->RegisterComponent();
	}

	if (HealthCmp == nullptr && true)
	{
		HealthCmp = NewObject<USL_HealthComponent>(this);
		HealthCmp->RegisterComponent();
		float HealthMax = 100.0f;
		HealthCmp->InitHealthInfo(HealthMax);
	}

	if (InventoryCmp == nullptr && true)
	{
		InventoryCmp = NewObject<USL_InventoryComponent>(this);
		InventoryCmp->RegisterComponent();
	}

	if (StaminaCmp == nullptr && true)
	{
		StaminaCmp = NewObject<USL_StaminaComponent>(this);
		StaminaCmp->RegisterComponent();
		//StaminaCmp->InitializeStaminaComponent();
	}

	if (LockOnCmp == nullptr && true)
	{
		LockOnCmp = NewObject<USL_LockOnComponent>(this);
		LockOnCmp->RegisterComponent();
		//LockOnCmp->InitializeLockOnComponent();
	}

	if (StateCmp == nullptr && true)
	{
		StateCmp = NewObject<USL_StateComponent>(this);
		StateCmp->RegisterComponent();
	}

	if (MovementCmp == nullptr && true)
	{
		MovementCmp = NewObject<USL_MovementComponent>(this);
		MovementCmp->RegisterComponent();
		//MovementCmp->InitMovemenetInfo(true,"");
	}

	if (ComboManagerCmp == nullptr && true)
	{
		ComboManagerCmp = NewObject<USL_ComboManagerComponent>(this);
		ComboManagerCmp->RegisterComponent();
	}


}

UAbilitySystemComponent* ASL_CharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}

// ==================== 属性值访问器 ====================

float ASL_CharacterBase::GetCurrentHealth() const
{
	return StatusAttributeSet ? StatusAttributeSet->GetHealth() : 0.0f;
}

float ASL_CharacterBase::GetMaxHealth() const
{
	return StatusAttributeSet ? StatusAttributeSet->GetMaxHealth() : 0.0f;
}

float ASL_CharacterBase::GetCurrentStamina() const
{
	return StatusAttributeSet ? StatusAttributeSet->GetStamina() : 0.0f;
}

float ASL_CharacterBase::GetMaxStamina() const
{
	return StatusAttributeSet ? StatusAttributeSet->GetMaxStamina() : 0.0f;
}

void ASL_CharacterBase::SetClassID(int32 InPlayerClassID)
{
	PlayerClassID = InPlayerClassID;
	// 此处临时存档,需要等gamemodebase 重写RestartPlayer后才能走begin正常初始化的流程
	InitCharacterWithClassID(PlayerClassID);
}

// 客户端视觉初始化（由 OnRep_PlayerClassID 触发）
void ASL_CharacterBase::ApplyClassAppearance(int32 InClassID)
{
	// 加载职业配置以获取视觉数据（武器装备、动画等）
	UDataTableManager* TableManager = UDataTableManager::Get(this);
	if (!TableManager) return;

	UClassConfigInfoTable* ClassTable = Cast<UClassConfigInfoTable>(
		TableManager->GetDataTable(EDataTableType::DT_ClassConfigInfo));
	if (!ClassTable) return;

	FClassConfigInfo Config;
	if (!ClassTable->GetClassConfig(InClassID, Config))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyClassAppearance - ClassConfig not found for ClassID=%d"), InClassID);
		return;
	}

	ClassConfig = Config;
	PlayerClassID = InClassID;

	// 客户端装备初始化（仅缓存配置，武器实例由服务端生成的复制同步过来）
	if (EquipmentCmp)
	{
		EquipmentCmp->InitializeWithConfig(Config);
	}

	// TODO: 在此处添加更多客户端视觉初始化
	// - 动画蓝图切换
	// - 材质覆盖
	// - 粒子效果等

	UE_LOG(LogTemp, Log, TEXT("ASL_CharacterBase::ApplyClassAppearance - Applied class %d appearance"), InClassID);
}

// ==================== GAS能力授予 ====================

void ASL_CharacterBase::GrantAbilities(const TArray<TSubclassOf<UGameplayAbility>>& InAbilities)
{
	if (!AbilitySystemComp || !HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("ASL_CharacterBase::GrantAbilities - Skipped (no ASC or no authority)"));
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : InAbilities)
	{
		if (!AbilityClass) continue;

		// 检查是否已授予过，避免重复
		if (AbilitySystemComp->FindAbilitySpecFromClass(AbilityClass))
		{
			UE_LOG(LogTemp, Verbose, TEXT("GrantAbilities - Ability %s already granted, skipping"), *AbilityClass->GetName());
			continue;
		}

		FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
		AbilitySystemComp->GiveAbility(Spec);
	}
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
	PlayerClassID = InPlayerClassID;

	// 应用配置
	ApplyEnemyConfig(ClassConfig);

	BindGASDeathEvent();
	BindGASReviveEvent();

	// 检测玩家当前是否是已死亡状态
	if (IsDie() && StatusAttributeSet)
	{
		StatusAttributeSet->OnCharacterReLive(this);
	}

	// Todo:切换职业的时候需要广播,改变能力值和Tag
}

void ASL_CharacterBase::ApplyEnemyConfig(const FClassConfigInfo& Config)
{
	// 1. 设置属性（通过GAS）
	if (AbilitySystemComp)
	{
		AbilitySystemComp->SetAliveTag();

		if (StatusAttributeSet)
		{
			// 通过GAS的Attribute设置初始值
			StatusAttributeSet->InitHealthAS(0, Config.BaseHealth);
			StatusAttributeSet->InitStaminaAS(0, Config.BaseStamina);
			// StatusSet->InitAttack(Config.BaseAttack);
			// StatusSet->InitDefense(Config.BaseDefense);
			// 设置其他属性
			// 注意：Stamina、Attack、Defense等需要额外配置GE
		}
	}
	// 2. 设置队伍归属
	SetTeamID(Config.TeamID);

	if (EquipmentCmp)
	{
		EquipmentCmp->InitializeWithClassID(PlayerClassID);
	}

	// 3. 将职业配置中的道具注册到背包
	if (InventoryCmp)
	{
		for (const FSlotItemInfo& SlotItem : Config.SlotItems)
		{
			if (SlotItem.ItemID != NAME_None && SlotItem.InitialCount > 0)
			{
				InventoryCmp->AddItemByID(SlotItem.ItemID, SlotItem.InitialCount);
				UE_LOG(LogTemp, Log, TEXT("ASL_CharacterBase::ApplyEnemyConfig - Added item %s x%d to inventory"),
					*SlotItem.ItemID.ToString(), SlotItem.InitialCount);
			}
		}
	}

	// 4. 授予GAS能力（可配置在数据表中）
	GrantAbilities(Config.GrantedAbilities);

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

void ASL_CharacterBase::BindGASReviveEvent()
{
	// 绑定GAS的复活
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		// 防止重复绑定
		if (!OnCharacterLivedHandle.IsValid())
		{
			OnCharacterLivedHandle = DelegateMgr->OnCharacterRevived.AddUObject(this, &ASL_CharacterBase::OnGASCharacterRevive);
		}
	}
}

void ASL_CharacterBase::OnGASCharacterDied(AActor* DiedActor, AActor* KillerActor)
{
	// 检查是否是自己的死亡事件
	if (DiedActor != this) return;

	UE_LOG(LogTemp, Log, TEXT("ASL_CharacterBase::OnGASCharacterDied - %s died (identity=%d)"),
		*GetName(), (int32)CurrentIdentity);

	// 确保执行死亡逻辑
	if (CurrentState != EPlayerState::Dead)
	{
		Die();
	}

	// 灵体死亡后自动遣返
	if (CurrentIdentity == ECharacterIdentity::Phantom)
	{
		Repatriate(EReturnReason::PhantomDied);
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

	RagDollStart();
}

void ASL_CharacterBase::RagDollStart()
{
	if (ACharacter* Char = Cast<ACharacter>(this))
	{
		if (UCharacterMovementComponent* comp = Char->GetCharacterMovement())
		{
			comp->SetMovementMode(MOVE_None);
		}

		if (UCapsuleComponent* comp = Char->GetCapsuleComponent())
		{
			comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (USkeletalMeshComponent* comp = Char->GetMesh())
		{
			comp->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
			comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			comp->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true, true);
			if (UAnimInstance* AimInstance = comp->GetAnimInstance())
			{
				AimInstance->StopAllMontages(0.2f);
			}
		}
	}
}

void ASL_CharacterBase::Revive()
{
	if (CurrentState == EPlayerState::Alive) return;
	CurrentState = EPlayerState::Alive;

	RagDollEnd();
}

void ASL_CharacterBase::RagDollEnd()
{
	// 移除布娃娃系统,恢复正常的碰撞
	if (ACharacter* Char = Cast<ACharacter>(this))
	{
		if (UCharacterMovementComponent* comp = Char->GetCharacterMovement())
		{
			comp->SetMovementMode(MOVE_Walking);
		}
		if (UCapsuleComponent* comp = Char->GetCapsuleComponent())
		{
			comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
		if (USkeletalMeshComponent* comp = Char->GetMesh())
		{
			comp->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
			comp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			comp->SetAllBodiesSimulatePhysics(false);
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

void ASL_CharacterBase::Destroyed()
{

}

int32 ASL_CharacterBase::GetTeamID() const
{
	return TeamID;
}

void ASL_CharacterBase::SetTeamID(int32 InTeamID)
{
	TeamID = InTeamID;
}

/************************************************************************/
/*                               网络RPC                                */
/************************************************************************/

void ASL_CharacterBase::BroadcastDamageFloatingText(const FDamageFloatingTextData& InData)
{
	Multicast_OnDamageFloatingText(InData);
}

void ASL_CharacterBase::BroadcastCharacterDeath(AActor* InDeadActor, AActor* InInstigator)
{
	// 所有客户端触发 GlobalDelegatesManager 广播
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		DelegateMgr->OnCharacterDied.Broadcast(InDeadActor, InInstigator);
	}
}

void ASL_CharacterBase::Multicast_OnDamageFloatingText_Implementation(const FDamageFloatingTextData& InData)
{
	// 客户端本地触发 GlobalDelegatesManager 广播
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		DelegateMgr->BroadcastDamageFloatingText(InData);
	}
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


/************************************************************************/
/*                             身份相关方法                               */
/************************************************************************/

void ASL_CharacterBase::SetIdentity(ECharacterIdentity InIdentity)
{
	CurrentIdentity = InIdentity;

	if (HasAuthority() && InIdentity == ECharacterIdentity::Phantom)
	{
		bCanInteractWithWorld = false;
		bCanBeDamagedByWorld = true;
	}
}

void ASL_CharacterBase::ApplyPhantomData(const FPhantomData& InData)
{
	PhantomData = InData;
	CurrentIdentity = ECharacterIdentity::Phantom;

	RebuildAppearance();
	ApplyTranslucentEffect();
	ApplyPhantomRestrictions();

	UE_LOG(LogTemp, Log, TEXT("CharacterBase: Applied phantom data for %s (mesh=%s)"),
		*PhantomData.OwnerName, *PhantomData.CharacterMeshPath);
}

void ASL_CharacterBase::SetInteractionEnabled(bool bEnabled)
{
	bCanInteractWithWorld = bEnabled;
}

void ASL_CharacterBase::Repatriate(EReturnReason InReason)
{
	if (!HasAuthority()) return;

	UE_LOG(LogTemp, Log, TEXT("CharacterBase: Repatriating %s (reason=%d, server=%s:%d)"),
		*PhantomData.OwnerName, (int32)InReason,
		*PhantomData.PlacerIP, PhantomData.PlacerPort);

	// 通知客户端返回自己的世界
	if (!PhantomData.PlacerIP.IsEmpty() && PhantomData.PlacerPort > 0)
	{
		FString ReturnURL = FString::Printf(TEXT("%s:%d"),
			*PhantomData.PlacerIP, PhantomData.PlacerPort);

		APlayerController* PC = GetController<APlayerController>();
		if (PC)
		{
			PC->ClientTravel(ReturnURL, TRAVEL_Absolute);
		}
	}

	SetLifeSpan(1.0f);
}

void ASL_CharacterBase::RebuildAppearance()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;

	if (!PhantomData.CharacterMeshPath.IsEmpty())
	{
		USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(
			StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *PhantomData.CharacterMeshPath));
		if (LoadedMesh)
			MeshComp->SetSkeletalMesh(LoadedMesh);
	}

	if (!PhantomData.AnimBlueprintPath.IsEmpty())
	{
		UClass* AnimBPClass = Cast<UClass>(
			StaticLoadObject(UClass::StaticClass(), nullptr, *PhantomData.AnimBlueprintPath));
		if (AnimBPClass)
			MeshComp->SetAnimInstanceClass(AnimBPClass);
	}

	PhantomMaterials.Empty();
	for (int32 i = 0; i < PhantomData.MaterialPaths.Num(); i++)
	{
		const FString& MatPath = PhantomData.MaterialPaths[i];
		if (!MatPath.IsEmpty())
		{
			UMaterialInterface* Mat = Cast<UMaterialInterface>(
				StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MatPath));
			if (Mat)
				MeshComp->SetMaterial(i, Mat);
		}
	}
}

void ASL_CharacterBase::ApplyTranslucentEffect()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp) return;

	UMaterialInterface* TranslucentMat = PhantomMaterialOverride.LoadSynchronous();
	if (!TranslucentMat)
	{
		TranslucentMat = UMaterial::GetDefaultMaterial(MD_Surface);
		if (!TranslucentMat) return;
	}

	PhantomMaterials.Empty();
	for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
	{
		UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(TranslucentMat, this);
		if (DynMat)
		{
			DynMat->SetScalarParameterValue(FName("Opacity"), 0.6f);
			DynMat->SetScalarParameterValue(FName("GlowIntensity"), 0.3f);
			MeshComp->SetMaterial(i, DynMat);
			PhantomMaterials.Add(DynMat);
		}
	}

	MeshComp->SetCastShadow(false);
	MeshComp->bReceivesDecals = false;
}

void ASL_CharacterBase::ApplyPhantomRestrictions()
{
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetSimulatePhysics(false);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ASL_CharacterBase::OnRep_PhantomData()
{
	
}


void ASL_CharacterBase::OnRep_CurrentIdentity()
{
	if (CurrentIdentity == ECharacterIdentity::Phantom)
	{
		if (!PhantomData.CharacterMeshPath.IsEmpty())
		{
			RebuildAppearance();
			ApplyTranslucentEffect();
		}
	}
}

