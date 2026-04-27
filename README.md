# TireflySquadNavigation

`TireflySquadNavigation` 是一个面向 **Unreal Engine 5.7** 的战斗导航插件，解决这类 AI 单位的核心问题：

- 自动接近目标，而不是简单直线硬挤
- 在同一目标周围按攻击距离分配多环站位
- 进入攻击距离后切换为站桩障碍物
- 让移动单位绕开站桩单位，减少穿模、重叠和卡死

它更像是一个“战斗移动层 + 站位层”插件，而不是完整战斗框架。目标选择、攻击判定、伤害结算和状态机仍由宿主项目控制。

## 适用范围

- 自走棋 / 战棋 / 小规模小队战斗
- 十几到几十个单位的围攻与接敌场景
- 目标是正确性和行为稳定性，不是超大规模性能压测

## 插件提供什么

运行时模块 `TireflySquadNavigation` 主要提供以下能力：

- `ITsnTacticalUnit`：宿主单位接口，插件通过它读取攻击距离和站姿状态
- `UTsnTacticalMovementComponent`：自定义移动组件，负责排斥力注入、EscapeMode、站桩 keep-out 防护
- `UTsnStanceObstacleComponent`：在 Moving / Stance 两种导航身份之间切换
- `UTsnEngagementSlotComponent`：把攻击者分配到目标周围的多环槽位上
- `UTsnUnitSeparationComponent`：可选，补充 Moving↔Moving 之间的软分离
- BT 节点：`TsnBTTask_ChaseEngagementTarget`、`TsnBTTask_MoveToEngagementSlot`、`TsnBTTask_EnterStanceMode`、`TsnBTTask_ReleaseEngagementSlot`、`TsnBTTask_ExitStanceMode`
- Decorator：`TsnBTDecorator_IsTargetStationary`

编辑器测试模块 `TireflySquadNavigationTest` 额外提供演示资产和自动化测试，但它是 **Editor-only**，不会进入 Shipping 包。

## 插件不提供什么

以下内容需要宿主项目自行实现：

- 目标选择逻辑
- 攻击 / 施法 / 伤害 / 仇恨系统
- 完整行为树顶层循环
- UI、特效、动画同步

测试模块里有 `SelectTarget` 和 `Attack` 示例节点，但它们属于演示与测试，不是运行时插件 API 的一部分。

## 快速接入

### 1. 启用插件并添加模块依赖

如果你的宿主项目需要在 C++ 中直接使用插件类型，建议在宿主模块的 `.Build.cs` 中加入：

```csharp
PublicDependencyModuleNames.AddRange(
    new string[]
    {
        "Core",
        "NavigationSystem",
        "TireflySquadNavigation",
    });

PrivateDependencyModuleNames.AddRange(
    new string[]
    {
        "CoreUObject",
        "Engine",
        "AIModule",
        "GameplayTasks",
    });
```

如果你只是在编辑器里使用现成蓝图和演示资产，启用插件即可。

### 2. 为战斗单位创建一个 C++ `ACharacter` 基类

这一步是最关键的。`UTsnTacticalMovementComponent` 继承自 `UCharacterMovementComponent`，推荐做法不是在现有蓝图 Character 上硬改，而是创建一个新的 C++ 基类。

攻击者单位至少应满足：

- 实现 `ITsnTacticalUnit`
- 使用 `UTsnTacticalMovementComponent` 作为 CharacterMovementComponent
- 挂载 `UTsnStanceObstacleComponent`
- 如果该单位也能被别人攻击，再挂载 `UTsnEngagementSlotComponent`
- 如需更紧凑的移动单位间距，再挂 `UTsnUnitSeparationComponent`

最小模板可以参考下面这段：

