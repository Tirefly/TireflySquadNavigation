// TsnBTTask_MoveToEngagementBand.h
// BT 任务节点——推进到接战带

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "TsnBTTask_MoveToEngagementBand.generated.h"

/**
 * 在预战斗距离内认领交战槽位并移动至合法接战带，目标静止或移动均适用。
 *
 * 与兼容节点 `TsnBTTask_MoveToEngagementSlot` 相比，这个节点把职责边界写得更明确：
 * 1. 槽位写入只发生在申请或恢复移动时
 * 2. Tick 阶段只读取当前已认领槽位的世界快照做判定
 * 3. 成功条件优先按“进入合法接战带”判断，而不是精确贴到槽位中心
 *
 * 本节点使用 bCreateNodeInstance = true，每个 BehaviorTreeComponent 拥有独立实例，
 * 因此运行时状态（包括委托绑定）可安全存储为类成员变量，无需使用 NodeMemory。
 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnBTTask_MoveToEngagementBand : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UTsnBTTask_MoveToEngagementBand();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		EBTNodeResult::Type TaskResult) override;

	/** Blackboard 中攻击目标的 Key */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/** 接战带到位判定容差，距离合法接战带半径差值 ≤ 此值时视为到位 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 30.f;

	/** 检查是否需要刷新当前槽位世界快照并重新寻路的间隔（秒） */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.1"))
	float RePathCheckInterval = 0.1f;

	/**
	 * 当前已持有槽位的世界快照移动超过此距离才触发重新寻路。
	 * 建议不大于 2 × AcceptanceRadius，避免长时间追逐旧快照点。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.0"))
	float RePathDistanceThreshold = 50.f;

	/**
	 * 接战带推进阶段的最大允许时间（秒）。
	 * 超时后以 Failed 结束任务（路径异常兜底），由 BT 失败分支处理恢复逻辑。
	 * 0 = 不限时（不推荐，路径异常时可能导致单位无限等待）。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.0"))
	float MaxApproachTime = 5.f;

	/**
	 * 当任务仍在 InProgress，但 RequestedVelocity 长时间为 0 时，
	 * 认为当前接战带推进意图已静默失效，超过该时长后主动重发一次请求。
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
	float TimeSinceLastRePathCheck = 0.f;
	FVector CachedTargetLocation = FVector::ZeroVector;
	float CachedAttackRange = 150.f;
	FVector CachedSlotPosition = FVector::ZeroVector;
	float ElapsedApproachTime = 0.f;

	/** 连续处于“无路径意图且几乎停住”状态的累计时长（秒） */
	float TimeSinceLastZeroIntentRecoveryCheck = 0.f;

	/** 缓存所属 BehaviorTreeComponent，供目标生命周期回调使用 */
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	/** 缓存当前绑定委托的目标 Actor，避免结束任务时丢失解绑入口 */
	TWeakObjectPtr<AActor> CachedTarget;

	/**
	 * 从 Blackboard 获取攻击目标 Actor。
	 *
	 * @param OwnerComp 所属行为树组件
	 * @return 目标 Actor，无效时返回 nullptr
	 */
	AActor* GetTargetActor(UBehaviorTreeComponent& OwnerComp) const;

	/**
	 * 申请槽位并发起 MoveTo 请求。
	 *
	 * @param OwnerComp 所属行为树组件
	 * @param Target    攻击目标
	 * @param bForceRefreshSlotAssignment 是否先释放旧槽位再重新认领
	 * @return 是否成功
	 */
	bool RequestSlotAndMove(
		UBehaviorTreeComponent& OwnerComp,
		AActor* Target,
		bool bForceRefreshSlotAssignment = false);

	/**
	 * 释放当前持有的交战槽位。
	 *
	 * @param OwnerComp 所属行为树组件
	 */
	void ReleaseCurrentSlot(UBehaviorTreeComponent& OwnerComp);

	/** 为当前目标绑定 OnDestroyed / OnEndPlay，先清掉残留绑定，保证幂等 */
	void BindTargetLifecycleDelegates(AActor* Target);

	/** 解绑当前目标的 OnDestroyed / OnEndPlay 委托 */
	void UnbindTargetLifecycleDelegates(AActor* Target);

	/** 防止 OnEndPlay / OnDestroyed 双触发导致重复释放与重复 FinishLatentTask */
	bool bIsFinishing = false;

	/** 目标 OnDestroyed 回调——释放槽位并以 Failed 结束任务 */
	UFUNCTION()
	void OnTargetDestroyed(AActor* DestroyedActor);

	/** 目标 OnEndPlay 回调——支持对象池回收场景，释放槽位并以 Failed 结束任务 */
	UFUNCTION()
	void OnTargetEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
};