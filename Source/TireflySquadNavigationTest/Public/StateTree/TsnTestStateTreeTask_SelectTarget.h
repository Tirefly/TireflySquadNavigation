// TsnTestStateTreeTask_SelectTarget.h
// 测试用 StateTree 任务 —— 搜索最近敌方目标（仅 Test 模块）

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "TsnTestStateTreeTask_SelectTarget.generated.h"

class AAIController;
class AActor;

/** SelectTarget 任务的实例数据 */
USTRUCT()
struct FTsnTestStateTreeTask_SelectTargetInstanceData
{
	GENERATED_BODY()

	/** Context-绑定的 AIController（由 AI Schema 自动注入） */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	/** 最大搜索半径（cm） */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "100.0"))
	float SearchRadius = 5000.f;

	/** 输出：选中的目标（可为 null） */
	UPROPERTY(EditAnywhere, Category = "Output")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** 输出：是否找到目标 */
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasTarget = false;
};

/**
 * 测试用 StateTree 任务：在当前 World 中按 TeamID 选最近敌方目标。
 *
 * 行为与 UTsnTestBTTask_SelectTarget 一致；仅用于 demo / 测试。
 * EnterState 同步返回 Succeeded（命中）或 Failed（无命中）。
 */
USTRUCT(meta = (DisplayName = "TSN Test Select Target", Category = "TireflySquadNavigationTest"))
struct FTsnTestStateTreeTask_SelectTarget : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnTestStateTreeTask_SelectTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FTsnTestStateTreeTask_SelectTarget();

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;
};
