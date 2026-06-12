#include "SL_PhantomCharacter.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include <Components/CapsuleComponent.h>

ASL_PhantomCharacter::ASL_PhantomCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;

    bReplicates = true;
    bCanInteractWithWorld = false;
    bCanBeDamagedByWorld = true;

    // 默认半透明材质路径（可在蓝图中替换）
    PhantomMaterialOverride = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/SoulLikeDemo/Materials/MI_Phantom.MI_Phantom")));
}

void ASL_PhantomCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        ApplyInteractionRestrictions();
    }
}

void ASL_PhantomCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ASL_PhantomCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASL_PhantomCharacter, PhantomData);
    DOREPLIFETIME(ASL_PhantomCharacter, bCanInteractWithWorld);
    DOREPLIFETIME(ASL_PhantomCharacter, bCanBeDamagedByWorld);
}

/************************************************************************/
/*                               外部调用                               */
/************************************************************************/

void ASL_PhantomCharacter::ApplyPhantomData(const FPhantomData& InData)
{
    PhantomData = InData;

    // 重建外观（网格体、材质、动画）
    RebuildAppearance();

    // 应用半透明视觉效果
    ApplyTranslucentEffect();

    UE_LOG(LogTemp, Log, TEXT("PhantomCharacter: Applied phantom data for %s (mesh=%s)"),
        *PhantomData.OwnerName, *PhantomData.CharacterMeshPath);
}

void ASL_PhantomCharacter::SetInteractionEnabled(bool bEnabled)
{
    bCanInteractWithWorld = bEnabled;
}

void ASL_PhantomCharacter::Repatriate(EReturnReason InReason)
{
    UE_LOG(LogTemp, Log, TEXT("PhantomCharacter: Repatriating %s (reason=%d)"),
        *PhantomData.OwnerName, (int32)InReason);

    // 播放遣返特效（Phase4 实现）
    // PlayRepatriationEffect();

    // 通知客户端准备断开
    // Multicast_OnRepatriate(InReason);

    // 延迟销毁，给客户端播放动画的时间
    SetLifeSpan(1.0f);
}

/************************************************************************/
/*                               内部调用                               */
/************************************************************************/

void ASL_PhantomCharacter::RebuildAppearance()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;

    // 加载并设置骨骼网格体
    if (!PhantomData.CharacterMeshPath.IsEmpty())
    {
        USkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(
            StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *PhantomData.CharacterMeshPath));
        if (LoadedMesh)
        {
            MeshComp->SetSkeletalMesh(LoadedMesh);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PhantomCharacter: Failed to load mesh %s"), *PhantomData.CharacterMeshPath);
        }
    }

    // 加载并设置动画蓝图
    if (!PhantomData.AnimBlueprintPath.IsEmpty())
    {
        UClass* AnimBPClass = Cast<UClass>(
            StaticLoadObject(UClass::StaticClass(), nullptr, *PhantomData.AnimBlueprintPath));
        if (AnimBPClass)
        {
            MeshComp->SetAnimInstanceClass(AnimBPClass);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PhantomCharacter: Failed to load anim BP %s"), *PhantomData.AnimBlueprintPath);
        }
    }

    // 设置材质
    PhantomMaterials.Empty();
    for (int32 i = 0; i < PhantomData.MaterialPaths.Num(); i++)
    {
        const FString& MatPath = PhantomData.MaterialPaths[i];
        if (!MatPath.IsEmpty())
        {
            UMaterialInterface* Mat = Cast<UMaterialInterface>(
                StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MatPath));
            if (Mat)
            {
                MeshComp->SetMaterial(i, Mat);
            }
        }
    }
}

void ASL_PhantomCharacter::ApplyTranslucentEffect()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp) return;

    // 加载半透明材质
    UMaterialInterface* TranslucentMat = PhantomMaterialOverride.LoadSynchronous();
    if (!TranslucentMat)
    {
        // 如果资源不存在，尝试创建动态材质
        TranslucentMat = UMaterial::GetDefaultMaterial(MD_Surface);
        if (!TranslucentMat) return;
    }

    // 为每个材质槽创建动态实例
    PhantomMaterials.Empty();
    for (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
    {
        UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(TranslucentMat, this);
        if (DynMat)
        {
            // 设置透明度参数（材质参数名需与材质保持一致）
            DynMat->SetScalarParameterValue(FName("Opacity"), 0.6f);
            DynMat->SetScalarParameterValue(FName("GlowIntensity"), 0.3f);
            MeshComp->SetMaterial(i, DynMat);
            PhantomMaterials.Add(DynMat);
        }
    }

    // 关闭投影（灵体不应该有影子）
    MeshComp->SetCastShadow(false);
    MeshComp->bReceivesDecals = false;
}

void ASL_PhantomCharacter::ApplyInteractionRestrictions()
{
    // 禁用物理互动
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetSimulatePhysics(false);

    // 启用基本的碰撞检测（让 AI 和攻击能命中）
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
  
    // 标记不能被其他玩家锁定
    // 如果需要支持锁定的游戏，可以在这里处理
}

void ASL_PhantomCharacter::OnRep_PhantomData()
{
    // 客户端收到 PhantomData 复制后重建外观
    RebuildAppearance();
    ApplyTranslucentEffect();
}