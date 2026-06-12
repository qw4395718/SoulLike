#include "SL_PhantomCharacter.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Animation/AnimBlueprintGeneratedClass.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

ASL_PhantomCharacter::ASL_PhantomCharacter()
{
\tPrimaryActorTick.bCanEverTick = true;
\tPrimaryActorTick.TickInterval = 1.0f;

\tbReplicates = true;
\tbCanInteractWithWorld = false;
\tbCanBeDamagedByWorld = true;

\t// 默认半透明材质路径（可在蓝图中替换）
\tPhantomMaterialOverride = TSoftObjectPtr<UMaterialInterface>(FSoftObjectPath(TEXT("/Game/SoulLikeDemo/Materials/MI_PhantomTranslucent.MI_PhantomTranslucent")));
}

void ASL_PhantomCharacter::BeginPlay()
{
\tSuper::BeginPlay();

\tif (HasAuthority())
\t{
\t\tApplyInteractionRestrictions();
\t}
}

void ASL_PhantomCharacter::Tick(float DeltaTime)
{
\tSuper::Tick(DeltaTime);
}

void ASL_PhantomCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
\tSuper::GetLifetimeReplicatedProps(OutLifetimeProps);

\tDOREPLIFETIME(ASL_PhantomCharacter, PhantomData);
\tDOREPLIFETIME(ASL_PhantomCharacter, bCanInteractWithWorld);
\tDOREPLIFETIME(ASL_PhantomCharacter, bCanBeDamagedByWorld);
}

/************************************************************************/
/*                               外部调用                               */
/************************************************************************/

void ASL_PhantomCharacter::ApplyPhantomData(const FPhantomData& InData)
{
\tPhantomData = InData;

\t// 重建外观（网格体、材质、动画）
\tRebuildAppearance();

\t// 应用半透明视觉效果
\tApplyTranslucentEffect();

\tUE_LOG(LogTemp, Log, TEXT("PhantomCharacter: Applied phantom data for %s (mesh=%s)"),
\t\t*PhantomData.OwnerName, *PhantomData.CharacterMeshPath);
}

void ASL_PhantomCharacter::SetInteractionEnabled(bool bEnabled)
{
\tbCanInteractWithWorld = bEnabled;
}

void ASL_PhantomCharacter::Repatriate(EReturnReason InReason)
{
\tUE_LOG(LogTemp, Log, TEXT("PhantomCharacter: Repatriating %s (reason=%d)"),
\t\t*PhantomData.OwnerName, (int32)InReason);

\t// 播放遣返特效（Phase4 实现）
\t// PlayRepatriationEffect();

\t// 通知客户端准备断开
\t// Multicast_OnRepatriate(InReason);

\t// 延迟销毁，给客户端播放动画的时间
\tSetLifeSpan(1.0f);
}

/************************************************************************/
/*                               内部调用                               */
/************************************************************************/

void ASL_PhantomCharacter::RebuildAppearance()
{
\tUSkeletalMeshComponent* MeshComp = GetMesh();
\tif (!MeshComp) return;

\t// 加载并设置骨骼网格体
\tif (!PhantomData.CharacterMeshPath.IsEmpty())
\t{
\t\tUSkeletalMesh* LoadedMesh = Cast<USkeletalMesh>(
\t\t\tStaticLoadObject(USkeletalMesh::StaticClass(), nullptr, *PhantomData.CharacterMeshPath));
\t\tif (LoadedMesh)
\t\t{
\t\t\tMeshComp->SetSkeletalMesh(LoadedMesh);
\t\t}
\t\telse
\t\t{
\t\t\tUE_LOG(LogTemp, Warning, TEXT("PhantomCharacter: Failed to load mesh %s"), *PhantomData.CharacterMeshPath);
\t\t}
\t}

\t// 加载并设置动画蓝图
\tif (!PhantomData.AnimBlueprintPath.IsEmpty())
\t{
\t\tUClass* AnimBPClass = Cast<UClass>(
\t\t\tStaticLoadObject(UClass::StaticClass(), nullptr, *PhantomData.AnimBlueprintPath));
\t\tif (AnimBPClass)
\t\t{
\t\t\tMeshComp->SetAnimInstanceClass(AnimBPClass);
\t\t}
\t\telse
\t\t{
\t\t\tUE_LOG(LogTemp, Warning, TEXT("PhantomCharacter: Failed to load anim BP %s"), *PhantomData.AnimBlueprintPath);
\t\t}
\t}

\t// 设置材质
\tPhantomMaterials.Empty();
\tfor (int32 i = 0; i < PhantomData.MaterialPaths.Num(); i++)
\t{
\t\tconst FString& MatPath = PhantomData.MaterialPaths[i];
\t\tif (!MatPath.IsEmpty())
\t\t{
\t\t\tUMaterialInterface* Mat = Cast<UMaterialInterface>(
\t\t\t\tStaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MatPath));
\t\t\tif (Mat)
\t\t\t{
\t\t\t\tMeshComp->SetMaterial(i, Mat);
\t\t\t}
\t\t}
\t}
}

void ASL_PhantomCharacter::ApplyTranslucentEffect()
{
\tUSkeletalMeshComponent* MeshComp = GetMesh();
\tif (!MeshComp) return;

\t// 加载半透明材质
\tUMaterialInterface* TranslucentMat = PhantomMaterialOverride.LoadSynchronous();
\tif (!TranslucentMat)
\t{
\t\t// 如果资源不存在，尝试创建动态材质
\t\tTranslucentMat = UMaterial::GetDefaultMaterial(MD_Surface);
\t\tif (!TranslucentMat) return;
\t}

\t// 为每个材质槽创建动态实例
\tPhantomMaterials.Empty();
\tfor (int32 i = 0; i < MeshComp->GetNumMaterials(); i++)
\t{
\t\tUMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(TranslucentMat, this);
\t\tif (DynMat)
\t\t{
\t\t\t// 设置透明度参数（材质参数名需与材质保持一致）
\t\t\tDynMat->SetScalarParameterValue(FName("Opacity"), 0.6f);
\t\t\tDynMat->SetScalarParameterValue(FName("GlowIntensity"), 0.3f);
\t\t\tMeshComp->SetMaterial(i, DynMat);
\t\t\tPhantomMaterials.Add(DynMat);
\t\t}
\t}

\t// 关闭投影（灵体不应该有影子）
\tMeshComp->SetCastShadow(false);
\tMeshComp->bReceivesDecals = false;
}

void ASL_PhantomCharacter::ApplyInteractionRestrictions()
{
\t// 禁用物理互动
\tGetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
\tGetMesh()->SetSimulatePhysics(false);

\t// 启用基本的碰撞检测（让 AI 和攻击能命中）
\tGetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

\t// 标记不能被其他玩家锁定
\t// 如果需要支持锁定的游戏，可以在这里处理
}

void ASL_PhantomCharacter::OnRep_PhantomData()
{
\t// 客户端收到 PhantomData 复制后重建外观
\tRebuildAppearance();
\tApplyTranslucentEffect();
}