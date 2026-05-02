// TsnStateTreeTask_ReleaseEngagementSlot.cpp

#include "StateTree/TsnStateTreeTask_ReleaseEngagementSlot.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "StateTreeExecutionContext.h"
#include "Components/TsnEngagementSlotComponent.h"

FTsnStateTreeTask_ReleaseEngagementSlot::FTsnStateTreeTask_ReleaseEngagementSlot()
{
	bShouldCallTick = false;
}

EStateTreeRunStatus FTsnStateTreeTask_ReleaseEngagementSlot::EnterState(
	FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& Data = Context.GetInstanceData<FInstanceDataType>(*this);

	// 目标缺失或无槽位组件均视为已释放，不阻塞流程
	if (!Data.AIController || !Data.AIController->GetPawn() || !Data.TargetActor)
	{
		return EStateTreeRunStatus::Succeeded;
	}

	if (UTsnEngagementSlotComponent* SlotComp =
			Data.TargetActor->FindComponentByClass<UTsnEngagementSlotComponent>())
	{
		SlotComp->ReleaseSlot(Data.AIController->GetPawn());
	}
	return EStateTreeRunStatus::Succeeded;
}
