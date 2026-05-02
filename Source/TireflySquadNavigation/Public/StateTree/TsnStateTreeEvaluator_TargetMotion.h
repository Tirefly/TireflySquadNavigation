// TsnStateTreeEvaluator_TargetMotion.h
// TSN StateTree 评估器——目标运动观测

#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "TsnStateTreeEvaluator_TargetMotion.generated.h"

class AActor;

USTRUCT()
struct FTsnStateTreeEvaluator_TargetMotionInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	/** 静止判定阈值（cm/s）；与现有 TsnBTDecorator_IsTargetStationary 一致 */
	UPROPERTY(EditAnywhere, Category = "Parameter", meta = (ClampMin = "0.0"))
	float SpeedThreshold = 10.f;

	/** 输出：目标 2D 速度（cm/s） */
	UPROPERTY(EditAnywhere, Category = "Output")
	float TargetSpeed2D = 0.f;

	/** 输出：目标是否静止（速度 ≤ 阈值） */
	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsTargetStationary = false;
};

/**
 * 目标运动评估器：每帧暴露 2D 速度与“是否静止”观测量。
 */
USTRUCT(meta = (DisplayName = "TSN Target Motion", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeEvaluator_TargetMotion : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeEvaluator_TargetMotionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
};
