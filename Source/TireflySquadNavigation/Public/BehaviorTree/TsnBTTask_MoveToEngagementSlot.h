// TsnBTTask_MoveToEngagementSlot.h
// BT 任务节点——预占位接近

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "TsnBTTask_MoveToEngagementSlot.generated.h"

/**
 * 在预战斗距离内认领交战槽位并移动至槽位世界快照，目标静止或移动均适用。
 *
 * 若进入时已在攻击距离内，立即 Succeeded（不分配槽位，后续直接站桩攻击）。
 * 否则认领最近空闲槽位并寻路至槽位；槽位身份由目标本地极坐标维持，
 * 目标移动时周期性重算当前世界快照并重寻路。
 * 到达槽位或超时后 Failed（超时为异常兜底），不阻塞攻击流程。
 *
 * 本节点使用 bCreateNodeInstance = true，每个 BehaviorTreeComponent 拥有独立实例，
 * 因此运行时状态（包括委托绑定）可安全存储为类成员变量，无需使用 NodeMemory。
 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnBTTask_MoveToEngagementSlot : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UTsnBTTask_MoveToEngagementSlot();

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

	/** 槽位到达判定容差，距离槽位世界快照 ≤ 此值时视为到达（不要设为 0，以免抖动） */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 30.f;

	/** 检查是否需要刷新槽位世界快照并重新寻路的间隔（秒） */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.1"))
	float RePathCheckInterval = 0.1f;

	/**
	 * 目标移动超过此距离才触发重新寻路。
	 * 建议不大于 2 × AcceptanceRadius，避免长时间追逐旧快照点。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.0"))
	float RePathDistanceThreshold = 50.f;

	/**
	 * 预占位接近阶段的最大允许时间（秒）。
	 * 超时后以 Failed 结束任务（路径异常兜底，即 §4.4 退出条件 c），由 BT 失败分支处理恢复逻辑。
	 * 0 = 不限时（不推荐，路径异常时可能导致单位无限等待）。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.0"))
	float MaxApproachTime = 5.f;

	/**
	 * 当预占位任务仍在 InProgress，但 RequestedVelocity 长时间为 0 时，
	 * 认为当前槽位 MoveTo 意图已静默失效，超过该时长后主动重发一次请求。
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

	/** 缓存所属 BehaviorTreeComponent，供 OnTargetDestroyed 回调使用 */
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;

	/**
	 * 缓存当前绑定委托的目标 Actor。
	 * OnTaskFinished 解绑时直接使用此缓存，避免目标已销毁时 Blackboard 返回 nullptr
	 * 导致 RemoveDynamic 被跳过、产生悬挂委托引用。
	 */
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
	 * @return 是否成功
	 */
	bool RequestSlotAndMove(UBehaviorTreeComponent& OwnerComp, AActor* Target);

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

	/**
	 * 防双触发标志。AActor 被真正销毁时 OnEndPlay 先于 OnDestroyed 触发，
	 * 若两个回调都响应会产生双重释放和双重 FinishLatentTask。
	 * ExecuteTask 入口重置为 false；任一回调先执行后置为 true，后续回调直接 return。
	 */
	bool bIsFinishing = false;

	/** 目标 OnDestroyed 回调——释放槽位并以 Failed 结束任务 */
	UFUNCTION()
	void OnTargetDestroyed(AActor* DestroyedActor);

	/** 目标 OnEndPlay 回调——支持对象池回收场景，释放槽位并以 Failed 结束任务 */
	UFUNCTION()
	void OnTargetEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);
};
