// TsnStateTreeTask_MoveToEngagementSlot.h
// TSN StateTree 任务节点——预占位接近交战槽位

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "TsnStateTreeTask_MoveToEngagementSlot.generated.h"

class AAIController;
class AActor;
class UTsnEngagementSlotComponent;

USTRUCT()
struct FTsnStateTreeTask_MoveToEngagementSlotInstanceData
{
	GENERATED_BODY()

	/** Context-绑定 AIController，由 AI Schema 自动注入 */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	/** 攻击目标 */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** 槽位到达判定容差（cm），距槽位世界快照 ≤ 此值视为到达 */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 30.f;

	/** 检查重新寻路的间隔（秒） */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.05"))
	float RePathCheckInterval = 0.1f;

	/** 目标移动超过此距离才触发重新寻路 */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float RePathDistanceThreshold = 50.f;

	/** 预占位接近的最大允许时间（秒），0 = 不限时 */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float MaxApproachTime = 5.f;

	/** 运行时缓存：当前持有的槽位组件（用于失败/中断退出时释放） */
	UPROPERTY()
	TWeakObjectPtr<UTsnEngagementSlotComponent> HeldSlotComponent;

	UPROPERTY()
	FVector CachedSlotPosition = FVector::ZeroVector;

	UPROPERTY()
	FVector CachedTargetLocation = FVector::ZeroVector;

	UPROPERTY()
	float CachedEngagementRange = 0.f;

	UPROPERTY()
	float ElapsedApproachTime = 0.f;

	UPROPERTY()
	float TimeSinceLastRePathCheck = 0.f;

	/** 任务是否已申请到槽位（决定 ExitState 时是否需要释放） */
	UPROPERTY()
	bool bSlotAcquired = false;

	/** 本次任务是否以成功结束（成功保留槽位，失败/中断释放槽位） */
	UPROPERTY()
	bool bExitedWithSuccess = false;
};

/**
 * 在攻击距离外申请目标的交战槽位并接近其世界快照。
 *
 * 退出条件（与 BT 版本对齐）：
 * - 到达槽位 → Succeeded（保留槽位）
 * - 中途进入攻击距离 → Succeeded（保留槽位）
 * - MoveTo 失败 / 目标失效 / 超时 → Failed（释放槽位）
 * - 状态被外部中断 → ExitState 释放槽位
 */
USTRUCT(meta = (DisplayName = "TSN Move To Engagement Slot", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeTask_MoveToEngagementSlot : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeTask_MoveToEngagementSlotInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FTsnStateTreeTask_MoveToEngagementSlot();

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
