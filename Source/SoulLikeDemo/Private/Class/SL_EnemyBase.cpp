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
#include <Components/WidgetComponent.h>
#include <Animation/AnimBlueprint.h>
#include <Animation/AnimBlueprintGeneratedClass.h>
#include "Net/UnrealNetwork.h"
#include <PartBreakDataTable.h>

ASL_EnemyBase::ASL_EnemyBase()
{
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

	// 
	// 网络复制
	bReplicates = true;

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

void ASL_EnemyBase::BeginPlay()
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

	// 保存 EnemyID 用于复制到客户端（晚加入时重新初始化）
	NetEnemyID = EnemyID;

	// 应用配置
	ApplyEnemyConfig(Config);

	// 绑定死亡委托
	BindGASDeathEvent();

	// 初始化部位破坏
	InitializePartBreak();

	UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::InitializeEnemy - Initialized enemy: ID=%d, Name=%s, Type=%d"),
		EnemyID, *Config.EnemyName.ToString(), (int32)Config.EnemyType);
}

bool ASL_EnemyBase::IsAlive() const
{
	RETURN_VALUE_IF_TRUE(AbilitySystemComp == nullptr, false);
	FGameplayTagContainer currentTags;
	AbilitySystemComp->GetOwnedGameplayTags(currentTags);
	return currentTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("State.Alive")));
}

bool ASL_EnemyBase::IsDie() const
{
	RETURN_VALUE_IF_TRUE(AbilitySystemComp == nullptr, false);
	FGameplayTagContainer currentTags;
	AbilitySystemComp->GetOwnedGameplayTags(currentTags);
	return currentTags.HasTag(FGameplayTag::RequestGameplayTag(TEXT("State.Dead")));
}

/************************************************************************/
/*                    ICombatEventDisplay_IF 接口实现                        */
/************************************************************************/

void ASL_EnemyBase::BroadcastDamageFloatingText(const FDamageFloatingTextData& InData)
{
	Multicast_OnDamageFloatingText(InData);
}

void ASL_EnemyBase::BroadcastCharacterDeath(AActor* InDeadActor, AActor* InInstigator)
{
	Multicast_OnCharacterDeath(InDeadActor, InInstigator);
}

/************************************************************************/
/*                               网络RPC                                */
/************************************************************************/

void ASL_EnemyBase::Multicast_OnDamageFloatingText_Implementation(const FDamageFloatingTextData& InData)
{
	// 所有客户端触发 GlobalDelegatesManager 广播
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		DelegateMgr->BroadcastDamageFloatingText(InData);
	}
}

void ASL_EnemyBase::Multicast_OnCharacterDeath_Implementation(AActor* InDeadActor, AActor* InInstigator)
{
	// 所有客户端触发 GlobalDelegatesManager 广播
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		DelegateMgr->OnCharacterDied.Broadcast(InDeadActor, InInstigator);
	}
}

/************************************************************************/
/*                              部位破坏实现                                       */
/************************************************************************/

void ASL_EnemyBase::InitializePartBreak()
{
	PartStates.Reset();

	// 优先从数据表加载部位破坏配置
	if (NetEnemyID > 0)
	{
		if (UDataTableManager* DTManager = UDataTableManager::Get(this))
		{
			if (UPartBreakDataTable* PartTable = Cast<UPartBreakDataTable>(DTManager->GetDataTable(EDataTableType::DT_PartBreakConfig)))
			{
				TArray<FPartBreakConfig> TableConfigs;
				if (PartTable->GetPartBreakConfigs(NetEnemyID, TableConfigs) && TableConfigs.Num() > 0)
				{
					PartBreakConfigs = TableConfigs;
					UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::InitializePartBreak - Loaded %d parts from table for EnemyID=%d"),
						TableConfigs.Num(), NetEnemyID);
				}
			}
		}
	}

	// 从 PartBreakConfigs 初始化运行时状态
	for (const FPartBreakConfig& Config : PartBreakConfigs)
	{
		FPartBreakState State;
		State.PartID = Config.PartID;
		PartStates.Add(State);
	}
}

void ASL_EnemyBase::AccumulatePartDamage(FName InBoneName, float InDamage)
{
	if (!HasAuthority()) return;
	FPartBreakConfig* Config = FindPartConfig(InBoneName);
	if (!Config) return;
	FPartBreakState* State = PartStates.FindByPredicate([&](const FPartBreakState& S){
		return S.PartID == Config->PartID;
	});
	if (!State || State->bIsFullyBroken) return;
	State->AccumulatedDamage += InDamage;
	CheckPartBreak(*Config, *State);
}

