// TsnEngagementSlotComponent.h
// 交战槽位组件 —— 挂载在可被攻击的目标身上

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TsnEngagementSlotComponent.generated.h"

/**
 * 槽位信息：挂在目标上的本地极坐标（角度 + 半径），不是持久缓存的世界坐标。
 */
USTRUCT()
struct FTsnEngagementSlotInfo
{
	GENERATED_BODY()

	/** 占用者 */
	TWeakObjectPtr<AActor> Occupant;

	/** 在目标周围的本地方位角（度，0~360） */
	float AngleDeg = 0.f;

	/** 本地环半径（由占用者的 GetEngagementRange() 决定） */
	float Radius = 100.f;
};

/**
 * 交战槽位组件 —— 挂载在可被攻击的目标身上。
 *
 * 特性：
 * 1. 多环支持：不同攻击距离的攻击者站在不同的环上
 * 2. 角度分配：从请求者当前位置计算理想角度，在该角度附近
 *    寻找不与同环已有攻击者冲突的空位
 * 3. 容错设计：槽位满时返回合理位置，不阻塞攻击行为
 */
UCLASS(ClassGroup=(TireflySquadNavigation), meta=(BlueprintSpawnableComponent))
class TIREFLYSQUADNAVIGATION_API UTsnEngagementSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTsnEngagementSlotComponent();

	/**
	 * 请求一个交战槽位。
	 * 已有槽位则复用本地极坐标并返回当前帧的世界快照；
	 * 否则在对应环上分配角度最近的空位。
	 *
	 * @param Requester   请求者 Actor
	 * @param AttackRange 请求者自身的交战距离
	 * @return 基于目标当前位置换算出的槽位世界空间快照
	 */
	UFUNCTION(BlueprintCallable, Category = "TireflySquadNavigation|Slots")
	FVector RequestSlot(AActor* Requester, float AttackRange);

	/**
	 * 释放指定请求者持有的槽位。
	 * 攻击者死亡、切换目标或脱战时必须调用。
	 *
	 * @param Requester 要释放槽位的 Actor
	 */
	UFUNCTION(BlueprintCallable, Category = "TireflySquadNavigation|Slots")
	void ReleaseSlot(AActor* Requester);

	/**
	 * 查询指定 Actor 是否持有槽位。
	 *
	 * @param Requester 要查询的 Actor
	 * @return true 表示持有槽位
	 */
	UFUNCTION(BlueprintPure, Category = "TireflySquadNavigation|Slots")
	bool HasSlot(AActor* Requester) const;

	/** 当前已占用槽位数 */
	UFUNCTION(BlueprintPure, Category = "TireflySquadNavigation|Slots")
	int32 GetOccupiedSlotCount() const;

	/**
	 * 查询当前是否有可分配的槽位。
	 * BT Task 在调用 RequestSlot 前应先通过此方法检查；
	 * 槽满时直接返回 Failed，让行为树切换攻击目标。
	 *
	 * @param Requester 请求者 Actor（已持有槽位时始终返回 true）
	 * @return true 表示可分配（已存在槽位或当前槽位未满）
	 */
	UFUNCTION(BlueprintPure, Category = "TireflySquadNavigation|Slots")
	bool IsSlotAvailable(AActor* Requester) const;

	/**
	 * 对象池释放回调。宿主项目在将此目标返回对象池时调用。
	 * 释放所有槽位并通知 TsnEngagementSlotSubsystem 清除关联的占用记录。
	 */
	UFUNCTION(BlueprintCallable, Category = "TireflySquadNavigation|ObjectPool")
	void OnOwnerReleased();

	/** 最大同时攻击者数 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Slots",
		meta = (ClampMin = "1"))
	int32 MaxSlots = 12;

	/**
	 * 槽位半径相对于攻击距离的偏移（负值表示站位比攻击距离稍近）。
	 * 确保站在槽位上时严格在攻击范围内。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Slots")
	float SlotRadiusOffset = -20.f;

	/**
	 * 同环上两个攻击者之间的最小间距（世界单位）。
	 * 用于计算最小角度间隔。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Slots",
		meta = (ClampMin = "10.0"))
	float MinSlotSpacing = 80.f;

	/**
	 * 半径差在此范围内的攻击者被视为"同一环"，需做角度冲突检测。
	 * 建议设为碰撞胶囊直径左右。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Slots",
		meta = (ClampMin = "10.0"))
	float SameRingRadiusTolerance = 60.f;

	/**
	 * 是否绘制槽位调试信息。
	 * 在 Shipping 配置中此属性仍存在但实际绘制代码不会编译进去。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Debug")
	bool bDrawDebugSlots = false;

#if ENABLE_DRAW_DEBUG
	/** 调试绘制所有槽位和各环半径参考圈（受 bDrawDebugSlots 门控） */
	void DrawDebugSlots() const;
#endif

private:
	UPROPERTY()
	TArray<FTsnEngagementSlotInfo> Slots;

	/** 查找请求者已占有的槽位 */
	FTsnEngagementSlotInfo* FindExistingSlot(AActor* Requester);

	/**
	 * 在指定半径的环上，从理想角度开始寻找不冲突的角度。
	 *
	 * 会优先选择导航可达、路径代价更低的候选角度；
	 * 若导航不可用或全部候选都不可达，再退回纯几何角度选择。
	 *
	 * @param Requester    请求槽位的 Actor
	 * @param IdealAngleDeg 理想角度（请求者相对目标的方位角）
	 * @param Radius        环的半径
	 * @return 满足最小间距约束的可用角度
	 */
	float FindUnoccupiedAngle(AActor* Requester, float IdealAngleDeg, float Radius) const;

	/**
	 * 计算指定本地角度和半径在目标当前位置下的世界空间快照。
	 *
	 * @param AngleDeg 角度（度）
	 * @param Radius   半径（cm）
	 * @return 世界空间快照
	 */
	FVector CalculateWorldPosition(float AngleDeg, float Radius) const;

	/** 清理已失效的槽位引用（Occupant 被销毁等） */
	void CleanupInvalidSlots();
};
