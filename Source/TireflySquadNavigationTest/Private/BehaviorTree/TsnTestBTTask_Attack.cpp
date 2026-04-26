// TsnTestBTTask_Attack.cpp
// 测试用 BT 任务 —— 模拟攻击（异步计时）

#include "BehaviorTree/TsnTestBTTask_Attack.h"
#include "TsnTestLog.h"
#include "AIController.h"

UTsnTestBTTask_Attack::UTsnTestBTTask_Attack()
{
	NodeName = TEXT("TSN Test Attack");
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UTsnTestBTTask_Attack::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon || !AICon->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	RemainingTime = AttackDuration;

	UE_LOG(LogTsnTest, Verbose, TEXT("[%s] Attack started (%.1fs)"),
		*AICon->GetPawn()->GetName(), AttackDuration);

	return EBTNodeResult::InProgress;
}

void UTsnTestBTTask_Attack::TickTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	RemainingTime -= DeltaSeconds;

	if (RemainingTime <= 0.f)
	{
		AAIController* AICon = OwnerComp.GetAIOwner();
		UE_LOG(LogTsnTest, Verbose, TEXT("[%s] Attack completed"),
			AICon && AICon->GetPawn() ? *AICon->GetPawn()->GetName() : TEXT("Unknown"));

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UTsnTestBTTask_Attack::AbortTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	RemainingTime = 0.f;
	return EBTNodeResult::Aborted;
}
