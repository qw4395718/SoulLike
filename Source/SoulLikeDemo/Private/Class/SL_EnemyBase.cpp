// Private/Class/SL_EnemyBase.cpp

#include "SL_EnemyBase.h"
#include <BehaviorTree/BehaviorTree.h>
#include <Manager/DataTableManager.h>
#include <EnemyConfigInfoTable.h>
#include <GameFramework/Character.h>
#include <GameplayAbilitySpec.h>
#include <Components/CapsuleComponent.h>
#include "Components/SkeletalMeshComponent.h"
#include <BehaviorTree/BlackboardData.h>
#include "SL_AbilitySystemComponent.h"
#include <SL_StatusAttributeSet.h>
#include <Manager/GlobalDelegatesManager.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <Kismet/GameplayStatics.h>
#include <SL_EnemyAIController.h>
#include <SL_GameplayAbilityNPCBase.h>
#include <SL_WeaponBase.h>

ASL_EnemyBase::ASL_EnemyBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false; 
	CurrentState = EEnemyState::Alive;
	CurrentTarget = nullptr;
	// 默认武器派生类
	WeaponBaseClass = ASL_WeaponBase::StaticClass();

	/************************************************************************/
	/*                                AI组件相关                                      */
	/************************************************************************/
	AIControllerClass = ASL_EnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	/************************************************************************/
	/*                                GAS组件相关                                      */
	/************************************************************************/
	 // ASC-核心功能组件
	AbilitySystemComp = CreateDefaultSubobject<USL_AbilitySystemComponent>(TEXT("AbilitySystem"));
	// AS(CharacterCombatState)
	StatusAttributeSet = CreateDefaultSubobject<USL_StatusAttributeSet>(TEXT("StatusSet"));

}

void ASL_EnemyBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASL_EnemyBase::InitializeEnemy(int32 EnemyID)
{
	// TODO: 从EnemyDataTable读取敌人配置
	// 例如：血量、攻击力、AI行为树等
	// 从DataTableManager获取敌人配置表
	UDataTableManager* TableManager = UDataTableManager::Get(this);
	if (!TableManager)
	{
		UE_LOG(LogTemp, Error, TEXT("ASL_EnemyBase::InitializeEnemy - DataTableManager not found"));
		return;
	}

	UEnemyConfigInfoTable* EnemyTable = Cast<UEnemyConfigInfoTable>(TableManager->GetDataTable(EDataTableType::DT_EnemyConfigInfo));
	if (!EnemyTable)
	{
		UE_LOG(LogTemp, Error, TEXT("ASL_EnemyBase::InitializeEnemy - EnemyConfigInfoTable not found"));
		return;
	}

	FEnemyConfigInfo Config;
	if (!EnemyTable->GetEnemyConfig(EnemyID, Config))
	{
		UE_LOG(LogTemp, Error, TEXT("ASL_EnemyBase::InitializeEnemy - EnemyID=%d not found in config table"), EnemyID);
		return;
	}

	// 保存配置
	EnemyConfig = Config;

	// 应用配置
	ApplyEnemyConfig(Config);

	// 绑定死亡委托
	BindGASDeathEvent();

	UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::InitializeEnemy - Initialized enemy: ID=%d, Name=%s, Type=%d"),
		EnemyID, *Config.EnemyName.ToString(), (int32)Config.EnemyType);
}

ASL_EnemyAIController* ASL_EnemyBase::GetEnemyAIController() const
{
    return Cast<ASL_EnemyAIController>(GetController());
}

void ASL_EnemyBase::Die()
{
	if (CurrentState == EEnemyState::Dead) return;

	CurrentState = EEnemyState::Dead;

	// === 修正：先广播死亡事件（WaveManager会收到这个） ===
	OnEnemyDied.Broadcast();

	// === 新增：禁用碰撞 ===
	SetActorEnableCollision(false);

	// === 新增：播放死亡动画（如果有） ===
	if (GetMesh())
	{
		// 可以在这里触发布娃娃或者死亡蒙太奇
		// 但为了避免与GAS的布娃娃逻辑冲突，这里只做简单的禁用
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// === 修正：延迟销毁（给死亡动画和掉落物品时间） ===
	FTimerHandle DestroyHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyHandle, FTimerDelegate::CreateLambda([this]()
		{
			// 延迟销毁敌人
			SetLifeSpan(2.0f);  // 2秒后自动销毁
		}), 1.0f, false);

	UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::Die - Enemy %s died"), *GetName());
}

