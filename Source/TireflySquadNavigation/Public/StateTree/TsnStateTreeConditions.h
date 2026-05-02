// TsnStateTreeConditions.h
// TSN StateTree 轻量条件节点集合

#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "TsnStateTreeConditions.generated.h"

class AAIController;

// ============================================================================
// HasValidTarget
// ============================================================================

USTRUCT()
struct FTsnStateTreeCondition_HasValidTargetInstanceData
{
	GENERATED_BODY()

	/** 通常绑定自 CombatContext 评估器输出 */
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bHasValidTarget = false;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "TSN Has Valid Target", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeCondition_HasValidTarget : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeCondition_HasValidTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

// ============================================================================
// IsInPreEngagementRange
// ============================================================================

USTRUCT()
struct FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	float DistanceToTarget2D = 0.f;

	UPROPERTY(EditAnywhere, Category = "Input")
	float PreEngagementRadius = 0.f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "TSN Is In PreEngagement Range", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeCondition_IsInPreEngagementRange : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

// ============================================================================
// IsInEngagementRange
// ============================================================================

USTRUCT()
struct FTsnStateTreeCondition_IsInEngagementRangeInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	float DistanceToTarget2D = 0.f;

	UPROPERTY(EditAnywhere, Category = "Input")
	float EngagementRange = 0.f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "TSN Is In Engagement Range", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeCondition_IsInEngagementRange : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeCondition_IsInEngagementRangeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

// ============================================================================
// IsTargetStationary
// ============================================================================

USTRUCT()
struct FTsnStateTreeCondition_IsTargetStationaryInstanceData
{
	GENERATED_BODY()

	/** 通常绑定自 TargetMotion 评估器输出 */
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bIsTargetStationary = false;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "TSN Is Target Stationary", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeCondition_IsTargetStationary : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeCondition_IsTargetStationaryInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

// ============================================================================
// IsInStanceMode
// ============================================================================

USTRUCT()
struct FTsnStateTreeCondition_IsInStanceModeInstanceData
{
	GENERATED_BODY()

	/** Context-绑定 AIController；运行时直接查询 Pawn 的 TsnStanceObstacleComponent */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

USTRUCT(meta = (DisplayName = "TSN Is In Stance Mode", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeCondition_IsInStanceMode : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeCondition_IsInStanceModeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