void ASL_EnemyBase::CheckPartBreak(const FPartBreakConfig& Config, FPartBreakState& State)
{
	float RequiredDamage = Config.BreakThreshold * (State.CurrentBreakLevel + 1);
	if (State.AccumulatedDamage >= RequiredDamage)
	{
		ApplyPartBreak(Config, State, State.CurrentBreakLevel + 1);
	}
}

void ASL_EnemyBase::ApplyPartBreak(const FPartBreakConfig& Config, FPartBreakState& State, int32 NewLevel)
{
	State.CurrentBreakLevel = NewLevel;
	if (NewLevel >= Config.BreakLevelCount)
		State.bIsFullyBroken = true;

	// 生成部位破坏掉落物
	SpawnBreakDrops(Config.DropItems);

	Multicast_OnPartBreak(Config.PartID, NewLevel);
	OnPartBroken.Broadcast(Config.PartID, NewLevel);
}

FPartBreakConfig* ASL_EnemyBase::FindPartConfig(FName InBoneName)
{
	FString BoneStr = InBoneName.ToString();
	for (FPartBreakConfig& Config : PartBreakConfigs)
	{
		if (BoneStr.StartsWith(Config.PartID.ToString()))
			return &Config;
	}
	return nullptr;
}

void ASL_EnemyBase::Multicast_OnPartBreak_Implementation(FName PartID, int32 BreakLevel)
{
	FPartBreakConfig* Config = FindPartConfig(PartID);
	if (!Config) return;

	// 通过 GlobalDelegatesManager 广播部位破坏事件
	if (UGlobalDelegatesManager* DelegateMgr = UGlobalDelegatesManager::Get(this))
	{
		DelegateMgr->BroadcastPartBroken(this, PartID);
	}

	//// 显示屏幕通知
	//UNotifyMessageManager* NotifyMgr = NewObject<UNotifyMessageManager>(this);
	//if (NotifyMgr)
	//{
	//	FText Msg = FText::Format(
	//		NSLOCTEXT("PartBreak", "BreakFormat", "{0} 部位破坏！"),
	//		FText::FromName(PartID));
	//	NotifyMgr->ShowNotification(Msg);
	//}

	if (!Config->BrokenMaterial.IsNull())
	{
		GetMesh()->SetMaterialByName(PartID, Config->BrokenMaterial.LoadSynchronous());
	}
	if (!Config->BrokenMesh.IsNull() && BreakLevel >= 2)
	{
		GetMesh()->SetSkeletalMesh(Config->BrokenMesh.LoadSynchronous());
	}
}

/************************************************************************/
/*                              掉落物实现                                       */
/************************************************************************/

void ASL_EnemyBase::SpawnBreakDrops(const TArray<FDropItemInfo>& InDropItems)
{
	if (!HasAuthority() || !DropItemActorClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (const FDropItemInfo& DropInfo : InDropItems)
	{
		if (FMath::FRand() > DropInfo.DropProbability) continue;

		FVector DropLoc = GetActorLocation() + FMath::VRand() * FMath::FRandRange(50.0f, 150.0f);
		DropLoc.Z = GetActorLocation().Z;

		ASL_DropItemActor* Drop = GetWorld()->SpawnActor<ASL_DropItemActor>(
			DropItemActorClass, FTransform(DropLoc), SpawnParams);

		if (Drop)
		{
			Drop->InitializeDrop(DropInfo.ItemID, DropInfo.Count);
		}
	}
}

void ASL_EnemyBase::SpawnCarvePoint()
{
	if (!HasAuthority() || !DropItemActorClass || RemainingCarveCount <= 0) return;

	// 在尸体旁生成一个可剥取的光点
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector DropLoc = GetActorLocation() + FVector(0, 0, 50);

	ASL_DropItemActor* Drop = GetWorld()->SpawnActor<ASL_DropItemActor>(
		DropItemActorClass, FTransform(DropLoc), SpawnParams);

	if (Drop)
	{
		Drop->InitializeDrop(NAME_None, 0);
	}
}

void ASL_EnemyBase::CarveItem()
{
	if (!HasAuthority() || RemainingCarveCount <= 0) return;

	// 从配置中随机取一条
	if (EnemyConfig.CarveItems.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, EnemyConfig.CarveItems.Num() - 1);
		const FDropItemInfo& DropInfo = EnemyConfig.CarveItems[Index];

		if (FMath::FRand() <= DropInfo.DropProbability)
		{
			// TODO: 通过 InventoryComponent 添加物品到玩家背包
			UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::CarveItem - Carved %s x%d"),
				*DropInfo.ItemID.ToString(), DropInfo.Count);
		}
	}

	RemainingCarveCount--;

	if (RemainingCarveCount <= 0)
	{
		// 剥取完毕，销毁剥取光点
	}
}