void ASL_EnemyBase::BindGASDeathEvent()
{
	// 绑定GAS的死亡委托
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		// 防止重复绑定
		if (!OnCharacterDiedHandle.IsValid())
		{
			OnCharacterDiedHandle = DelegateMgr->OnCharacterDied.AddUObject(this, &ASL_EnemyBase::OnGASCharacterDied);
		}
	}
}

void ASL_EnemyBase::OnGASCharacterDied(AActor* DiedActor, AActor* KillerActor)
{
	// 检查是否是自己的死亡事件
	if (DiedActor != this) return;

	UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::OnGASCharacterDied - Enemy %s died"), *GetName());

	// 确保执行死亡逻辑
	if (CurrentState != EEnemyState::Dead)
	{
		Die();
	}
}

void ASL_EnemyBase::ApplyEnemyConfig(const FEnemyConfigInfo& Config)
{
	// 1. 设置属性（通过GAS）
	if (AbilitySystemComp)
	{
		// 设置初始血量
		if (USL_StatusAttributeSet* StatusSet = const_cast<USL_StatusAttributeSet*>(AbilitySystemComp->GetSet<USL_StatusAttributeSet>()))
		{
			// 通过GAS的Attribute设置初始值
			StatusSet->InitHealth(Config.BaseHealth);
			StatusSet->InitMaxHealth(Config.BaseHealth);

			// 设置其他属性
			// 注意：Stamina、Attack、Defense等需要额外配置GE
		}
	}

	// 2. 修改碰撞体大小
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCapsuleSize(Config.CapsuleRadius, Config.CapsuleHalfHeight);
	}

	// 3. 设置模型缩放
	if (GetMesh())
	{
		GetMesh()->SetRelativeScale3D(FVector(Config.MeshScale));
	}

	// 4. 加载外观
	LoadEnemyAppearance(Config);

	// 4.1 初始化武器
	SpawnEnemyWeapons(Config);

	// 5. 初始化AI
	InitializeEnemyAI(Config);

	// 6. 授予GAS能力
	if (AbilitySystemComp && Config.GrantedAbilities.Num() > 0)
	{
		for (TSubclassOf<UGameplayAbility> AbilityClass : Config.GrantedAbilities)
		{
			if (AbilityClass)
			{
 				// UE4.26: 通过 GiveAbility 授予能力
                FGameplayAbilitySpec Spec(AbilityClass, 1, INDEX_NONE, this);
                
                // 设置能力的SourceTag（用于后续通过Tag查找）
                if (USL_GameplayAbilityNPCBase* DefaultAbility = Cast<USL_GameplayAbilityNPCBase>(AbilityClass->GetDefaultObject()))
                {
                    if (DefaultAbility->ActivationTag.IsValid())
                    {
                        // 通过DynamicAbilityTags标记能力
                        Spec.DynamicAbilityTags.AddTag(DefaultAbility->ActivationTag);
                        
                        UE_LOG(LogTemp, Log, TEXT("Enemy %s: Granting ability %s with tag %s"), 
                            *GetName(), *AbilityClass->GetName(), *DefaultAbility->ActivationTag.ToString());
                    }
                }

                AbilitySystemComp->GiveAbility(Spec);			
			}
		}
	}

	// 7. 绑定GAS死亡事件
	BindGASDeathEvent();
}

void ASL_EnemyBase::LoadEnemyAppearance(const FEnemyConfigInfo& Config)
{
	// 加载骨骼网格体
	if (!Config.SkeletalMesh.IsNull())
	{
		USkeletalMesh* EnemyMesh = Config.SkeletalMesh.LoadSynchronous();
		if (EnemyMesh && GetMesh())
		{
			GetMesh()->SetSkeletalMesh(EnemyMesh);
			GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
		}
	}

	// 加载动画蓝图
	if (!Config.AnimBlueprint.IsNull())
	{
		UAnimBlueprint* AnimBP = Config.AnimBlueprint.LoadSynchronous();
		if (AnimBP && GetMesh())
		{
			GetMesh()->SetAnimInstanceClass(AnimBP->GetAnimBlueprintGeneratedClass());
		}
	}
}

