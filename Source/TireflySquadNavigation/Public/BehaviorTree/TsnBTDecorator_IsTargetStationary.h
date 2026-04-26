// TsnBTDecorator_IsTargetStationary.h
// BT 装饰器节点——判断目标是否静止

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "TsnBTDecorator_IsTargetStationary.generated.h"

/**
 * 判断目标是否静止（速度 < SpeedThreshold）。
 * 辅助工具节点，供行为树根据目标运动状态微调策略（非主流程必须）。
 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnBTDecorator_IsTargetStationary : public UBTDecorator
{
	GENERATED_BODY()

public:
	UTsnBTDecorator_IsTargetStationary();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual bool CalculateRawConditionValue(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	/** Blackboard 中攻击目标的 Key */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/** 速度低于此阈值视为静止（cm/s） */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "0.0"))
	float SpeedThreshold = 10.f;
};
