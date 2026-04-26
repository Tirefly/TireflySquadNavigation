// TsnTestTargetDummy.h
// 测试用靶标角色 —— 被攻击目标，含槽位环可视化 + 可选 Spline 巡逻

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TsnTestTargetDummy.generated.h"

class UTsnEngagementSlotComponent;
class USplineComponent;

/**
 * 测试用靶标角色。
 *
 * 被攻击目标，挂载 TsnEngagementSlotComponent 供攻击者认领槽位。
 * 支持可选的 Spline 巡逻功能，用于验证移动目标时槽位追踪。
 * 含 DrawDebug 槽位环可视化。
 */
UCLASS()
class ATsnTestTargetDummy : public ACharacter
{
	GENERATED_BODY()

public:
	ATsnTestTargetDummy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// --- 组件 ---

	/** 交战槽位组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TsnTest|Components")
	TObjectPtr<UTsnEngagementSlotComponent> EngagementSlotComp;

	// --- 调试 ---

	/** 是否绘制槽位环可视化 */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Debug")
	bool bDrawDebugSlotRings = true;

	// --- 巡逻 ---

	/** 是否沿 Spline 巡逻 */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Patrol")
	bool bPatrolAlongSpline = false;

	/** 持有 SplineComponent 的 Actor */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Patrol",
		meta = (EditCondition = "bPatrolAlongSpline"))
	TObjectPtr<AActor> SplineActor;

	/** 巡逻速度 (cm/s) */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Patrol",
		meta = (EditCondition = "bPatrolAlongSpline", ClampMin = "0.0"))
	float PatrolSpeed = 200.f;

private:
	/** 解析并缓存巡逻用的 SplineComponent */
	USplineComponent* ResolvePatrolSplineComponent();

	/** 沿 Spline 的当前距离 */
	float SplineDistance = 0.f;

	/** 巡逻移动方向（1 或 -1，用于往返） */
	float PatrolDirection = 1.f;

	/** 缓存已解析到的 SplineComponent，避免每帧重复查找 */
	TWeakObjectPtr<USplineComponent> CachedPatrolSplineComponent;

	/** 巡逻配置非法时只记录一次日志，避免刷屏 */
	bool bHasLoggedInvalidPatrolSetup = false;

#if ENABLE_DRAW_DEBUG
	/** 绘制槽位环和占用信息 */
	void DrawDebugSlotVisualization() const;
#endif
};
