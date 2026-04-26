// TsnTestBTTask_Attack.h
// 测试用 BT 任务 —— 模拟攻击（异步计时）

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "TsnTestBTTask_Attack.generated.h"

/**
 * 模拟攻击过程，等待指定时长后返回 Succeeded。
 *
 * 异步执行：ExecuteTask 返回 InProgress，
 * TickTask 中倒计时完成后 FinishLatentTask(Succeeded)。
 *
 * 使用 bCreateNodeInstance = true，运行时状态存为成员变量。
 */
UCLASS()
class UTsnTestBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UTsnTestBTTask_Attack();

	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 攻击持续时长（秒） */
	UPROPERTY(EditAnywhere, Category = "TsnTest", meta = (ClampMin = "0.1"))
	float AttackDuration = 2.f;

private:
	/** 剩余攻击时间 */
	float RemainingTime = 0.f;
};
