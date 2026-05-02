// TsnStateTreeEvaluator_CombatContext.cpp

#include "StateTree/TsnStateTreeEvaluator_CombatContext.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"
#include "Interfaces/ITsnTacticalUnit.h"

namespace
{
	static void ResetOutputs(FTsnStateTreeEvaluator_CombatContextInstanceData& Data)
	{
		Data.bHasValidTarget = false;
		Data.DistanceToTarget2D = 0.f;
		Data.EngagementRange = 0.f;
		Data.PreEngagementRadius = 0.f;
		Data.bIsInEngagementRange = false;
		Data.bIsInPreEngagementRange = false;
	}

	static void Evaluate(FTsnStateTreeEvaluator_CombatContextInstanceData& Data)
	{
		AAIController* AICon = Data.AIController;
		APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
		if (!Pawn || !Data.TargetActor)
		{
			ResetOutputs(Data);
			return;
		}

		if (!Pawn->Implements<UTsnTacticalUnit>())
		{
			ResetOutputs(Data);
			return;
		}

		const float EngagementRange = ITsnTacticalUnit::Execute_GetEngagementRange(Pawn);
		if (EngagementRange <= 0.f)
		{
			ResetOutputs(Data);
			return;
		}

		const float Dist2D = FVector::Dist2D(
			Pawn->GetActorLocation(), Data.TargetActor->GetActorLocation());
		const float PreEngagementRadius = EngagementRange * Data.PreEngagementRadiusMultiplier;

		Data.bHasValidTarget = true;
		Data.DistanceToTarget2D = Dist2D;
		Data.EngagementRange = EngagementRange;
		Data.PreEngagementRadius = PreEngagementRadius;
		Data.bIsInEngagementRange = Dist2D <= EngagementRange;
		Data.bIsInPreEngagementRange = Dist2D <= PreEngagementRadius;
	}
}

void FTsnStateTreeEvaluator_CombatContext::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	Evaluate(Data);
}

void FTsnStateTreeEvaluator_CombatContext::Tick(
	FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	Evaluate(Data);
}
