// TsnStateTreeTask_ReleaseEngagementSlot.h
// TSN StateTree 任务节点——释放交战槽位

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "TsnStateTreeTask_ReleaseEngagementSlot.generated.h"

class AAIController;
class AActor;

USTRUCT()
struct FTsnStateTreeTask_ReleaseEngagementSlotInstanceData
{
	GENERATED_BODY()

	/** Context-绑定 AIController，由 AI Schema 自动注入 */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	/** 攻击目标 */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<AActor> TargetActor = nullptr;
};

/**
 * 释放当前持有的交战槽位。
 * 即使目标已失效或目标上无 EngagementSlotComponent，也返回 Succeeded，不阻塞流程。
 */
USTRUCT(meta = (DisplayName = "TSN Release Engagement Slot", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeTask_ReleaseEngagementSlot : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeTask_ReleaseEngagementSlotInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FTsnStateTreeTask_ReleaseEngagementSlot();

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
