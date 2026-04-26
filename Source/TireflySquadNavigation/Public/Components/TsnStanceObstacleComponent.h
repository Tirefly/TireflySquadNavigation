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
	/**
	 * 每帧绘制站姿障碍调试信息（受调试开关门控）。
	 *
	 * @param DeltaTime         当前帧时间
	 * @param TickType          Tick 类型
	 * @param ThisTickFunction  当前组件 Tick 函数上下文
	 */
	virtual void TickComponent(
		float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
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
	 * NavModifier 障碍半径（同时作为排斥力双阶段分界线）。
	 * 建议设为角色碰撞胶囊半径 × 1.2~1.5。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|NavModifier",
		meta = (ClampMin = "10.0"))
	float ObstacleRadius = 60.f;

	/** 是否使用 NavModifier 影响路径规划 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|NavModifier")
	bool bUseNavModifier = true;

	/** NavModifier 区域策略 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|NavModifier",
		meta = (EditCondition = "bUseNavModifier"))
	ETsnNavModifierMode NavModifierMode = ETsnNavModifierMode::Impassable;

	/**
	 * 额外放大的导航影响半径。
	 * 仅作用于 NavModifier 的面积，用于让路径规划更早把站桩单位视为绕行障碍，
	 * 不改变排斥力内层边界本身。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|NavModifier",
		meta = (ClampMin = "0.0", EditCondition = "bUseNavModifier"))
	float NavModifierExtraRadius = 45.f;

	/**
	 * 排斥力作用半径（必须大于 ObstacleRadius）。
	 * 推荐落在 ObstacleRadius × 1.5~2.5。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Repulsion",
		meta = (ClampMin = "10.0"))
	float RepulsionRadius = 150.f;

	/** 排斥力强度 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Repulsion",
		meta = (ClampMin = "0.0"))
	float RepulsionStrength = 800.f;

	/** 退出站姿模式时，延迟多久关闭 NavModifier */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|NavModifier",
		meta = (ClampMin = "0.0"))
	float NavModifierDeactivationDelay = 0.3f;

	/**
	 * 是否绘制当前单位的 ObstacleRadius、NavModifier 半径和 RepulsionRadius。
	 * 适合直接在 PIE 中观察哪些单位仍把窄缝当成可通行。
	 */
	UPROPERTY(EditAnywhere, Category = "TireflySquadNavigation|Debug")
	bool bDrawDebugObstacle = false;

private:
	ETsnMobilityStance CurrentMobilityStance = ETsnMobilityStance::Moving;

	UPROPERTY()
	TObjectPtr<UNavModifierComponent> NavModifierComp = nullptr;

	UPROPERTY()
	TObjectPtr<UCrowdFollowingComponent> CrowdFollowingComp = nullptr;

	FTimerHandle NavModifierDeactivationTimer;

	/** 对象池回收进行中标志，使 ExitStanceMode 跳过延迟定时器 */
	bool bIsReleasingOwner = false;

	/** 对 ObstacleRadius / RepulsionRadius 做运行时合法化 */
	void GetSanitizedRadii(float& OutObstacleRadius, float& OutRepulsionRadius) const;

	/**
	 * 获取实际写入 NavModifier 的半径。
	 * 在 ObstacleRadius 基础上叠加额外导航放大值，使寻路层更早绕开过窄缝隙。
	 *
	 * @param SanitizedObstacleRadius 已合法化后的障碍半径
	 * @return 实际用于 NavModifier 的影响半径
	 */
	float GetEffectiveNavModifierRadius(float SanitizedObstacleRadius) const;

	/** BeginPlay 时预创建 NavModifier，初始状态关闭 */
	void InitNavModifier();

	/** 开启 NavModifier */
	void ActivateNavModifier();

	/** 关闭 NavModifier */
	void DeactivateNavModifier();

	/** 根据 NavModifierMode 设置对应的 NavArea 类 */
	void ApplyNavAreaClass();

	/** 从 Controller 缓存 CrowdFollowingComponent */
	void CacheComponents();

	/** 绘制当前单位的障碍、导航影响区和状态标签 */
	void DrawDebugObstacleState() const;
};