void ASL_EnemyBase::InitializeEnemyAI(const FEnemyConfigInfo& Config)
{
	// 设置AI感知范围
	PerceptionRange = Config.PerceptionRange;
	AttackRange = Config.AttackRange;

	// 加载行为树
	if (!Config.BehaviorTree.IsNull())
	{
		BehaviorTree = Config.BehaviorTree.LoadSynchronous();
	}

	// 加载黑板
	if (!Config.BlackboardData.IsNull())
	{
		BlackboardData = Config.BlackboardData.LoadSynchronous();
	}

	UE_LOG(LogTemp, Verbose, TEXT("ASL_EnemyBase::InitializeEnemyAI - PerceptionRange=%.2f, AttackRange=%.2f"),
		PerceptionRange, AttackRange);
}

// ===== 新增：生成敌人武器 =====
void ASL_EnemyBase::SpawnEnemyWeapons(const FEnemyConfigInfo& Config)
{
    // 生成左手武器
    if (Config.LeftHandWeaponID > 0)
    {
        LeftHandWeapon = SpawnWeaponByID(Config.LeftHandWeaponID);
        if (LeftHandWeapon)
        {
            // 附加到左手插槽
            FName SocketName = (Config.LeftHandSocketName != NAME_None) 
                ? Config.LeftHandSocketName 
                : FName("Weapon_L");
            
            LeftHandWeapon->AttachToComponent(GetMesh(), 
                FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
            
            // 设置缩放
            LeftHandWeapon->SetActorScale3D(FVector(Config.LeftHandWeaponScale));
        }
    }

    // 生成右手武器
    if (Config.RightHandWeaponID > 0)
    {
        RightHandWeapon = SpawnWeaponByID(Config.RightHandWeaponID);
        if (RightHandWeapon)
        {
            FName SocketName = (Config.RightHandSocketName != NAME_None) 
                ? Config.RightHandSocketName 
                : FName("Weapon_R");
            
            RightHandWeapon->AttachToComponent(GetMesh(), 
                FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
            
            RightHandWeapon->SetActorScale3D(FVector(Config.RightHandWeaponScale));
        }
    }
}

// ===== 新增：根据武器ID派生武器实例 =====
ASL_WeaponBase* ASL_EnemyBase::SpawnWeaponByID(int32 WeaponID)
{
    if (!GetWorld() || WeaponID <= 0) return nullptr;

	ACharacter* OwningCharacter = Cast<ACharacter>(this);
	if (!OwningCharacter) return nullptr;

	// 创建武器实例
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = OwningCharacter;
	SpawnParams.Instigator = Cast<APawn>(OwningCharacter);
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASL_WeaponBase* Weapon = GetWorld()->SpawnActorDeferred<ASL_WeaponBase>(
		WeaponBaseClass,
		FTransform::Identity,
		OwningCharacter,
		Cast<APawn>(OwningCharacter),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn
		);

	if (Weapon)
	{
		// 初始化武器
		FName SocketName;
		if (EnemyConfig.LeftHandWeaponID != 0 && EnemyConfig.RightHandWeaponID != 0)
		{
			SocketName = EnemyConfig.TwoHandSocketName;
		}
		else if (EnemyConfig.LeftHandWeaponID != 0)
		{
			SocketName = EnemyConfig.LeftHandSocketName;
		}
		else
		{
			SocketName = EnemyConfig.RightHandSocketName;
		}
		Weapon->InitializeWeaponWithID(WeaponID, SocketName);
		Weapon->SetOwner(OwningCharacter);
		Weapon->FinishSpawning(FTransform::Identity);

		UE_LOG(LogTemp, Verbose, TEXT("USL_EquipmentComponent::SpawnWeaponByID - Spawned weapon ID=%d"), WeaponID);
	}

	return Weapon;
}

// IWeaponAccessory_IF 接口实现
ASL_WeaponBase* ASL_EnemyBase::GetLeftHandWeapon() const
{
    return LeftHandWeapon;
}

ASL_WeaponBase* ASL_EnemyBase::GetRightHandWeapon() const
{
    return RightHandWeapon;
}

ASL_WeaponBase* ASL_EnemyBase::GetWeaponByHand(int32 HandIndex) const
{
    // 约定：0=左手, 1=右手
    if (HandIndex == 0) return LeftHandWeapon;
    if (HandIndex == 1) return RightHandWeapon;
    return nullptr;
}

// IAbilitySystemInterface 接口实现
UAbilitySystemComponent* ASL_EnemyBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}