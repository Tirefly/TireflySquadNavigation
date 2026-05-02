// TsnStateTreeEvaluator_CombatContext.h
// TSN StateTree 评估器——共享战斗上下文

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "TsnStateTreeEvaluator_CombatContext.generated.h"

class AAIController;
class AActor;

USTRUCT()
struct FTsnStateTreeEvaluator_CombatContextInstanceData
{
	GENERATED_BODY()

	/** Context-绑定 AIController */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	/** 当前目标 */
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** 预战斗距离倍数（相对于 AttackRange） */
	UPROPERTY(EditAnywhere, Category = "Parameter",
		meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float PreEngagementRadiusMultiplier = 1.5f;

	/** 输出：目标是否有效（包含 Pawn / 接口校验） */
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasValidTarget = false;

	/** 输出：当前 2D 距离（cm）；目标无效时为 0 */
	UPROPERTY(EditAnywhere, Category = "Output")
	float DistanceToTarget2D = 0.f;

	/** 输出：Pawn 的攻击距离（cm）；目标无效时为 0 */
	UPROPERTY(EditAnywhere, Category = "Output")
	float EngagementRange = 0.f;

	/** 输出：预战斗距离（cm）；目标无效时为 0 */
	UPROPERTY(EditAnywhere, Category = "Output")
	float PreEngagementRadius = 0.f;

	/** 输出：是否已进入攻击距离 */
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsInEngagementRange = false;

	/** 输出：是否已进入预战斗距离 */
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsInPreEngagementRange = false;
};

/**
 * 共享战斗上下文评估器：每帧重算目标有效性、2D 距离、攻击范围、预战斗范围与对应布尔。
 * Transition 与轻量条件应优先消费此评估器输出，避免重复计算。
 */
USTRUCT(meta = (DisplayName = "TSN Combat Context", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeEvaluator_CombatContext : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeEvaluator_CombatContextInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
};
