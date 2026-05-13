// TsnTacticalMovementComponent.h
// 自定义角色移动组件——合法排斥力注入

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TsnTacticalMovementComponent.generated.h"

class UTsnStanceRepulsionSubsystem;

/**
 * 自定义角色移动组件。
 *
 * 提供合法的排斥力注入接口，在 CalcVelocity（Super 完成后）统一叠加。
 * UE 5.7 中无论使用普通 PathFollowing 还是 DetourCrowd，CalcVelocity 均在
 * PhysWalking 中每帧调用，因此单一注入点即可覆盖所有路径。
 *
 * 站桩排斥力采用"至少 1 帧延迟消费"契约：
 * Subsystem 在帧 N 写入，MovementComponent 在帧 N+1 的首次 CalcVelocity 中消费，
 * 从而彻底消除 Tick 先后不确定性。
 *
 * 第三层防线——站桩 keep-out 防护（Stance Penetration Guard）：
 * CalcVelocity 最终速度确定后，在 RepulsionRadius 内渐进裁剪朝向 Stance 单位的向内分量；
 * 进入 ObstacleRadius 后则完全裁剪，避免继续硬挤静态站桩单位或试图穿过过窄缝隙。
 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnTacticalMovementComponent
	: public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UTsnTacticalMovementComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * 由外部排斥力系统每帧调用，累加当前帧产生的排斥速度增量。
	 * 采用累加语义（+=），允许多个系统在同一帧内安全写入。
	 * 该值不会尝试同帧生效，而是在下一帧首次进入
	 * CalcVelocity（Super 之后）时消费一次后清零。
	 *
	 * @param InRepulsionVelocity 排斥速度增量（调用方已乘 DeltaTime）
	 */
	UFUNCTION(BlueprintCallable, Category = "TireflySquadNavigation|Avoidance")
	void SetRepulsionVelocity(const FVector& InRepulsionVelocity);

	/** 排斥力占最大速度的上限比例，防止排斥力完全压过寻路意图 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MaxRepulsionVelocityRatio = 0.4f;

	/**
	 * 排斥力叠加后允许的超速比例。
	 * 1.0 = 不允许超速，1.1 = 允许 10% 超速（推荐，确保紧凑空间能被有效推开）。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "1.0", ClampMax = "1.5"))
	float OverspeedToleranceRatio = 1.1f;

	/**
	 * 在外层感知带（ObstacleRadius ~ RepulsionRadius）内，
	 * 对朝向站桩单位的速度分量进行的最大裁剪比例。
	 * 0 = 不做外层软裁剪；1 = 靠近 ObstacleRadius 时几乎完全裁掉向内速度。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float OuterBandInwardClipRatio = 0.75f;

	/**
	 * EscapeMode 的持续时间（秒）。
	 * 当路径意图仍然顶向站桩障碍、而最终速度已接近 0 时，
	 * 短时间内改为使用排斥主导的逃逸速度，避免单位原地僵住。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EscapeModeDuration = 0.25f;

	/**
	 * 触发 EscapeMode 时的速度上限比例。
	 * 逃逸阶段直接采用排斥主导的速度，但仍需要限幅，避免瞬间过冲。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float EscapeModeSpeedRatio = 0.65f;

	/**
	 * 触发 EscapeMode 的“几乎停住”阈值（cm/s）。
	 * 最终速度低于此值，同时路径请求与排斥对抗时，视为局部卡死。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "0.0"))
	float EscapeModeStuckSpeedThreshold = 12.f;

	/**
	 * 触发 EscapeMode 所需的最小路径请求速度（cm/s）。
	 * 避免单位本来就接近停下时误进入逃逸模式。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "0.0"))
	float EscapeModeRequestedSpeedThreshold = 80.f;

	/**
	 * 触发 EscapeMode 所需的最小排斥强度占比。
	 * 以 MaxWalkSpeed 为基准，过滤掉微弱排斥造成的正常减速。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EscapeModeMinRepulsionRatio = 0.15f;

	/**
	 * 触发 EscapeMode 的方向对抗阈值。
	 * 请求方向与排斥方向的点积低于此值时，视为“明显对抗”。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float EscapeModeOppositionDotThreshold = -0.15f;

	/**
	 * 进入 EscapeMode 时允许附加在 NavModifier 半径外的缓冲带（cm）。
	 * 单位只要仍压在这条缓冲带内，就说明局部路径仍有较高概率是错误的。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "0.0"))
	float EscapeModeEnterMargin = 20.f;

	/**
	 * 退出 EscapeMode 时使用的释放缓冲带（cm）。
	 * 该值应大于 EnterMargin，形成滞后，避免刚推出一点就立刻恢复旧路径。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "0.0"))
	float EscapeModeReleaseMargin = 45.f;

	/**
	 * 触发 EscapeMode 所需的连续判定帧数。
	 * 用于过滤单帧速度抖动或瞬时接触导致的误触发。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "1", ClampMax = "10"))
	int32 EscapeModeTriggerFrames = 2;

	/**
	 * 退出 EscapeMode 所需的连续清除帧数。
	 * 只有持续离开释放带后，才恢复正常路径驱动。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "1", ClampMax = "10"))
	int32 EscapeModeClearFrames = 2;

	/**
	 * EscapeMode 中保留路径切向分量的比例。
	 * 0 = 纯排斥逃逸；1 = 尽可能保留原路径的绕行切向趋势。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Avoidance",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EscapeModeTangentCarryRatio = 0.35f;

	/**
	 * 单位持续静止多久后输出一次诊断日志（秒）。
	 * 用于定位“任务仍在跑，但单位原地不动”的根因。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Debug",
		meta = (ClampMin = "0.1"))
	float StationaryLogThreshold = 2.f;

	/**
	 * 判定“静止”时使用的二维速度阈值（cm/s）。
	 * 低于此速度且位置几乎不变时，计入静止时长。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Debug",
		meta = (ClampMin = "0.0"))
	float StationaryLogSpeedThreshold = 8.f;

	/**
	 * 判定“静止”时允许的二维位移容差（cm）。
	 * 防止角色轻微抖动导致静止监测被频繁重置。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Debug",
		meta = (ClampMin = "0.0"))
	float StationaryLogDistanceTolerance = 10.f;

	/**
	 * 对象池释放回调。宿主项目在将 Owner 返回对象池时调用此方法。
	 * 等价于 EndPlay 中的清理逻辑，但不依赖 Actor 实际销毁。
	 */
	UFUNCTION(BlueprintCallable, Category = "TireflySquadNavigation|ObjectPool")
	void OnOwnerReleased();

	/**
	 * 当前是否处于 EscapeMode。
	 * 主要供 BT 任务与调试日志读取。
	 */
	UFUNCTION(BlueprintPure, Category = "TireflySquadNavigation|Avoidance")
	bool IsEscapeModeActive() const;

	/**
	 * EscapeMode 剩余持续时间（秒）。
	 * 仅表示最短逃逸时间倒计时，不代表一定会在该时刻立即退出。
	 */
	UFUNCTION(BlueprintPure, Category = "TireflySquadNavigation|Avoidance")
	float GetEscapeModeTimeRemaining() const;

	/**
	 * 最近一次真正消费掉的排斥速度。
	 * 供调试输出使用，帮助判断“没有移动”时是否仍存在强排斥。
	 */
	UFUNCTION(BlueprintPure, Category = "TireflySquadNavigation|Avoidance")
	FVector GetLastConsumedRepulsionVelocity() const;

	/**
	 * 当前 EscapeMode 缓存的逃逸速度。
	 * 供调试输出与可视化观察使用。
	 */
	UFUNCTION(BlueprintPure, Category = "TireflySquadNavigation|Avoidance")
	FVector GetCachedEscapeVelocity() const;

	/**
	 * 供上层 BT 任务消费一次的重寻路请求。
	 * EscapeMode 退出后返回 true 一次，用于触发新的 MoveTo / Repath。
	 */
	bool ConsumePendingEscapeRepathRequest();

