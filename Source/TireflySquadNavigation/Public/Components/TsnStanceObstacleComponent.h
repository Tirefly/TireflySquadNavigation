// TsnStanceObstacleComponent.h
// 站桩障碍物状态管理组件

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TsnStanceObstacleComponent.generated.h"

class UNavModifierComponent;
class UCrowdFollowingComponent;

/**
 * NavModifier 区域类型策略。
 * 决定站姿单位脚下的区域在 NavMesh 中如何标记。
 */
UENUM(BlueprintType)
enum class ETsnNavModifierMode : uint8
{
	/**
	 * 不可通行（推荐：棋盘格、规整开阔地）。
	 * 使用 NavArea_Null，NavMesh 中该区域被完全封禁。
	 */
	Impassable  UMETA(DisplayName = "Impassable"),

	/**
	 * 高代价（适合：复杂地形、走廊、室内）。
	 * 使用 TsnNavArea_StanceUnit，寻路算法优先绕行但不强制。
	 */
	HighCost    UMETA(DisplayName = "High Cost"),
};

/** 单位移动姿态 */
UENUM(BlueprintType)
enum class ETsnMobilityStance : uint8
{
	/** 正在移动，作为正常 Crowd Agent 参与速度协商 */
	Moving      UMETA(DisplayName = "Moving"),
	/** 站桩战斗中，作为动态障碍物 */
	Stance      UMETA(DisplayName = "Stance"),
};

/**
 * 站桩障碍物状态管理组件 —— 挂载在每个 AI 战斗单位身上。
 *
 * 管理单位在"移动 Agent"和"站桩障碍物"之间的导航身份切换：
 * - Moving：正常 DetourCrowd Agent，参与群体寻路和避障
 * - Stance：Crowd 切换为 ObstacleOnly；NavModifier 开启（开关式，非创建/销毁）；
 *           注册到 TsnStanceRepulsionSubsystem
 */
