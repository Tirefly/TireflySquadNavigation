// TsnTestBTTask_SelectTarget.h
// 测试用 BT 任务 —— 搜索最近敌方目标

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "TsnTestBTTask_SelectTarget.generated.h"

/**
 * 遍历场景中所有敌方阵营的棋子 / 靶标，选择最近的存活目标。
 *
 * 搜索条件：
 * - ATsnTestChessPiece：TeamID 与自身不同
 * - ATsnTestTargetDummy：始终视为有效目标
 *
 * 同步完成，不使用 InProgress。
 */
UCLASS()
class UTsnTestBTTask_SelectTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UTsnTestBTTask_SelectTarget();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 写入选中目标 Actor 的 Blackboard Key */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	/** 写入是否找到目标的 Blackboard Key */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector HasTargetKey;

	/** 最大搜索半径 (cm) */
	UPROPERTY(EditAnywhere, Category = "TsnTest", meta = (ClampMin = "100.0"))
	float SearchRadius = 5000.f;
};