protected:
	/**
	 * 排斥力唯一注入点。
	 * UE 5.7 中无论 PathFollowing 还是 CrowdFollowing 最终都经过 PhysWalking → CalcVelocity，
	 * 因此在 Super 完成后统一叠加排斥力，无需额外重写 RequestDirectMove。
	 */
	virtual void CalcVelocity(
		float DeltaTime, float Friction,
		bool bFluid, float BrakingDeceleration) override;

private:
	/** 延迟到下一帧消费的排斥速度增量（消费后清零） */
	FVector DeferredRepulsionVelocity = FVector::ZeroVector;

	/** 是否有待处理的延迟排斥力 */
	bool bHasDeferredRepulsion = false;

	/** 记录排斥力首次写入发生的帧号，后续累加不更新此值（保证消费检测正确） */
	uint64 DeferredRepulsionWriteFrame = 0;

	/** 缓存 RepulsionSubsystem 指针，避免每帧 GetSubsystem 查询 */
	UPROPERTY()
	TObjectPtr<UTsnStanceRepulsionSubsystem> CachedRepulsionSubsystem = nullptr;

	/** 最近一次真正消费掉的排斥速度，供 EscapeMode 和调试使用 */
	FVector LastConsumedRepulsionVelocity = FVector::ZeroVector;

	/** 最近一次消费排斥速度发生的帧号，用于过滤陈旧排斥数据 */
	uint64 LastConsumedRepulsionFrame = 0;

	/** EscapeMode 是否已激活 */
	bool bEscapeModeActive = false;

	/** EscapeMode 退出后等待上层 BT 消费的一次性重寻路请求 */
	bool bPendingEscapeRepathRequest = false;

	/** 最近一帧是否位于 EscapeMode 进入带内 */
	bool bLastFrameInsideEscapeEnterZone = false;

	/** 最近一帧是否位于 EscapeMode 释放带内 */
	bool bLastFrameInsideEscapeReleaseZone = false;

	/** EscapeMode 连续触发帧计数 */
	int32 EscapeModeTriggerFrameCount = 0;

	/** EscapeMode 连续清除帧计数 */
	int32 EscapeModeClearFrameCount = 0;

	/** EscapeMode 剩余持续时间（秒） */
	float EscapeModeTimeRemaining = 0.f;

	/** EscapeMode 当前缓存的主逃逸方向 */
	FVector CachedEscapeDirection = FVector::ZeroVector;

	/** EscapeMode 当前缓存的逃逸速度 */
	FVector CachedEscapeVelocity = FVector::ZeroVector;

	/** 当前静止监测锚点位置 */
	FVector StationaryAnchorLocation = FVector::ZeroVector;

	/** 当前静止累计时长（秒） */
	float StationaryTime = 0.f;

	/** 当前静止阶段是否已经输出过诊断日志 */
	bool bHasLoggedStationary = false;

	/** 对排斥向量做限幅处理 */
	FVector GetClampedRepulsion() const;

	/** 当前帧是否允许消费延迟排斥力 */
	bool CanConsumeDeferredRepulsion() const;

	/** 消费延迟排斥力并清零 */
	void ConsumeDeferredRepulsion();

	/** 根据最近的路径请求与排斥对抗情况，更新 EscapeMode */
	void UpdateEscapeMode(float DeltaTime);

	/** 判断当前局部状态是否满足进入 EscapeMode 的条件 */
	bool ShouldEnterEscapeMode(
		const FVector& RequestedVelocity2D,
		const FVector& Repulsion2D) const;

	/** 基于当前排斥方向计算 EscapeMode 期间使用的逃逸速度 */
	FVector ComputeEscapeVelocity(
		const FVector& RequestedVelocity2D,
		const FVector& EscapeDirection) const;

	/** 更新“持续静止 2 秒以上”的诊断日志状态 */
	void UpdateStationaryLog(float DeltaTime);

	/**
	 * 站桩穿透防护（第三层防线）。
	 * 查询 CachedRepulsionSubsystem 的 Stance 单位列表，
	 * 在 RepulsionRadius 内渐进裁剪最终 Velocity 中朝向该单位的分量，
	 * 进入 ObstacleRadius 后则完全裁剪至零。
	 */
	void ApplyStancePenetrationGuard();
};
