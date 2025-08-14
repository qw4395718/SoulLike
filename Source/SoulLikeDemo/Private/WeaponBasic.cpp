#include "WeaponBasic.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"

AWeaponBasic::AWeaponBasic()
{
	//是否开启tick
	PrimaryActorTick.bCanEverTick = false;

	/************************************************************************/
	/*                              组件初始化                                        */
	/************************************************************************/
	// 场景组件root
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneRoot;

	// 骨骼网格体-
	SkeletalWeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalWeaponMesh->SetupAttachment(RootComponent);

	/************************************************************************/
	/*                              变量初始化                                        */
	/************************************************************************/
	OwnerActor = nullptr;
	WeaponID = 0;
	SoftMeshReference = nullptr;
	CollisionBoxSize = FVector(0.0f,0.0f,0.0f);
	SoftWeaponAnimInstanceReference = nullptr;
	WeaponComponentMap.Reset();
	WeaponLoadComponentInfoMap.Reset();
	WeaponEquipInfo = EWeaponEquipState::No_Equip;
}

void AWeaponBasic::InitWeaponInfo(const FWeaponDefinition& WeaponInfo)
{
	if(WeaponInfo.WeaponID == 0){return;}
	// 变量赋值
	OwnerActor = GetOwner();
	WeaponID = WeaponInfo.WeaponID;
	// 加载武器模型
	LoadWeaponMeshAsync(WeaponInfo.Mesh);
	CollisionBoxSize = WeaponInfo.WeaponCollisionBoxSize;
	// 加载武器动作蓝图
	LoadWeaponAnimInstanceAsync(WeaponInfo.AnimClass);
	// 加载武器模组
	LoadWeaponComponents(WeaponInfo.NeedLoadComponentInfoMap);
}

void AWeaponBasic::UpdateWeaponEquipState(EWeaponEquipState CurrentState)
{
	
}

void AWeaponBasic::LeftMouseCallEvent()
{

}

void AWeaponBasic::RightMouseCallEvent()
{

}

void AWeaponBasic::CtrlMouseCallEvent()
{

}

void AWeaponBasic::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBasic::LoadWeaponMeshAsync(FString WeaponMeshName)
{
	// 资源异步加载
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	SoftMeshReference = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(*WeaponMeshName));
	Streamable.RequestAsyncLoad(
		SoftMeshReference.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &AWeaponBasic::OnLoadedWeaponMesh)
	);
}

void AWeaponBasic::OnLoadedWeaponMesh()
{
	if (USkeletalMesh* Mesh = SoftMeshReference.Get()) {
		SkeletalWeaponMesh->SetSkeletalMesh(Mesh);
	}
}

void AWeaponBasic::LoadWeaponAnimInstanceAsync(FString WeapinAnimName)
{

}

void AWeaponBasic::OnLoadedWeaponAnimInstance()
{

}

bool AWeaponBasic::LoadWeaponComponents(const TMap<EWeaponComponentType, bool>& pWeaponComponentInfo)
{
	// 根据map信息创建各个模组,并将创建的实际情况更新至WeaponLoadComponentInfoMap
	return false;
}

bool AWeaponBasic::CanExecute(AActor* MasterActor, float AllowedExecuteDistance, float AllowdBackStabRange)
{
	return false;
}

bool AWeaponBasic::CanBackStab(AActor* MasterActor, float AllowedBackStabDistance, float AllowdBackStabRange)
{
	return false;
}
