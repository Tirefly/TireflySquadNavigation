// TsnStanceRepulsionSubsystem.h
// 站姿单位排斥力子系统

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TsnStanceRepulsionSubsystem.generated.h"

/**
 * 站姿单位注册数据。
 * 结构体不缓存位置，只缓存 Actor 弱引用和注册时确定的参数值。
 * 调用方每帧通过 Obs.Unit->GetActorLocation() 动态读取当前位置。
 */
USTRUCT()
struct TIREFLYSQUADNAVIGATION_API FTsnStanceObstacle
{
	GENERATED_BODY()

	/** 站桩单位的 Actor 弱引用 */
	TWeakObjectPtr<AActor> Unit;

	/** 排斥感知半径 */
	float RepulsionRadius = 0.f;

	/** 排斥力强度 */
	float RepulsionStrength = 0.f;

	/** NavModifier 半径（双阶段排斥力分界线） */
	float NavModifierRadius = 0.f;
};

/**
 * 站姿单位排斥力子系统 —— WorldSubsystem（自动创建）。
 *
 * 职责：
 * 1. 维护所有处于 Stance 模式的单位列表
 * 2. 每帧遍历所有移动中的战斗单位
 * 3. 对接近站桩单位的移动单位施加双阶段排斥力
 * 4. 排斥力通过 TsnTacticalMovementComponent::SetRepulsionVelocity 合法注入
 * 5. Moving↔Stance 的 keep-out 语义仅由本子系统负责
 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnStanceRepulsionSubsystem
	: public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UTsnStanceRepulsionSubsystem, STATGROUP_Tickables);
	}

	/**
	 * 注册站姿单位。由 TsnStanceObstacleComponent::EnterStanceMode 调用。
	 *
	 * @param Unit              注册的 Actor
	 * @param InRepulsionRadius 排斥力感知半径
	 * @param InRepulsionStrength 排斥力强度
	 * @param InNavModifierRadius NavModifier 半径（双阶段分界线）
	 */
	void RegisterStanceUnit(
		AActor* Unit,
		float InRepulsionRadius,
		float InRepulsionStrength,
		float InNavModifierRadius);

	/**
	 * 注销站姿单位。
	 *
	 * @param Unit 要注销的 Actor
	 */
	void UnregisterStanceUnit(AActor* Unit);

	/**
	 * 注册移动单位。由 TsnTacticalMovementComponent::BeginPlay 调用。
	 *
	 * @param Unit 要注册的 Actor
	 */
	void RegisterMovingUnit(AActor* Unit);

	/**
	 * 注销移动单位。
	 *
	 * @param Unit 要注销的 Actor
	 */
	void UnregisterMovingUnit(AActor* Unit);

	/** 获取当前注册的站姿单位数量（调试用） */
	UFUNCTION(BlueprintPure, Category = "TireflySquadNavigation|Debug")
	int32 GetStanceUnitCount() const { return StanceUnits.Num(); }

	/**
	 * 获取当前站姿单位列表。
	 *
	 * @return 站姿单位数组的 const 引用
	 */
	const TArray<FTsnStanceObstacle>& GetStanceUnits() const { return StanceUnits; }

	/**
	 * 获取当前注册的移动单位列表。
	 *
	 * @return 移动单位弱指针数组的 const 引用
	 */
	const TArray<TWeakObjectPtr<AActor>>& GetMovingUnits() const { return MovingUnits; }

	/** 外层排斥力阶段系数（默认 0.1 = RepulsionStrength 的 10%） */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Repulsion",
		meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float OuterRepulsionRatio = 0.1f;

	/** 排斥力方向中切线引导占比 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Repulsion",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TangentBlendRatio = 0.7f;

	/**
	 * 切线选边死区。
	 * 当站桩单位相对移动意图几乎正前方时，左右绕行没有稳定优先级；
	 * 若 Side 落入此死区，则退回纯径向排斥，避免切线方向在两侧来回翻转。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Repulsion",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TangentSideDeadZone = 0.15f;

	/** 是否绘制排斥力调试信息 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Debug")
	bool bDrawDebugRepulsion = false;

	/**
	 * 运行时更新已注册站姿单位的参数。
	 *
	 * @param Unit                 要更新的 Actor
	 * @param InRepulsionRadius    新的排斥感知半径
	 * @param InRepulsionStrength  新的排斥力强度
	 * @param InNavModifierRadius  新的 NavModifier 半径
	 */
	void UpdateStanceUnit(
		AActor* Unit,
		float InRepulsionRadius,
		float InRepulsionStrength,
		float InNavModifierRadius);

private:
	TArray<FTsnStanceObstacle> StanceUnits;

	/** 已注册的移动单位列表 */
	TArray<TWeakObjectPtr<AActor>> MovingUnits;

	void CleanupInvalidEntries();
};