UCLASS(ClassGroup=(TireflySquadNavigation), meta=(BlueprintSpawnableComponent))
class TIREFLYSQUADNAVIGATION_API UTsnStanceObstacleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTsnStanceObstacleComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 进入站姿模式（幂等） */
	UFUNCTION(BlueprintCallable, Category = "TireflySquadNavigation|Navigation")
	void EnterStanceMode();

	/** 退出站姿模式，恢复移动 Agent 身份（幂等） */
	UFUNCTION(BlueprintCallable, Category = "TireflySquadNavigation|Navigation")
	void ExitStanceMode();

	/**
	 * 运行时更新障碍物参数并同步到 NavModifier 和 RepulsionSubsystem。
	 * 宿主项目修改 ObstacleRadius / RepulsionRadius / RepulsionStrength 后调用此方法。
	 */
	UFUNCTION(BlueprintCallable, Category = "TireflySquadNavigation|Navigation")
	void UpdateStanceUnitParams();

	/**
	 * 对象池释放回调。宿主项目在将 Owner 返回对象池时调用此方法。
	 * 等价于 EndPlay 中的清理逻辑，但不依赖 Actor 实际销毁。
	 */
	UFUNCTION(BlueprintCallable, Category = "TireflySquadNavigation|ObjectPool")
	void OnOwnerReleased();

	/**
	 * 获取当前移动姿态。
	 *
	 * @return ETsnMobilityStance::Moving 或 ETsnMobilityStance::Stance
	 */
	UFUNCTION(BlueprintPure, Category = "TireflySquadNavigation|Navigation")
	ETsnMobilityStance GetMobilityStance() const { return CurrentMobilityStance; }

	/**
	 * 获取当前单位实际生效的 TSN 三层半径。
	 * 该接口只读现有配置与合法化结果，不会改变运行时状态。
	 *
	 * @param OutObstacleRadius      站姿障碍内层半径
	 * @param OutNavModifierRadius   导航绕行层半径
	 * @param OutRepulsionRadius     外层排斥半径
	 */
	void GetDebugRadii(
		float& OutObstacleRadius,
		float& OutNavModifierRadius,
		float& OutRepulsionRadius) const;

	/**
	 * 获取当前 Crowd agent 的调试碰撞信息。
	 * 该接口用于统一 overlay 读取 Crowd 半径与查询范围，不改变 Crowd 运行时状态。
	 *
	 * @param OutAgentRadius          Crowd agent 半径
	 * @param OutAgentHalfHeight      Crowd agent 半高
	 * @param OutCollisionQueryRange  Crowd 查询范围
	 * @return true 表示成功读取到 CrowdFollowingComponent 的调试数据
	 */
	bool GetCrowdDebugInfo(
		float& OutAgentRadius,
		float& OutAgentHalfHeight,
		float& OutCollisionQueryRange) const;

	/**
	 * 获取当前 NavModifier 的调试状态。
	 * 该接口只读当前配置和组件状态，不会改变运行时行为。
	 *
	 * @param OutUsesNavModifier   是否启用 NavModifier 能力
	 * @param OutNavModifierActive 当前 NavModifier 是否处于激活状态
	 * @param OutNavModifierMode   当前 NavModifier 的区域策略
	 */
	void GetDebugNavModifierState(
		bool& OutUsesNavModifier,
		bool& OutNavModifierActive,
		ETsnNavModifierMode& OutNavModifierMode) const;

	/** 是否改为使用当前组件的本地 override，而不是跟随 TSN 插件默认值。 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Defaults")
	bool bOverrideTsnDefaults = false;

	/**
	 * 本地 override 的 NavModifier 障碍半径（同时作为排斥力双阶段分界线）。
	 * 仅在 `bOverrideTsnDefaults = true` 时生效。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|NavModifier",
		meta = (ClampMin = "10.0", EditCondition = "bOverrideTsnDefaults"))
	float ObstacleRadius = 60.f;

	/** 本地 override 的 NavModifier 开关。仅在 `bOverrideTsnDefaults = true` 时生效。 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|NavModifier",
		meta = (EditCondition = "bOverrideTsnDefaults"))
	bool bUseNavModifier = true;

	/** 本地 override 的 NavModifier 区域策略。仅在 `bOverrideTsnDefaults = true` 时生效。 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|NavModifier",
		meta = (EditCondition = "bOverrideTsnDefaults && bUseNavModifier"))
	ETsnNavModifierMode NavModifierMode = ETsnNavModifierMode::Impassable;

	/**
	 * 本地 override 的额外导航放大半径。
	 * 仅作用于 NavModifier 的面积，用于让路径规划更早把站桩单位视为绕行障碍，
	 * 不改变排斥力内层边界本身。仅在 `bOverrideTsnDefaults = true` 时生效。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|NavModifier",
		meta = (ClampMin = "0.0", EditCondition = "bOverrideTsnDefaults && bUseNavModifier"))
	float NavModifierExtraRadius = 45.f;

	/**
	 * 本地 override 的排斥力作用半径（必须大于 ObstacleRadius）。
	 * 仅在 `bOverrideTsnDefaults = true` 时生效。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Repulsion",
		meta = (ClampMin = "10.0", EditCondition = "bOverrideTsnDefaults"))
	float RepulsionRadius = 150.f;

	/** 本地 override 的排斥力强度。仅在 `bOverrideTsnDefaults = true` 时生效。 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Repulsion",
		meta = (ClampMin = "0.0", EditCondition = "bOverrideTsnDefaults"))
	float RepulsionStrength = 800.f;

	/** 本地 override 的 NavModifier 延迟关闭时间。仅在 `bOverrideTsnDefaults = true` 时生效。 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|NavModifier",
		meta = (ClampMin = "0.0", EditCondition = "bOverrideTsnDefaults && bUseNavModifier"))
	float NavModifierDeactivationDelay = 0.3f;

private:
	/** 当前最终生效的 stance obstacle 配置。 */
	struct FTsnResolvedStanceObstacleSettings
	{
		/** 是否启用 NavModifier。 */
		bool bUseNavModifier = true;

		/** NavModifier 区域策略。 */
		ETsnNavModifierMode NavModifierMode = ETsnNavModifierMode::Impassable;

		/** 站姿障碍半径。 */
		float ObstacleRadius = 60.f;

		/** 导航外扩半径。 */
		float NavModifierExtraRadius = 45.f;

		/** 排斥感知半径。 */
		float RepulsionRadius = 150.f;

		/** 排斥力强度。 */
		float RepulsionStrength = 800.f;

		/** NavModifier 延迟关闭时间。 */
		float NavModifierDeactivationDelay = 0.3f;
	};

	ETsnMobilityStance CurrentMobilityStance = ETsnMobilityStance::Moving;

	UPROPERTY()
	TObjectPtr<UNavModifierComponent> NavModifierComp = nullptr;

	UPROPERTY()
	TObjectPtr<UCrowdFollowingComponent> CrowdFollowingComp = nullptr;

	FTimerHandle NavModifierDeactivationTimer;

	/** 对象池回收进行中标志，使 ExitStanceMode 跳过延迟定时器 */
	bool bIsReleasingOwner = false;

	/** 解析当前最终生效的 stance obstacle 设置。 */
	void GetResolvedSettings(FTsnResolvedStanceObstacleSettings& OutResolvedSettings) const;

	/** 对 resolved 的 ObstacleRadius / RepulsionRadius 做运行时合法化。 */
	void GetSanitizedRadii(
		const FTsnResolvedStanceObstacleSettings& ResolvedSettings,
		float& OutObstacleRadius,
		float& OutRepulsionRadius) const;

	/**
	 * 获取实际写入 NavModifier 的半径。
	 * 在 ObstacleRadius 基础上叠加额外导航放大值，使寻路层更早绕开过窄缝隙。
	 *
	 * @param ResolvedSettings       当前最终生效的 stance obstacle 设置
	 * @param SanitizedObstacleRadius 已合法化后的障碍半径
	 * @return 实际用于 NavModifier 的影响半径
	 */
	float GetEffectiveNavModifierRadius(
		const FTsnResolvedStanceObstacleSettings& ResolvedSettings,
		float SanitizedObstacleRadius) const;

	/** BeginPlay 或运行时切换时预创建 NavModifier，初始状态关闭。 */
	void InitNavModifier(const FTsnResolvedStanceObstacleSettings& ResolvedSettings);

	/** 将当前 resolved 设置同步到 NavModifier。 */
	void SyncNavModifierConfig(const FTsnResolvedStanceObstacleSettings& ResolvedSettings);

	/** 开启 NavModifier。 */
	void ActivateNavModifier(const FTsnResolvedStanceObstacleSettings& ResolvedSettings);

	/** 关闭 NavModifier。 */
	void DeactivateNavModifier();

	/** 根据 NavModifierMode 设置对应的 NavArea 类。 */
	void ApplyNavAreaClass(ETsnNavModifierMode InNavModifierMode);

	/** 从 Controller 缓存 CrowdFollowingComponent。 */
	void CacheComponents();
};
