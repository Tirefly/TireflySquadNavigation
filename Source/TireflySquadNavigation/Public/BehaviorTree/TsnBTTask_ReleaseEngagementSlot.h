// TsnBTTask_ReleaseEngagementSlot.h
// BT 任务节点——释放交战槽位（Header-only）

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "TsnBTTask_ReleaseEngagementSlot.generated.h"

/** 释放当前持有的交战槽位，失败时也返回 Succeeded（不阻塞流程） */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnBTTask_ReleaseEngagementSlot : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UTsnBTTask_ReleaseEngagementSlot() { NodeName = TEXT("TSN Release Engagement Slot"); }

	/** Blackboard 中攻击目标的 Key */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override
	{
		AAIController* AICon = OwnerComp.GetAIOwner();
		UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
		if (!AICon || !AICon->GetPawn() || !BB) return EBTNodeResult::Succeeded;

		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKey.SelectedKeyName));
		if (!Target) return EBTNodeResult::Succeeded;

		if (UTsnEngagementSlotComponent* SlotComp =
				Target->FindComponentByClass<UTsnEngagementSlotComponent>())
		{
			SlotComp->ReleaseSlot(AICon->GetPawn());
		}
		return EBTNodeResult::Succeeded;
	}
};
