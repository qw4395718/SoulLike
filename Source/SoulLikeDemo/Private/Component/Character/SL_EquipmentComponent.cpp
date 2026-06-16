// Private/Component/Character/SL_EquipmentComponent.cpp

#include "SL_EquipmentComponent.h"
#include "SL_CharacterBase.h"
#include "DataTableManager.h"
#include "ClassConfigInfoTable.h"
#include "WeaponDataTable.h"
#include "SL_Macros.h"
#include "Engine/World.h"

USL_EquipmentComponent::USL_EquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CurrentLeftHandWeapon = nullptr;
	CurrentRightHandWeapon = nullptr;
	CurrentSelectSlotIndex = 0;

	// 默认武器派生类
	WeaponBaseClass = ASL_WeaponBase::StaticClass();
}

void USL_EquipmentComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

void USL_EquipmentComponent::OnRegister()
{
	Super::OnRegister();
	OwningCharacter = Cast<ASL_CharacterBase>(GetOwner());
}

void USL_EquipmentComponent::OnUnregister()
{
	Super::OnUnregister();
}

void USL_EquipmentComponent::WeaponAnimNotifyResponse(int NotifyType)
{
	
}

// ==================== 初始化 ====================

void USL_EquipmentComponent::InitializeWithClassID(int32 ClassID)
{
	// 从DataTableManager获取职业配置表
	if (UDataTableManager* TableManager = UDataTableManager::Get(this))
	{
		if (UClassConfigInfoTable* ClassTable = Cast<UClassConfigInfoTable>(TableManager->GetDataTable(EDataTableType::DT_ClassConfigInfo)))
		{
			FClassConfigInfo OutConfig;
			if (ClassTable->GetClassConfig(ClassID, OutConfig))
			{
				InitializeWithConfig(OutConfig);
				return;
			}
		}
	}

	UE_LOG(LogTemp, Error, TEXT("USL_EquipmentComponent::InitializeWithClassID - Failed to load class config for ClassID: %d"), ClassID);
}

void USL_EquipmentComponent::InitializeWithConfig(const FClassConfigInfo& ClassConfig)
{
	CurrentClassConfig = ClassConfig;

	// 1. 设置道具ID
	//SlotItemIDList = ClassConfig.SlotItemIDs;

	// 2. 装备武器
	EquipLeftHandWeapon(ClassConfig.LeftHandWeaponID);
	EquipRightHandWeapon(ClassConfig.RightHandWeaponID);

	UE_LOG(LogTemp, Log, TEXT("USL_EquipmentComponent::InitializeWithConfig - ClassID=%d, LHWeapon=%d, RHWeapon=%d"),
		ClassConfig.ClassID,
		ClassConfig.LeftHandWeaponID,
		ClassConfig.RightHandWeaponID);
}

// ==================== 武器派生 ====================

ASL_WeaponBase* USL_EquipmentComponent::SpawnWeaponByID(int32 WeaponID, bool IsRightHand /*= true*/)
{
	if (!OwningCharacter || !GetWorld() || WeaponID <= 0) return nullptr;

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
		if (IsRightHand)
		{
			SocketName = CurrentClassConfig.RightHandSocketName;
		}
		else
		{
			SocketName = CurrentClassConfig.LeftHandSocketName;
		}
		
		Weapon->InitializeWeaponWithID(WeaponID, SocketName);
		Weapon->SetOwner(OwningCharacter);
		Weapon->FinishSpawning(FTransform::Identity);

		UE_LOG(LogTemp, Verbose, TEXT("USL_EquipmentComponent::SpawnWeaponByID - Spawned weapon ID=%d"), WeaponID);
	}

	return Weapon;
}

void USL_EquipmentComponent::DestroyWeapon(ASL_WeaponBase* Weapon)
{
	if (Weapon && !Weapon->IsPendingKillPending())
	{
		Weapon->Destroy();
	}
}

// ==================== 装备武器 ====================

void USL_EquipmentComponent::EquipLeftHandWeapon(int32 WeaponID)
{
	// 1. 解除当前武器
	UnequipCurrentWeapon(CurrentLeftHandWeapon);

	// 2. 如果没有武器ID（值为0或不合法），直接返回
	if (WeaponID <= 0)
	{
		CurrentLeftHandWeapon = nullptr;
		OnLeftHandWeaponEquipped.Broadcast(nullptr, EArrowKeyType::ARROWKEY_Left);
		return;
	}

	// 3. 派生新武器
	CurrentLeftHandWeapon = SpawnWeaponByID(WeaponID,false);

	// 4. 广播事件
	OnLeftHandWeaponEquipped.Broadcast(CurrentLeftHandWeapon, EArrowKeyType::ARROWKEY_Left);
}

void USL_EquipmentComponent::EquipRightHandWeapon(int32 WeaponID)
{
	// 1. 解除当前武器
	UnequipCurrentWeapon(CurrentRightHandWeapon);

	// 2. 如果没有武器ID（值为0或不合法），直接返回
	if (WeaponID <= 0)
	{
		CurrentRightHandWeapon = nullptr;
		OnRightHandWeaponEquipped.Broadcast(nullptr, EArrowKeyType::ARROWKEY_Right);
		return;
	}

	// 3. 派生新武器
	CurrentRightHandWeapon = SpawnWeaponByID(WeaponID,true);

	// 4. 广播事件
	OnRightHandWeaponEquipped.Broadcast(CurrentRightHandWeapon, EArrowKeyType::ARROWKEY_Right);
}

void USL_EquipmentComponent::UnequipCurrentWeapon(ASL_WeaponBase*& CurrentWeapon)
{
	if (CurrentWeapon)
	{
		// 关闭碰撞
		CurrentWeapon->DisableAttackCollision();
		CurrentWeapon->DisableParryWindow();

		// 销毁武器
		DestroyWeapon(CurrentWeapon);
		CurrentWeapon = nullptr;
	}
}

// ==================== 武器访问 ====================

ASL_WeaponBase* USL_EquipmentComponent::GetWeaponByHand(int32 bIsRightHand) const
{
	return bIsRightHand ? CurrentRightHandWeapon : CurrentLeftHandWeapon;
}

// ==================== 道具操作 ====================

void USL_EquipmentComponent::UseSelectedSlotItem()
{
}

int32 USL_EquipmentComponent::GetSelectSlotItemID() const
{
	RETURN_VALUE_IF_TRUE(CurrentSelectSlotIndex <= 0 || CurrentSelectSlotIndex > SlotItemIDList.Num(),0);
	return SlotItemIDList[CurrentSelectSlotIndex];

}