```cpp
#include "GameFramework/Character.h"
#include "Interfaces/ITsnTacticalUnit.h"
#include "Components/TsnTacticalMovementComponent.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "Components/TsnUnitSeparationComponent.h"

class AMySquadUnit : public ACharacter, public ITsnTacticalUnit
{
	GENERATED_BODY()

public:
	AMySquadUnit(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer.SetDefaultSubobjectClass<UTsnTacticalMovementComponent>(
			ACharacter::CharacterMovementComponentName))
	{
		StanceObstacleComp = CreateDefaultSubobject<UTsnStanceObstacleComponent>(TEXT("StanceObstacleComp"));
		EngagementSlotComp = CreateDefaultSubobject<UTsnEngagementSlotComponent>(TEXT("EngagementSlotComp"));
		UnitSeparationComp = CreateDefaultSubobject<UTsnUnitSeparationComponent>(TEXT("UnitSeparationComp"));

		AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	}

	virtual float GetEngagementRange_Implementation() const override
	{
		return EngagementRange;
	}

	virtual bool IsInStanceMode_Implementation() const override
	{
		return StanceObstacleComp
			&& StanceObstacleComp->GetMobilityStance() == ETsnMobilityStance::Stance;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float EngagementRange = 150.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TSN")
	TObjectPtr<UTsnStanceObstacleComponent> StanceObstacleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TSN")
	TObjectPtr<UTsnEngagementSlotComponent> EngagementSlotComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TSN")
	TObjectPtr<UTsnUnitSeparationComponent> UnitSeparationComp;
};
```

### 3. 为 AIController 启用 `UCrowdFollowingComponent`

如果你要使用插件设计里的 Crowd 局部避障，AIController 应切换到 `UCrowdFollowingComponent`。

对正式接入 TSN 的战斗单位来说，`CharacterMovementComponent` 自带的 RVO 已由 `UTsnTacticalMovementComponent` 构造函数从根源关闭，因此 AIController **不需要**再重复关闭一遍。

只有当你让这个 AIController 去控制一个**没有使用** `UTsnTacticalMovementComponent` 的 Pawn 时，才需要额外手动关闭 `bUseRVOAvoidance`。

```cpp
#include "AIController.h"
#include "Navigation/CrowdFollowingComponent.h"

class AMySquadAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMySquadAIController(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(
			TEXT("PathFollowingComponent")))
	{
	}
};
```

### 4. 配置 NavMesh 和站桩障碍策略

运行时站桩障碍需要 NavMesh 支持动态更新。当前开发工程里的配置是：

```ini
[/Script/NavigationSystem.RecastNavMesh]
RuntimeGeneration=Dynamic
bForceRebuildOnLoad=True
```

实际接入时，至少要确认：

- 关卡里有覆盖战场的 `NavMeshBoundsVolume`
- `RecastNavMesh` 的 `RuntimeGeneration` 为 `Dynamic`
- 使用 DetourCrowd 时，角色移动组件的 `bUseRVOAvoidance` 已关闭

`UTsnStanceObstacleComponent` 提供两种 NavModifier 策略：

- `Impassable`：把站桩单位脚下区域视为不可通行，适合棋盘格、开阔地、规则平面
- `HighCost`：允许寻路穿过，但代价更高，适合走廊、复杂室内或不希望完全切断路径的地形

两种模式当前都已在本工程中验证可运行。

### 5. 让目标对象可被围攻

如果一个目标需要被多个攻击者围住，它必须挂载 `UTsnEngagementSlotComponent`。

- 静态靶子 / Dummy：只挂 `UTsnEngagementSlotComponent` 即可
- 可移动且也会被攻击的战斗单位：通常同时实现 `ITsnTacticalUnit` 并挂 `UTsnEngagementSlotComponent`

槽位组件按攻击者的 `GetEngagementRange()` 分环，因此“近战 / 长矛 / 远程”会自然落在不同半径的环上。

### 6. 组装行为树

运行时插件自带的节点只负责“接敌、占位、站桩、释放”，不负责“选谁打”和“打完怎么结算”。推荐流程如下：

1. 你的目标选择逻辑写入 Blackboard 的 `TargetActor`
2. `TsnBTTask_ChaseEngagementTarget`
3. `TsnBTTask_MoveToEngagementSlot`
4. `TsnBTTask_EnterStanceMode`
5. 你的攻击 / 施法 Task
6. `TsnBTTask_ReleaseEngagementSlot`
7. `TsnBTTask_ExitStanceMode`
8. 回到选目标或下一轮战斗循环

最小黑板要求：

- `TargetActor`：运行时 BT 任务真正依赖的目标键

