// TsnStateTreeTask_ChaseEngagementTarget.h
// TSN StateTree 任务节点——追击移动中的目标至预战斗距离

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "TsnStateTreeTask_ChaseEngagementTarget.generated.h"

class AAIController;
class AActor;

/**
 * 任务实例数据（每个 StateTree 资产实例独立持有）。
 *
 * Context 字段由 `UStateTreeAIComponentSchema` 自动提供，
 * 设计师只需绑定 TargetActor 与可选的 PreEngagementRadiusMultiplier。
 */
USTRUCT()
struct FTsnStateTreeTask_ChaseEngagementTargetInstanceData
{
	GENERATED_BODY()

	/** Context-绑定 AIController，由 AI Schema 自动注入 */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	/** 追击目标 */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<AActor> TargetActor = nullptr;

	/**
	 * 预战斗距离倍数（相对于 AttackRange）；进入此半径后任务 Succeeded。
	 * 与现有 BT 节点语义保持一致。
	 */
	UPROPERTY(EditAnywhere, Category = "Parameter",
		meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float PreEngagementRadiusMultiplier = 1.5f;

	/** 运行时缓存：Pawn 的攻击距离（cm） */
	UPROPERTY()
	float CachedEngagementRange = 0.f;
};

/**
 * 追击目标直至进入预战斗距离（AttackRange × PreEngagementRadiusMultiplier）。
 *
 * - 进入状态时若已在预战斗距离内 → 立即 Succeeded
 * - 目标失效、AIController/Pawn 缺失、Pawn 未实现 ITsnTacticalUnit → Failed
 * - 否则发起 MoveTo，每帧检查距离；到达预战斗距离 → Succeeded
 */
USTRUCT(meta = (DisplayName = "TSN Chase Engagement Target", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeTask_ChaseEngagementTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeTask_ChaseEngagementTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FTsnStateTreeTask_ChaseEngagementTarget();

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