void ASL_EnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASL_EnemyBase, NetEnemyID);
	DOREPLIFETIME(ASL_EnemyBase, LeftHandWeapon);
	DOREPLIFETIME(ASL_EnemyBase, RightHandWeapon);
	DOREPLIFETIME(ASL_EnemyBase, PartStates);
	DOREPLIFETIME(ASL_EnemyBase, RemainingCarveCount);
}

ASL_EnemyAIController* ASL_EnemyBase::GetEnemyAIController() const
{
    return Cast<ASL_EnemyAIController>(GetController());
}

void ASL_EnemyBase::Die()
{
	if (CurrentState == EEnemyState::Dead) return;

	CurrentState = EEnemyState::Dead;

	// 初始化剥取次数
	RemainingCarveCount = EnemyConfig.MaxCarveCount;
	// 生成剥取光点
	SpawnCarvePoint();

	// === 修正：先广播死亡事件（WaveManager会收到这个） ===
	OnEnemyDied.Broadcast();

	// === 新增：禁用碰撞 ===
	SetActorEnableCollision(false);

	// === 新增：播放死亡动画（如果有） ===
	if (GetMesh())
	{
		// 可以在这里触发布娃娃或者死亡蒙太奇
		// 但为了避免与GAS的布娃娃逻辑冲突，这里只做简单的禁用
		//GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		RagDollStart();
	}

	// === 修正：延迟销毁（给死亡动画和掉落物品时间） ===
	FTimerHandle DestroyHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyHandle, FTimerDelegate::CreateLambda([this]()
		{
			// 延迟销毁敌人
			SetLifeSpan(2.0f);  // 2秒后自动销毁
			// 附属销毁
			if (LeftHandWeapon)
			{
				LeftHandWeapon->SetLifeSpan(3.5f);
			}
			if (RightHandWeapon)
			{
				RightHandWeapon->SetLifeSpan(3.5f);
			}

		}), 1.0f, false);

	UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::Die - Enemy %s died"), *GetName());
}

void ASL_EnemyBase::RagDollStart()
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

int32 ASL_EnemyBase::GetTeamID() const
{
	return TeamID;
}

void ASL_EnemyBase::SetTeamID(int32 InTeamID)
{
	TeamID = InTeamID;
}

