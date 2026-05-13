// TsnBTDecorator_IsWithinEngagementBand.h
// BT 装饰器节点——判断当前是否已进入接战带

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "TsnBTDecorator_IsWithinEngagementBand.generated.h"

/**
 * 只读判断当前单位是否已经进入当前目标上的合法接战带。
 *
 * 该节点不会认领槽位、不会刷新槽位，也不会发起移动请求。
 * 只有当单位已经在目标上持有槽位时，才会基于当前帧的槽位世界快照
 * 和真实接战半径预算判断是否处于合法接战带内。
 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnBTDecorator_IsWithinEngagementBand : public UBTDecorator
{
	GENERATED_BODY()

public:
	UTsnBTDecorator_IsWithinEngagementBand();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual bool CalculateRawConditionValue(
		UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	/** Blackboard 中攻击目标的 Key */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetKey;

	/**
	 * 接战带到位判定容差。
	 * 最终仍会受真实攻击距离预算约束，避免把已脱离攻击窗的位置误判为合法接战带。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation",
		meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 30.f;
};