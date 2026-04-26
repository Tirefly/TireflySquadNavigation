// TsnBTTask_EnterStanceMode.h
// BT 任务节点——进入站姿模式（Header-only）

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AIController.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "TsnBTTask_EnterStanceMode.generated.h"

/** 将单位切换为站姿模式，在 MoveToEngagementSlot 之后执行 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnBTTask_EnterStanceMode : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UTsnBTTask_EnterStanceMode() { NodeName = TEXT("TSN Enter Stance Mode"); }

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override
	{
		AAIController* AICon = OwnerComp.GetAIOwner();
		if (!AICon || !AICon->GetPawn()) return EBTNodeResult::Failed;

		UTsnStanceObstacleComponent* ObsComp =
			AICon->GetPawn()->FindComponentByClass<UTsnStanceObstacleComponent>();
		if (!ObsComp) return EBTNodeResult::Failed;

		ObsComp->EnterStanceMode();
		return EBTNodeResult::Succeeded;
	}
};
