// TsnUnitSeparationComponent.h
// 单位分离力组件——处理 Moving↔Moving 的微量分离

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TsnUnitSeparationComponent.generated.h"

class UTsnStanceRepulsionSubsystem;

/**
 * 单位分离力组件 —— 仅处理移动中单位之间的微量分离，防止 Moving↔Moving 重叠。
 *
 * 作为 DetourCrowd 群体避障的补充层，通过 AddMovementInput 注入，
 * 与路径跟随的输入自然融合，在 CalcVelocity 中统一处理。
 *
 * Moving↔Stance 的 keep-out 由 TsnStanceRepulsionSubsystem 独占负责；
 * 若附近存在站桩单位，本组件必须裁剪任何"朝站桩单位内侧"的分离分量，
 * 只保留切向或远离站桩单位的软协调输入。
 * 这一裁剪规则与 Penetration Guard 自洽：排斥力被设计为柔性（Guard 必然存在），
 * 因此软通道必须主动过滤朝内分量，避免产生注定被 Guard 裁掉的无效输入。
 *
 * 本组件为可选组件：若宿主项目的 DetourCrowd 群体避障已能提供足够的单位间距，
 * 可不挂载此组件，其余三层防线（NavModifier、RepulsionSubsystem、PenetrationGuard）零影响。
 */
UCLASS(ClassGroup=(TireflySquadNavigation), meta=(BlueprintSpawnableComponent))
class TIREFLYSQUADNAVIGATION_API UTsnUnitSeparationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTsnUnitSeparationComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** 分离力感知半径（约碰撞半径 2~3 倍） */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Separation",
		meta = (ClampMin = "10.0"))
	float SeparationRadius = 120.f;

	/**
	 * 分离力强度（作为 AddMovementInput 的 ScaleValue）。
	 * 0.3 → 柔和；0.5 → 标准；0.8 → 明显推开。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Separation",
		meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float SeparationStrength = 0.5f;

	/**
	 * 是否绘制分离力调试信息：分离力方向 + Stance 约束裁剪后的最终方向（箭头）。
	 * 在 Shipping 配置中此属性仍存在，但实际绘制代码不会编译进去。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Debug")
	bool bDrawDebugSeparation = false;

	/**
	 * 对象池回收时由宿主调用——停止 Tick 并清空运行时状态，
	 * 配合 Deactivate() 使用，避免对象池中休眠组件仍参与计算。
	 */
	UFUNCTION(BlueprintCallable, Category = "TireflySquadNavigation|Separation")
	void OnOwnerReleased();

private:
	/**
	 * 缓存 TsnStanceRepulsionSubsystem 指针，供 TickComponent 遍历注册列表。
	 * BeginPlay 时赋值，EndPlay / OnOwnerReleased 时清空。
	 */
	UPROPERTY()
	TObjectPtr<UTsnStanceRepulsionSubsystem> CachedRepulsionSubsystem = nullptr;
};
