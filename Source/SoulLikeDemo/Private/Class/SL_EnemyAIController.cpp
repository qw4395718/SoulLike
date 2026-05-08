// Private/AI/SL_EnemyAIController.cpp
#include "SL_EnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "SL_EnemyBase.h"

ASL_EnemyAIController::ASL_EnemyAIController()
{
    // UE4.26: 使用BehaviorTreeComponent
    BrainComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BrainComponent"));
    Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

/************************************************************************/
/*                               继承实现                                */
/************************************************************************/

void ASL_EnemyAIController::BeginPlay()
{
    Super::BeginPlay();
}

void ASL_EnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 获取敌人实例
    ASL_EnemyBase* Enemy = Cast<ASL_EnemyBase>(InPawn);
    if (Enemy)
    {
        // 如果敌人已经初始化了AI配置，直接从敌人获取
        if (Enemy->GetBehaviorTree() && Enemy->GetBlackboardData())
        {
            InitializeAI(Enemy->GetBehaviorTree(), Enemy->GetBlackboardData());
        }
    }
}

void ASL_EnemyAIController::OnUnPossess()
{
    // UE4.26: 停止行为树
    if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
    {
        BTComp->StopTree(EBTStopMode::Safe);
    }

    Super::OnUnPossess();
}

void ASL_EnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

/************************************************************************/
/*                               外部调用                                */
/************************************************************************/

void ASL_EnemyAIController::InitializeAI(UBehaviorTree* InBehaviorTree, UBlackboardData* InBlackboardData)
{
    if (!InBehaviorTree || !InBlackboardData)
    {
        UE_LOG(LogTemp, Error, TEXT("ASL_EnemyAIController::InitializeAI - Invalid BT or BB"));
        return;
    }

    CurrentBehaviorTree = InBehaviorTree;
    CurrentBlackboardData = InBlackboardData;

    RunBehaviorTreeAndBlackboard();
}

void ASL_EnemyAIController::SetTargetActor(AActor* NewTarget)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsObject(FName("TargetActor"), NewTarget);
    }
}

AActor* ASL_EnemyAIController::GetTargetActor() const
{
    if (Blackboard)
    {
        return Cast<AActor>(Blackboard->GetValueAsObject(FName("TargetActor")));
    }
    return nullptr;
}

void ASL_EnemyAIController::SetBlackboardValue(const FName& KeyName, bool bValue)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsBool(KeyName, bValue);
    }
}

void ASL_EnemyAIController::SetBlackboardValueAsObject(const FName& KeyName, UObject* Value)
{
    if (Blackboard)
    {
        Blackboard->SetValueAsObject(KeyName, Value);
    }
}

/************************************************************************/
/*                               内部调用                                */
/************************************************************************/

void ASL_EnemyAIController::RunBehaviorTreeAndBlackboard()
{
    if (!CurrentBehaviorTree || !CurrentBlackboardData)
    {
        UE_LOG(LogTemp, Warning, TEXT("ASL_EnemyAIController::RunBehaviorTree - No BT or BB set"));
        return;
    }

    // UE4.26: 初始化黑板
    if (Blackboard && Blackboard->InitializeBlackboard(*CurrentBlackboardData))
    {
        // UE4.26: 运行行为树
        if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
        {
            BTComp->StartTree(*CurrentBehaviorTree, EBTExecutionMode::Looped);
            UE_LOG(LogTemp, Log, TEXT("ASL_EnemyAIController: Started BT %s with BB %s"),
                *CurrentBehaviorTree->GetName(), *CurrentBlackboardData->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ASL_EnemyAIController: Failed to initialize Blackboard"));
    }
}