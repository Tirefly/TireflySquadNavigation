// TsnStateTreeTask_ChaseEngagementTarget.cpp

#include "StateTree/TsnStateTreeTask_ChaseEngagementTarget.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"
#include "StateTreeExecutionContext.h"
#include "Interfaces/ITsnTacticalUnit.h"
#include "TsnLog.h"

FTsnStateTreeTask_ChaseEngagementTarget::FTsnStateTreeTask_ChaseEngagementTarget()
{
	bShouldCallTick = true;
}

EStateTreeRunStatus FTsnStateTreeTask_ChaseEngagementTarget::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	AAIController* AICon = Data.AIController;
	if (!AICon || !AICon->GetPawn())
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnStateTreeTask_ChaseEngagementTarget: missing AIController or Pawn."));
		return EStateTreeRunStatus::Failed;
	}

	if (!Data.TargetActor)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnStateTreeTask_ChaseEngagementTarget: TargetActor is null."));
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = AICon->GetPawn();
	if (!Pawn->Implements<UTsnTacticalUnit>())
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnStateTreeTask_ChaseEngagementTarget: Pawn [%s] does not implement ITsnTacticalUnit."),
			*Pawn->GetName());
		return EStateTreeRunStatus::Failed;
	}

	const float EngagementRange = ITsnTacticalUnit::Execute_GetEngagementRange(Pawn);
	if (EngagementRange <= 0.f)
	{
		UE_LOG(LogTireflySquadNav, Warning,
			TEXT("TsnStateTreeTask_ChaseEngagementTarget: GetEngagementRange() returned %.1f."),
			EngagementRange);
		return EStateTreeRunStatus::Failed;
	}

	Data.CachedEngagementRange = EngagementRange;
	const float PreEngagementRadius = EngagementRange * Data.PreEngagementRadiusMultiplier;

	// 已在预战斗距离内 �?立即成功
	if (FVector::Dist2D(Pawn->GetActorLocation(), Data.TargetActor->GetActorLocation())
		<= PreEngagementRadius)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalActor(Data.TargetActor);
	MoveReq.SetAcceptanceRadius(PreEngagementRadius);
	MoveReq.SetUsePathfinding(true);
	MoveReq.SetAllowPartialPath(true);
	MoveReq.SetReachTestIncludesAgentRadius(false);

	const FPathFollowingRequestResult Result = AICon->MoveTo(MoveReq);
	if (Result.Code == EPathFollowingRequestResult::Failed)
	{
		return EStateTreeRunStatus::Failed;
	}
	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FTsnStateTreeTask_ChaseEngagementTarget::Tick(
	FStateTreeExecutionContext& Context,
	const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	AAIController* AICon = Data.AIController;
	if (!AICon || !AICon->GetPawn() || !Data.TargetActor)
	{
		if (AICon) AICon->StopMovement();
		return EStateTreeRunStatus::Failed;
	}

	const float PreEngagementRadius =
		Data.CachedEngagementRange * Data.PreEngagementRadiusMultiplier;
	const float Dist2D = FVector::Dist2D(
		AICon->GetPawn()->GetActorLocation(),
		Data.TargetActor->GetActorLocation());

	if (Dist2D <= PreEngagementRadius)
	{
		AICon->StopMovement();
		return EStateTreeRunStatus::Succeeded;
	}

	return EStateTreeRunStatus::Running;
}

void FTsnStateTreeTask_ChaseEngagementTarget::ExitState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	if (Data.AIController)
	{
		Data.AIController->StopMovement();
	}
}
