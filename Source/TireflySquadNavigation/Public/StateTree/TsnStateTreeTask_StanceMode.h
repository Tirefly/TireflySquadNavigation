// TsnStateTreeTask_StanceMode.h
// TSN StateTree 任务节点——进入 / 退出站姿

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "TsnStateTreeTask_StanceMode.generated.h"

class AAIController;

USTRUCT()
struct FTsnStateTreeTask_StanceModeInstanceData
{
	GENERATED_BODY()

	/** Context-绑定 AIController，由 AI Schema 自动注入 */
	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<AAIController> AIController = nullptr;
};

/** 进入站姿模式（一次性任务） */
USTRUCT(meta = (DisplayName = "TSN Enter Stance Mode", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeTask_EnterStanceMode : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeTask_StanceModeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FTsnStateTreeTask_EnterStanceMode();

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

/** 退出站姿模式（一次性任务） */
USTRUCT(meta = (DisplayName = "TSN Exit Stance Mode", Category = "TireflySquadNavigation"))
struct TIREFLYSQUADNAVIGATION_API FTsnStateTreeTask_ExitStanceMode : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FTsnStateTreeTask_StanceModeInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	FTsnStateTreeTask_ExitStanceMode();

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
