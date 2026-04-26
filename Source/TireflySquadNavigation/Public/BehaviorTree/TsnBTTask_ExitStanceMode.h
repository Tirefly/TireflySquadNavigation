// TsnBTTask_ExitStanceMode.h
// BT 任务节点——退出站姿模式（Header-only）

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AIController.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "TsnBTTask_ExitStanceMode.generated.h"

/** 将单位从站姿模式恢复为移动状态，在攻击结束或切换目标时执行 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnBTTask_ExitStanceMode : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UTsnBTTask_ExitStanceMode() { NodeName = TEXT("TSN Exit Stance Mode"); }

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override
	{
		AAIController* AICon = OwnerComp.GetAIOwner();
		if (!AICon || !AICon->GetPawn()) return EBTNodeResult::Failed;

		UTsnStanceObstacleComponent* ObsComp =
			AICon->GetPawn()->FindComponentByClass<UTsnStanceObstacleComponent>();
		if (!ObsComp) return EBTNodeResult::Failed;

		ObsComp->ExitStanceMode();
		return EBTNodeResult::Succeeded;
	}
};