void ASL_EnemyBase::RagDollEnd()
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
		AbilitySystemComp->SetAliveTag();
		// 设置初始血量
		if (StatusAttributeSet)
		{
			// 通过GAS的Attribute设置初始值
			StatusAttributeSet->InitHealthAS(0, Config.BaseHealth);
			StatusAttributeSet->InitStaminaAS(0, Config.BaseStamina);
			// 设置其他属性
			// 注意：Stamina、Attack、Defense等需要额外配置GE
		}
	}

	// 2. 修改碰撞体大小
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCapsuleSize(Config.CapsuleRadius, Config.CapsuleHalfHeight);
	}

	// 3. 设置模型缩放,旋转,偏移
	if (GetMesh())
	{
		GetMesh()->SetRelativeScale3D(Config.MeshScale);
		GetMesh()->SetRelativeRotation(Config.MeshRelativeRotate);
		GetMesh()->SetRelativeLocation(FVector(0,0, -Config.CapsuleHalfHeight));
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

                // 检查是否已授予过，避免重复
                if (AbilitySystemComp->FindAbilitySpecFromClass(AbilityClass))
                {
                    UE_LOG(LogTemp, Verbose, TEXT("Enemy %s: Ability %s already granted, skipping"), 
                        *GetName(), *AbilityClass->GetName());
                    continue;
                }

                AbilitySystemComp->GiveAbility(Spec);			
			}
		}
	}

	// 6.1 设置队伍归属
	SetTeamID(Config.TeamID);

	// 7. 绑定GAS死亡事件
	BindGASDeathEvent();

	// 8. 设置基础移速
	GetCharacterMovement()->MaxWalkSpeed = Config.BaseMoveSpeed;
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
		}
	}

	// 加载动画蓝图
	if (!Config.AnimBlueprint.IsNull())
	{
		const FString AnimBPPath = Config.AnimBlueprint.ToString();
		UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::LoadEnemyAppearance - Loading AnimBP: %s"), *AnimBPPath);
		
		UClass* AnimClass = nullptr;
		
		// 尝试1: 直接加载 _C 类（Cook后UBlueprint可能被剥离，只剩生成的类）
		FString ClassPath = AnimBPPath + TEXT("_C");
		AnimClass = LoadObject<UClass>(nullptr, *ClassPath);
		if (AnimClass)
		{
			UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::LoadEnemyAppearance - Loaded via _C path: %s"), *ClassPath);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ASL_EnemyBase::LoadEnemyAppearance - _C path failed: %s"), *ClassPath);
			// 尝试2: 加载 UAnimBlueprint 对象
			UAnimBlueprint* AnimBP = LoadObject<UAnimBlueprint>(nullptr, *AnimBPPath);
			if (AnimBP)
			{ 
				AnimClass = Cast<UClass>(AnimBP->GetAnimBlueprintGeneratedClass());
				UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::LoadEnemyAppearance - Loaded via UAnimBlueprint"));
			}
		}
		
		if (AnimClass && GetMesh())
		{
			GetMesh()->SetAnimInstanceClass(AnimClass);
			UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::LoadEnemyAppearance - AnimClass set successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ASL_EnemyBase::LoadEnemyAppearance - AnimClass or Mesh is null, skip setting anim class"));
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

void ASL_EnemyBase::OnRep_EnemyID()
{
#if !UE_SERVER
	if (HasAuthority()) return;
	if (NetEnemyID <= 0) return;

	// 客户端从本地 DataTable 重新查询配置
	UDataTableManager* TableManager = UDataTableManager::Get(this);
	if (!TableManager) return;

	UEnemyConfigInfoTable* EnemyTable = Cast<UEnemyConfigInfoTable>(
		TableManager->GetDataTable(EDataTableType::DT_EnemyConfigInfo));
	if (!EnemyTable) return;

	FEnemyConfigInfo Config;
	if (!EnemyTable->GetEnemyConfig(NetEnemyID, Config))
	{
		UE_LOG(LogTemp, Error, TEXT("ASL_EnemyBase::OnRep_EnemyID - EnemyID=%d not found in config table"), NetEnemyID);
		return;
	}

	// 保存配置（使 GetBehaviorTree/GetBlackboardData 等访问器在客户端可用）
	EnemyConfig = Config;

	// 胶囊体大小
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCapsuleSize(Config.CapsuleRadius, Config.CapsuleHalfHeight);
	}

	// 模型变换（缩放、旋转、偏移）
	if (GetMesh())
	{
		GetMesh()->SetRelativeScale3D(Config.MeshScale);
		GetMesh()->SetRelativeRotation(Config.MeshRelativeRotate);
		GetMesh()->SetRelativeLocation(FVector(0, 0, -Config.CapsuleHalfHeight));
	}

	// 外观（骨骼网格体 + 动画蓝图）
	LoadEnemyAppearance(Config);

	// 武器（已有复制武器指针时跳过 SpawnActor）
	SpawnEnemyWeapons(Config);

	// AI 配置（供 AI Controller 引用）
	PerceptionRange = Config.PerceptionRange;
	AttackRange = Config.AttackRange;
	if (!Config.BehaviorTree.IsNull())
	{
		BehaviorTree = Config.BehaviorTree.LoadSynchronous();
	}
	if (!Config.BlackboardData.IsNull())
	{
		BlackboardData = Config.BlackboardData.LoadSynchronous();
	}

	// 基础移速
	GetCharacterMovement()->MaxWalkSpeed = Config.BaseMoveSpeed;

	// 队伍
	SetTeamID(Config.TeamID);

	UE_LOG(LogTemp, Log, TEXT("ASL_EnemyBase::OnRep_EnemyID - Client reinitialized: ID=%d"), NetEnemyID);
#endif
}

// ===== 新增：生成敌人武器 =====
void ASL_EnemyBase::SpawnEnemyWeapons(const FEnemyConfigInfo& Config)
{
	// 生成左手武器
	if (Config.LeftHandWeaponID > 0)
	{
		// 客户端上武器已通过复制到达时跳过生成，只做重定位
		if (!LeftHandWeapon || LeftHandWeapon->IsPendingKillPending())
		{
			LeftHandWeapon = SpawnWeaponByID(Config.LeftHandWeaponID);
		}
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
		// 客户端上武器已通过复制到达时跳过生成，只做重定位
		if (!RightHandWeapon || RightHandWeapon->IsPendingKillPending())
		{
			RightHandWeapon = SpawnWeaponByID(Config.RightHandWeaponID);
		}
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

// ===== 继承实现 =====
void ASL_EnemyBase::Destroyed()
{
	// 销毁附属武器，防止 ResetLevel 直接 Destroy 时武器悬空
	if (LeftHandWeapon)
	{
		LeftHandWeapon->Destroy();
		LeftHandWeapon = nullptr;
	}
	if (RightHandWeapon)
	{
		RightHandWeapon->Destroy();
		RightHandWeapon = nullptr;
	}

	Destroy();
}

// IAbilitySystemInterface 接口实现
UAbilitySystemComponent* ASL_EnemyBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComp;
}