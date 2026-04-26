// TsnEngagementSlotSubsystem.h
// 全局交战槽位注册表子系统

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TsnEngagementSlotSubsystem.generated.h"

class UTsnEngagementSlotComponent;

/**
 * 全局交战槽位注册表子系统 —— WorldSubsystem（自动创建）。
 *
 * 维护 Requester → SlotComponent 的全局映射，实现"单目标槽位约束"：
 * 同一攻击者同一时间只能在一个目标上持有槽位。
 * TsnEngagementSlotComponent::RequestSlot 内部通过本子系统检查并自动释放旧槽位。
 *
 * 不需要 Tick，仅做注册/查询。
 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnEngagementSlotSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * 注册槽位占用关系。由 TsnEngagementSlotComponent::RequestSlot 内部调用。
	 *
	 * @param Requester 占用槽位的攻击者
	 * @param SlotComp  持有该槽位的组件
	 */
	void RegisterSlotOccupancy(AActor* Requester, UTsnEngagementSlotComponent* SlotComp);

	/**
	 * 注销槽位占用关系。由 TsnEngagementSlotComponent::ReleaseSlot 内部调用。
	 *
	 * @param Requester 释放槽位的攻击者
	 */
	void UnregisterSlotOccupancy(AActor* Requester);

	/**
	 * 查询请求者当前占用的槽位组件。
	 *
	 * @param Requester 查询的攻击者
	 * @return 当前持有槽位的组件指针，若无则返回 nullptr
	 */
	UTsnEngagementSlotComponent* FindOccupiedSlotComponent(AActor* Requester) const;

private:
	/** 全局映射：Requester → 当前持有槽位的 SlotComponent */
	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UTsnEngagementSlotComponent>> SlotOccupancyMap;
};
