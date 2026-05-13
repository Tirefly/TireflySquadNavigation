// TsnDebugDrawSubsystem.h
// TSN 统一导航调试绘制子系统

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TsnDebugDrawSubsystem.generated.h"

/**
 * TSN 统一导航调试绘制子系统。
 *
 * 负责在运行时以命令行控制的方式，为实现了 ITsnTacticalUnit 的单位
 * 绘制多层半径 overlay。该子系统只读取现有运行时状态，
 * 不会改变槽位认领、站姿状态或移动请求。
 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnDebugDrawSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 刷新当前帧的 TSN 导航调试绘制。 */
	virtual void Tick(float DeltaTime) override;

	/** 仅在启用统一 overlay 且处于 Game/PIE World 时才参与 Tick。 */
	virtual bool IsTickable() const override;

	/** 返回 Tick 统计 ID。 */
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UTsnDebugDrawSubsystem, STATGROUP_Tickables);
	}
};