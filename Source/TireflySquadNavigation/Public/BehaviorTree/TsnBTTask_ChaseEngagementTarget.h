// TsnBTTask_ChaseEngagementTarget.h
// BT 任务节点——追击移动中的目标

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "TsnBTTask_ChaseEngagementTarget.generated.h"

/**
 * 追击目标直至进入预战斗距离（AttackRange × PreEngagementRadiusMultiplier），不分配交战槽位。
 *
 * 追击阶段只负责拉近距离，不分配槽位，避免因目标大幅移动导致槽位坐标漂移失效。
 * 进入预战斗距离后 Succeeded，由后续 TsnBTTask_MoveToEngagementSlot 负责槽位认领。
 * 若启动时已在预战斗距离（含已在攻击距离内）则立即 Succeeded。
 *
 * 本节点使用 bCreateNodeInstance = true，每个 BehaviorTreeComponent 拥有独立实例，
 * 因此运行时状态可安全存储为类成员变量，无需使用 NodeMemory。
 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnBTTask_ChaseEngagementTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UTsnBTTask_ChaseEngagementTarget();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** Blackboard 中攻击目标的 Key */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * 预战斗距离倍数，相对于 AttackRange 的倍率。
	 * 追击阶段到达距离 ≤ AttackRange × PreEngagementRadiusMultiplier 时停止。
	 * 推荐 1.3~2.0：值越大，单位越早开始占位，战场布局越提前展开。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float PreEngagementRadiusMultiplier = 1.5f;

	/**
	 * 检查当前 MoveTo 是否因动态 NavMesh / 动态障碍而停住的间隔（秒）。
	 * 若路径跟随已不在 Moving，但仍未进入预战斗距离，则重发一次 MoveTo。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.05"))
	float PathRecoveryInterval = 0.25f;

	/**
	 * 当任务仍在 InProgress，但 RequestedVelocity 长时间为 0 时，
	 * 认为当前追击路径意图已静默失效，超过该时长后主动重发 MoveTo。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.05"))
	float ZeroIntentRecoveryDelay = 0.35f;

	/**
	 * 判定“路径请求速度已经掉空”时使用的阈值（cm/s）。
	 * 低于该值时，视为当前帧几乎没有可用的路径推进意图。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.0"))
	float ZeroIntentRequestedSpeedThreshold = 5.f;

	/**
	 * 判定“单位实际几乎停住”时使用的二维速度阈值（cm/s）。
	 * 仅当请求速度和实际速度都很低时，才进入零意图恢复逻辑。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.0"))
	float ZeroIntentActualSpeedThreshold = 10.f;

private:
	float CachedAttackRange = 150.f;
	float CachedPreEngagementRadius = 225.f;
	float TimeSinceLastPathRecoveryCheck = 0.f;

	/** 连续处于“无路径意图且几乎停住”状态的累计时长（秒） */
	float TimeSinceLastZeroIntentRecoveryCheck = 0.f;

	/**
	 * 从 Blackboard 获取攻击目标 Actor。
	 *
	 * @param OwnerComp 所属行为树组件
	 * @return 目标 Actor，无效时返回 nullptr
	 */
	AActor* GetTargetActor(UBehaviorTreeComponent& OwnerComp) const;

	/**
	 * 发起或重发追击 MoveTo 请求。
	 *
	 * @param OwnerComp 所属行为树组件
	 * @param Target    当前追击目标
	 * @return true 表示请求已成功提交
	 */
	bool RequestChaseMove(UBehaviorTreeComponent& OwnerComp, AActor* Target);
};
