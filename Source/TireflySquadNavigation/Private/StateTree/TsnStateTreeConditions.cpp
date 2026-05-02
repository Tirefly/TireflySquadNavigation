// TsnStateTreeConditions.cpp

#include "StateTree/TsnStateTreeConditions.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"
#include "Components/TsnStanceObstacleComponent.h"

bool FTsnStateTreeCondition_HasValidTarget::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	const bool bResult = Data.bHasValidTarget;
	return Data.bInvert ? !bResult : bResult;
}

bool FTsnStateTreeCondition_IsInPreEngagementRange::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	const bool bResult = Data.PreEngagementRadius > 0.f
		&& Data.DistanceToTarget2D <= Data.PreEngagementRadius;
	return Data.bInvert ? !bResult : bResult;
}

bool FTsnStateTreeCondition_IsInEngagementRange::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	const bool bResult = Data.EngagementRange > 0.f
		&& Data.DistanceToTarget2D <= Data.EngagementRange;
	return Data.bInvert ? !bResult : bResult;
}

bool FTsnStateTreeCondition_IsTargetStationary::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	const bool bResult = Data.bIsTargetStationary;
	return Data.bInvert ? !bResult : bResult;
}

bool FTsnStateTreeCondition_IsInStanceMode::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	bool bResult = false;
	if (Data.AIController && Data.AIController->GetPawn())
	{
		if (UTsnStanceObstacleComponent* ObsComp =
				Data.AIController->GetPawn()->FindComponentByClass<UTsnStanceObstacleComponent>())
		{
			bResult = ObsComp->GetMobilityStance() == ETsnMobilityStance::Stance;
		}
	}
	return Data.bInvert ? !bResult : bResult;
}
