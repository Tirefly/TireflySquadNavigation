// TsnTestStateTreeTask_Attack.cpp

#include "StateTree/TsnTestStateTreeTask_Attack.h"
#include "TsnTestLog.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"

FTsnTestStateTreeTask_Attack::FTsnTestStateTreeTask_Attack()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FTsnTestStateTreeTask_Attack::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!Data.AIController || !Data.AIController->GetPawn())
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.ElapsedTime = 0.f;

	UE_LOG(LogTsnTest, Verbose, TEXT("[%s] ST Attack started (%.1fs)"),
		*Data.AIController->GetPawn()->GetName(), Data.AttackDuration);

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FTsnTestStateTreeTask_Attack::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	if (!Data.AIController || !Data.AIController->GetPawn())
	{
		return EStateTreeRunStatus::Failed;
	}

	Data.ElapsedTime += DeltaTime;

	if (Data.ElapsedTime >= Data.AttackDuration)
	{
		UE_LOG(LogTsnTest, Verbose, TEXT("[%s] ST Attack completed"),
			*Data.AIController->GetPawn()->GetName());
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}
