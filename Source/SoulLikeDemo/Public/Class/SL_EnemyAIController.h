// Public/AI/SL_EnemyAIController.h
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SL_EnemyAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;

UCLASS()
class SOULLIKEDEMO_API ASL_EnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    ASL_EnemyAIController();

    /************************************************************************/
    /*                               外部调用                               */
    /************************************************************************/
    // 初始化AI（由Spawner或EnemyBase调用）
    UFUNCTION(BlueprintCallable, Category = "AI")
        void InitializeAI(UBehaviorTree* InBehaviorTree, UBlackboardData* InBlackboardData);

    // 设置目标
    UFUNCTION(BlueprintCallable, Category = "AI")
        void SetTargetActor(AActor* NewTarget);

    // 获取当前目标
    UFUNCTION(BlueprintPure, Category = "AI")
        AActor* GetTargetActor() const;

    // 设置黑板值（方便外部调用）
    UFUNCTION(BlueprintCallable, Category = "AI")
        void SetBlackboardValue(const FName& KeyName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "AI")
        void SetBlackboardValueAsObject(const FName& KeyName, UObject* Value);

protected:
    /************************************************************************/
    /*                               继承实现                               */
    /************************************************************************/
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void Tick(float DeltaTime) override;

    /************************************************************************/
    /*                               内部调用                               */
    /************************************************************************/
    // 运行行为树
    void RunBehaviorTreeAndBlackboard();

protected:
    /************************************************************************/
    /*                               内部访问                               */
    /************************************************************************/
    // 行为树和黑板（从EnemyConfig中设置）
    UPROPERTY()
        UBehaviorTree* CurrentBehaviorTree;

    UPROPERTY()
        UBlackboardData* CurrentBlackboardData;
};