如果你要直接复用测试模块中的演示树，还会用到：

- `HasTarget`
- `BT_Tsn_Test`
- `BB_Tsn_Test`

### 7. 在蓝图中做参数覆写

推荐方式是：

- C++ 里定义一个稳定的宿主 `ACharacter` 基类
- 蓝图里只做兵种参数和资产引用覆写

例如：

- 近战：`EngagementRange = 150`，速度更快
- 长矛：`EngagementRange = 300`
- 远程：`EngagementRange = 600`，速度略慢

这也是 `FunctionShowcase` 演示资产采用的做法。

## 推荐组件组合

### 攻击者单位

- 必须：`UTsnTacticalMovementComponent`
- 必须：`UTsnStanceObstacleComponent`
- 必须：实现 `ITsnTacticalUnit`
- 可选：`UTsnUnitSeparationComponent`
- 若自己也能成为被围攻目标：再挂 `UTsnEngagementSlotComponent`

### 被攻击目标

- 必须：`UTsnEngagementSlotComponent`
- 如果目标本身也是战斗单位，按攻击者配置继续补齐其余组件

## 示例与演示资产

当前仓库已经带了一套可直接参考的演示资产，位于：

- 演示地图：`Content/FunctionShowcase/Map/`
- BT / Blackboard：`Content/FunctionShowcase/AI/BehaviorTree/`
- 示例蓝图：`Content/FunctionShowcase/Blueprint/`

推荐先看这几张地图：

- `MAP_TsnDemo_Siege`：围攻单目标
- `MAP_TsnDemo_Skirmish`：红蓝对战
- `MAP_TsnDemo_MovingTarget`：追击移动目标

## 常见坑

- 不要同时开 `UCrowdFollowingComponent` 和 `bUseRVOAvoidance`，否则会双重避障。
- 不要直接改 `MovementComponent->Velocity` 来实现排斥力。插件的合法入口是 `UTsnTacticalMovementComponent::SetRepulsionVelocity()`。
- 如果站桩单位无法正确变成路径障碍，先检查 `RecastNavMesh.RuntimeGeneration` 是否为 `Dynamic`。
- 如果单位会被对象池回收，记得在回收时调用相关组件的 `OnOwnerReleased()`。
- `UTsnUnitSeparationComponent` 是可选的。若你的 Crowd 参数已经足够稳定，可以不挂。
- 这个插件默认面向小规模战斗，别拿它当几百单位 RTS 的性能方案。

## 调试与验证

推荐的验证顺序：

1. 在 PIE 中按 `P` 查看 NavMesh 覆盖
2. 打开 `UTsnStanceObstacleComponent::bDrawDebugObstacle`
3. 打开 `UTsnTacticalMovementComponent::bDrawDebugRepulsion`
4. 打开 `UTsnEngagementSlotComponent::bDrawDebugSlots`
5. 用 `FunctionShowcase` 地图先确认基础行为，再接你的宿主项目

如果你要跑自动化测试，请在编辑器中打开：

- `Window -> Developer Tools -> Session Frontend -> Automation`

## 进一步文档

- [战场小队导航系统 - 技术方案文档 V2](Documents/%E6%88%98%E5%9C%BA%E5%B0%8F%E9%98%9F%E5%AF%BC%E8%88%AA%E7%B3%BB%E7%BB%9F%20-%20%E6%8A%80%E6%9C%AF%E6%96%B9%E6%A1%88%E6%96%87%E6%A1%A3%20V2.md)
- [测试模块说明](Documents/%E6%B5%8B%E8%AF%95%E6%A8%A1%E5%9D%97%E8%AF%B4%E6%98%8E.md)
- [纯蓝图测试执行指南](Documents/%E7%BA%AF%E8%93%9D%E5%9B%BE%E6%B5%8B%E8%AF%95%E6%89%A7%E8%A1%8C%E6%8C%87%E5%8D%97.md)

如果你只想快速验证插件是否工作，最短路径是：

1. 打开 `MAP_TsnDemo_Siege`
2. 运行 PIE 观察单位接近、分环、站桩
3. 再用自己的 Character / AIController 按上面的最小模板接入

