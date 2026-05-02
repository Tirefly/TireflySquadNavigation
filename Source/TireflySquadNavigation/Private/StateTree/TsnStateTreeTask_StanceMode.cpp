// TsnStateTreeTask_StanceMode.cpp

#include "StateTree/TsnStateTreeTask_StanceMode.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "TsnLog.h"

FTsnStateTreeTask_EnterStanceMode::FTsnStateTreeTask_EnterStanceMode()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FTsnStateTreeTask_EnterStanceMode::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!Data.AIController || !Data.AIController->GetPawn())
	{
		return EStateTreeRunStatus::Failed;
	}

	UTsnStanceObstacleComponent* ObsComp =
		Data.AIController->GetPawn()->FindComponentByClass<UTsnStanceObstacleComponent>();
	if (!ObsComp)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnStateTreeTask_EnterStanceMode: Pawn [%s] missing UTsnStanceObstacleComponent."),
			*Data.AIController->GetPawn()->GetName());
		return EStateTreeRunStatus::Failed;
	}

	ObsComp->EnterStanceMode();
	return EStateTreeRunStatus::Succeeded;
}

FTsnStateTreeTask_ExitStanceMode::FTsnStateTreeTask_ExitStanceMode()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FTsnStateTreeTask_ExitStanceMode::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	if (!Data.AIController || !Data.AIController->GetPawn())
	{
		return EStateTreeRunStatus::Failed;
	}

	UTsnStanceObstacleComponent* ObsComp =
		Data.AIController->GetPawn()->FindComponentByClass<UTsnStanceObstacleComponent>();
	if (!ObsComp)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnStateTreeTask_ExitStanceMode: Pawn [%s] missing UTsnStanceObstacleComponent."),
			*Data.AIController->GetPawn()->GetName());
		return EStateTreeRunStatus::Failed;
	}

	ObsComp->ExitStanceMode();
	return EStateTreeRunStatus::Succeeded;
}
