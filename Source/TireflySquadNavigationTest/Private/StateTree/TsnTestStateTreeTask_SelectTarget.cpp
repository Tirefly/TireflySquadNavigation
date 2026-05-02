// TsnTestStateTreeTask_SelectTarget.cpp

#include "StateTree/TsnTestStateTreeTask_SelectTarget.h"
#include "TsnTestChessPiece.h"
#include "TsnTestTargetDummy.h"
#include "TsnTestLog.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"
#include "EngineUtils.h"
#include "Engine/World.h"

FTsnTestStateTreeTask_SelectTarget::FTsnTestStateTreeTask_SelectTarget()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FTsnTestStateTreeTask_SelectTarget::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	Data.TargetActor = nullptr;
	Data.bHasTarget = false;

	if (!Data.AIController || !Data.AIController->GetPawn())
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* OwnerPawn = Data.AIController->GetPawn();
	UWorld* World = OwnerPawn->GetWorld();
	if (!World)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FVector OwnerLoc = OwnerPawn->GetActorLocation();

	// 自身 TeamID（仅 ATsnTestChessPiece 持有；非棋子记 -1，等价于不过滤）
	int32 MyTeamID = -1;
	if (const ATsnTestChessPiece* MyPiece = Cast<ATsnTestChessPiece>(OwnerPawn))
	{
		MyTeamID = MyPiece->TeamID;
	}

	AActor* BestTarget = nullptr;
	float BestDistSq = Data.SearchRadius * Data.SearchRadius;

	// 搜索敌方棋子
	for (TActorIterator<ATsnTestChessPiece> It(World); It; ++It)
	{
		ATsnTestChessPiece* Candidate = *It;
		if (!Candidate || Candidate == OwnerPawn) continue;
		if (MyTeamID >= 0 && Candidate->TeamID == MyTeamID) continue;

		const float DistSq = FVector::DistSquared(OwnerLoc, Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	// 搜索靶标（始终视为有效目标）
	for (TActorIterator<ATsnTestTargetDummy> It(World); It; ++It)
	{
		ATsnTestTargetDummy* Candidate = *It;
		if (!Candidate) continue;

		const float DistSq = FVector::DistSquared(OwnerLoc, Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	if (BestTarget)
	{
		Data.TargetActor = BestTarget;
		Data.bHasTarget = true;

		UE_LOG(LogTsnTest, Verbose, TEXT("[%s] ST SelectTarget -> %s (dist=%.0f)"),
			*OwnerPawn->GetName(), *BestTarget->GetName(), FMath::Sqrt(BestDistSq));
		return EStateTreeRunStatus::Succeeded;
	}

	UE_LOG(LogTsnTest, Verbose, TEXT("[%s] ST SelectTarget -> no target found"), *OwnerPawn->GetName());
	return EStateTreeRunStatus::Failed;
}
