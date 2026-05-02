// TsnTestStateTreeTask_Attack.h
// 测试用 StateTree 任务 —— 模拟攻击计时（仅 Test 模块）

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "TsnTestStateTreeTask_Attack.generated.h"

class AAIController;

/** Attack 任务的实例数据 */
USTRUCT()
struct FTsnTestStateTreeTask_AttackInstanceData
{
	GENERATED_BODY()

	/** Context-绑定的 AIController（由 AI Schema 自动注入） */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	/** 攻击持续时长（秒） */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.1"))
	float AttackDuration = 2.f;

	/** 运行时累计经过时间（秒） */
	UPROPERTY()
	float ElapsedTime = 0.f;
};

/**
 * 测试用 StateTree 任务：模拟攻击过程（计时器）。
 *
 * 行为与 UTsnTestBTTask_Attack 一致；仅用于 demo / 测试。
 * EnterState 启动倒计时；Tick 累计 ElapsedTime，到时返回 Succeeded。
 */
USTRUCT(meta = (DisplayName = "TSN Test Attack", Category = "TireflySquadNavigationTest"))
struct FTsnTestStateTreeTask_Attack : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnTestStateTreeTask_AttackInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FTsnTestStateTreeTask_Attack();

	virtual EStateTreeRunStatus EnterState(
		FStateTreeExecutionContext& Context,
		const FStateTreeTransitionResult& Transition) const override;

	virtual EStateTreeRunStatus Tick(
		FStateTreeExecutionContext& Context,
		const float DeltaTime) const override;
};
