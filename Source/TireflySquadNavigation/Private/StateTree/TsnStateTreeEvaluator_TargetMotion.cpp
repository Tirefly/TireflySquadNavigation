// TsnStateTreeEvaluator_TargetMotion.cpp

#include "StateTree/TsnStateTreeEvaluator_TargetMotion.h"
#include "GameFramework/Actor.h"
#include "StateTreeExecutionContext.h"

namespace
{
	static void Evaluate(FTsnStateTreeEvaluator_TargetMotionInstanceData& Data)
	{
		if (!Data.TargetActor)
		{
			Data.TargetSpeed2D = 0.f;
			Data.bIsTargetStationary = false;
			return;
		}

		const float Speed2D = Data.TargetActor->GetVelocity().Size2D();
		Data.TargetSpeed2D = Speed2D;
		Data.bIsTargetStationary = Speed2D <= Data.SpeedThreshold;
	}
}

void FTsnStateTreeEvaluator_TargetMotion::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	Evaluate(Data);
}

void FTsnStateTreeEvaluator_TargetMotion::Tick(
	FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);
	Evaluate(Data);
}
