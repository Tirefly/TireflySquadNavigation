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
- StateTree 运行时节点：追击目标、移动到槽位、进入/退出站桩、释放槽位，以及战斗上下文/目标运动评估器与相关条件节点

编辑器测试模块 `TireflySquadNavigationTest` 额外提供演示资产和自动化测试，但它是 **Editor-only**，不会进入 Shipping 包。

它还包含测试专用 BT / StateTree 节点，例如 `SelectTarget` 和 `Attack`，这些节点用于演示与测试，不属于运行时插件 API。

## 插件不提供什么

以下内容需要宿主项目自行实现：

- 目标选择逻辑
- 攻击 / 施法 / 伤害 / 仇恨系统
- 完整行为树顶层循环
- UI、特效、动画同步

测试模块里有 `SelectTarget` 和 `Attack` 示例节点，但它们属于演示与测试，不是运行时插件 API 的一部分。

## 目录速览

- `Source/TireflySquadNavigation/`：运行时模块，放可复用组件、子系统、NavArea、BT 与 StateTree 节点
- `Source/TireflySquadNavigationTest/`：Editor-only 测试模块，放演示 Actor、自动化测试和测试专用节点
- `Content/FunctionShowcase/`：演示地图、蓝图、BT/Blackboard 资产
- `Documents/`：技术方案、测试模块说明、纯蓝图与 StateTree 执行指南
- `openspec/`：插件级 OpenSpec 工作区；本仓库默认的 OpenSpec 变更都在这里管理

## 决策层支持

当前插件同时支持 **BehaviorTree** 和 **StateTree** 两条决策接入路径。

- BehaviorTree：适合已经有 Blackboard / BT 体系的宿主项目，插件直接提供接敌、占位、站桩、释放等运行时节点
- StateTree：适合 UE 5.7 下的新流程接入，运行时模块提供可复用节点，测试模块提供最小演示用 `SelectTarget` / `Attack` 节点

如果你要按当前工程里的最小 StateTree 路径复现演示场景，优先看：

- `Documents/StateTree 测试执行指南.md`

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

## LegendsTD-style geometry contract

如果宿主项目采用 LegendsTD 这一类“近战固定 1 格、双方可能互为目标”的接敌模型，TSN 几何不要再依赖隐藏倍率去推导。

推荐把几何配置拆成两层：

1. **宿主 Project Settings**：负责胶囊半径、胶囊半高、格子边长、近战攻击距离这类玩法尺度。
2. **TSN DeveloperSettings 默认值**：由 `UTsnDeveloperSettings` 负责 obstacle / nav modifier / repulsion 的默认几何预算。

单位上的 `UTsnStanceObstacleComponent` 默认跟随 TSN 插件默认值；当组件启用 `bOverrideTsnDefaults` 时，改用该组件自己的本地 override。推荐解析顺序：

1. 单位组件显式 override
2. TSN 插件 `DeveloperSettings` 默认值
3. 插件内部 fallback

对 1 格近战模型，建议把下列语义当作硬约束，而不是经验倍率：

- `CapsuleRadius <= ObstacleRadius < NavModifierRadius <= RepulsionRadius < EngagementRange`
- `ObstacleRadius - CapsuleRadius <= 10cm`
- `RepulsionRadius - ObstacleRadius <= 10cm`
- `NavModifierRadius - ObstacleRadius` 只应保留一层很薄的导航外扩，建议控制在 `0~5cm`
- `EngagementRange - RepulsionRadius >= CapsuleRadius`
- 若两名单位会以相同攻击距离互为目标，还应满足 `2 * RepulsionRadius <= EngagementRange`

最后一条不是“观感建议”，而是避免两个对称近战单位天然产生 repulsion 环重叠的几何前提。若不满足，后续卡住通常不是调 steering 系数能真正修掉的问题。

对这类宿主，不再建议继续使用 `1.35` 一类隐藏倍率作为 obstacle 或 repulsion 的主要推导方式。更稳妥的做法是使用编辑器可见的默认预算和 override 结果，让每一层半径都能被直接读懂。

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

### 命令速查

统一 overlay 的正式控制面全部以 `tsn.debug.*` 开头。下面这张表优先覆盖运行时最常用的命令：

| 命令 | 作用 | 常用值 |
| --- | --- | --- |
| `tsn.debug.DrawUnitOverlay <0/1>` | 开关统一 TSN 单位调试 overlay | `1` 开启，`0` 关闭 |
| `tsn.debug.UnitOverlayPreset <0..3>` | 切换 overlay 预设 | `0=Overview`，`1=Focus`，`2=TSN`，`3=Crowd` |
| `tsn.debug.UnitOverlayMask <Mask>` | 用显式 layer mask 覆盖 preset | `0` 表示继续使用 preset |
| `tsn.debug.UnitOverlayText <0/1>` | 开关文本标签层 | `1` 常开 |
| `tsn.debug.UnitOverlayTargetPair <0/1>` | 是否额外绘制目标侧 slot overlay；只有最终只绘制 1 个单位时才会实际生效 | `1` 开启 |
| `tsn.debug.UnitOverlayFilterMode <0..2>` | 切换范围过滤模式 | `0=All`，`1=Nearest`，`2=Name` |
| `tsn.debug.UnitOverlayLaneSpacing <cm>` | 调整整套 overlay 的层间距 | 例如 `24`、`32` |
| `tsn.debug.UnitOverlaySlotHeightOffset <cm>` | 额外抬高 slot 圈、占据圈和槽位箭头的绘制高度 | 默认 `120`；需要更强可见性时可改成 `180`、`240` |
| `tsn.debug.UnitOverlayMaxUnits <N>` | `All` 模式下限制最多绘制多少个单位 | 例如 `24` |
| `tsn.debug.UnitOverlayFocusName <ActorNameSubstring>` | 将过滤模式切到 `Name`，只看名称包含该片段的单位 | 例如 `BP_Hero_C_0` |
| `tsn.debug.UnitOverlayFocusNearest` | 聚焦离本地相机最近的单位 | 无参数 |
| `tsn.debug.UnitOverlayFocusAll` | 恢复为多单位绘制 | 无参数 |
| `tsn.debug.UnitOverlayClearFocusName` | 清除名称过滤 | 无参数 |

如果你需要手动拼 `UnitOverlayMask`，当前 layer bit 定义如下：

| Bit 值 | Layer |
| --- | --- |
| `1` | Capsule |
| `2` | Crowd |
| `4` | Separation |
| `8` | Engagement |
| `16` | Obstacle |
| `32` | NavModifier |
| `64` | Repulsion |
| `128` | Slots |
| `256` | Text |
| `512` | Vectors |

### 推荐调试方案

如果你只是想快速找到“单位到底在追哪个槽位、当前局部半径关系是否合理”，推荐按下面的顺序做：

1. 先在 PIE 中按 `P`，确认 NavMesh 本身覆盖正常。
2. 开统一 overlay：`tsn.debug.DrawUnitOverlay 1`。
3. 如果你想先看全场总体空间关系，用 `tsn.debug.UnitOverlayPreset 2`。
4. 如果你要深挖一个卡顿单位，切到 `tsn.debug.UnitOverlayPreset 1`。
5. 再执行 `tsn.debug.UnitOverlayFocusNearest` 或 `tsn.debug.UnitOverlayFocusName <ActorNameSubstring>`，把画面收敛到一个单位。
6. 为了同时看到目标侧接战环和当前已分配槽位，执行 `tsn.debug.UnitOverlayTargetPair 1`。
7. `tsn.debug.UnitOverlaySlotHeightOffset` 默认就是 `120`。只有当槽位圈、占据圈和箭头仍然压在模型附近不好看时，再额外执行 `tsn.debug.UnitOverlaySlotHeightOffset 180` 或更大值。
8. 如果整套 overlay 层和层之间还挤在一起，再执行 `tsn.debug.UnitOverlayLaneSpacing 32`。

最常用的一组“单单位看槽位”命令如下：

```text
tsn.debug.DrawUnitOverlay 1
tsn.debug.UnitOverlayPreset 1
tsn.debug.UnitOverlayFocusNearest
tsn.debug.UnitOverlayTargetPair 1
```

如果默认高度 `120` 还不够高，再额外补一条：

```text
tsn.debug.UnitOverlaySlotHeightOffset 180
```

如果你并不关心某一个单位，而是想直接看“全场所有单位”的槽位占据圈和槽位指向箭头，推荐改用下面这组命令：

```text
tsn.debug.DrawUnitOverlay 1
tsn.debug.UnitOverlayFocusAll
tsn.debug.UnitOverlayPreset 2
```

默认情况下这里也会使用 `120` 的 slot 高度偏移；如果你从高机位或大模型场景里仍然看不清，再补一条：

```text
tsn.debug.UnitOverlaySlotHeightOffset 180
```

这组命令的效果是：

1. 对所有接入 TSN 的单位绘制自己的已分配槽位占据圈。
2. 对所有已占槽单位绘制“单位位置 -> 自己槽位中心”的黄色箭头。
3. 不会额外补画每个单位目标侧的完整 slot ring，因为 `UnitOverlayTargetPair` 只有在最终只绘制 1 个单位时才会生效。

如果你想把噪声压到最低，只保留所有单位的槽位层，可以继续执行：

```text
tsn.debug.UnitOverlayMask 128
tsn.debug.UnitOverlayText 0
```

这里的 `128` 就是 `Slots` layer。这样会保留：

1. 已分配槽位中心的小球。
2. 单位最终占据圈。
3. 单位指向槽位中心的箭头。

但不会再显示 capsule、engagement、repulsion、文字和速度向量。

### 颜色与图案图例

统一 overlay 当前使用的颜色和图形语义如下：

| 颜色 / 图案 | 语义 |
| --- | --- |
| 白色圆环 | Capsule 半径 |
| 橙色圆环 | Crowd agent 半径与 Crowd 查询范围 |
| 青色圆环 | Separation 半径 |
| 黄色圆环 | Engagement 半径 |
| 红色圆环 | Stance obstacle 半径 |
| 绿色圆环 | NavModifier 半径 |
| 洋红色圆环 | Repulsion 半径 |
| 蓝色圆环 | 目标侧 slot ring |
| 蓝色小球 | 目标侧其他已占用槽位中心 |
| 黄色小球 | 当前 focus/requester 对应的已分配槽位中心 |
| 黄色占据圈 | 当前单位在该槽位上最终应占据的脚印范围，半径取当前单位胶囊半径 |
| 黄色箭头 | 当前单位位置指向自己已分配槽位中心 |
| 蓝色连线 | 目标中心指向各个目标侧槽位中心 |
| 白色箭头 | 单位当前实际速度 |
| 绿色箭头 | PathFollowing / Movement 请求速度 |
| 红色箭头 | 最近一次消耗的 repulsion 速度 |
| 青色箭头 | EscapeMode 当前缓存速度 |
| 白色文本 | 单位状态摘要、半径数值、NavModifier 状态 |

几条特别容易误读的图形，单独强调如下：

1. `slot ring` 是目标周围的接战环，不是导航终点。
2. 黄色占据圈是“单位最终希望站住后的脚印范围”，不是攻击距离。
3. 黄色箭头是“单位当前位置 -> 已分配槽位中心”的向量，不是当前速度方向。
4. 绿色箭头才是移动系统本帧请求速度；如果黄色箭头和绿色箭头偏差很大，通常意味着局部收尾或恢复逻辑正在打架。
5. 当你在 `All` 模式下看全场单位时，能稳定看到的是“每个单位自己的槽位占据圈和指向箭头”；目标侧整圈 slot ring 只会在单单位绘制时补画。

推荐的验证顺序：

1. 在 PIE 中按 `P` 查看 NavMesh 覆盖
2. 执行 `tsn.debug.DrawUnitOverlay 1`
3. 需要全场观察时，执行 `tsn.debug.UnitOverlayPreset 2`；`2` 对应 TSN 预设，包含接战槽位层
4. 需要单单位诊断时，执行 `tsn.debug.UnitOverlayPreset 1`，再配合 `tsn.debug.UnitOverlayFocusNearest`、`tsn.debug.UnitOverlayFocusName <ActorNameSubstring>` 与 `tsn.debug.UnitOverlayTargetPair 1`
5. 如果目标槽位圈和箭头仍然太低，执行 `tsn.debug.UnitOverlaySlotHeightOffset 180`；若还需要拉开整套 overlay 的层间距，再执行 `tsn.debug.UnitOverlayLaneSpacing 32`
6. 若只想看最近单位的槽位效果，推荐直接连着输入下面这组命令：

```text
tsn.debug.DrawUnitOverlay 1
tsn.debug.UnitOverlayPreset 1
tsn.debug.UnitOverlayFocusNearest
tsn.debug.UnitOverlayTargetPair 1
tsn.debug.UnitOverlaySlotHeightOffset 180
```
7. 用 `FunctionShowcase` 地图先确认基础行为，再接你的宿主项目

旧的组件级 `bDrawDebug...` raw debug 入口已经退役；当前正式调试路径是统一的 TSN overlay 子系统。

如果你要跑自动化测试，请在编辑器中打开：

- `Window -> Developer Tools -> Session Frontend -> Automation`

## 开发工作流

如果你是在这个插件仓库里继续开发 TSN，而不是单纯接入使用，推荐按下面的顺序工作：

1. 先看 `Documents/战场小队导航系统 - 技术方案文档 V2.md`，确认当前架构和术语
2. 若变更属于功能/规格层，进入 `openspec/` 按插件级工作区维护 proposal / spec / archive
3. 改完 C++ 后刷新项目文件，再编译 `SquadNavDevEditor Win64 Development`
4. 先在 `FunctionShowcase` 地图做行为烟雾测试，再视需要跑 Automation

OpenSpec CLI 现在默认应从插件目录执行，例如：

```bash
Push-Location "E:\Projects_Unreal\SquadNavDev\Plugins\TireflySquadNavigation"
openspec list
openspec list --specs
openspec validate --specs --strict --no-interactive
Pop-Location
```

新增或删除 C++ 文件后，先刷新 Unreal 项目文件：

```bash
"E:\UnrealEngine\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="E:\Projects_Unreal\SquadNavDev\SquadNavDev.uproject" -game -rocket -progress
```

推荐的编译验证命令：

```bash
"E:\UnrealEngine\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" SquadNavDevEditor Win64 Development -Project="E:\Projects_Unreal\SquadNavDev\SquadNavDev.uproject" -rocket -progress
```

## 进一步文档

- [战场小队导航系统 - 技术方案文档 V2](Documents/%E6%88%98%E5%9C%BA%E5%B0%8F%E9%98%9F%E5%AF%BC%E8%88%AA%E7%B3%BB%E7%BB%9F%20-%20%E6%8A%80%E6%9C%AF%E6%96%B9%E6%A1%88%E6%96%87%E6%A1%A3%20V2.md)
- [StateTree 测试执行指南](Documents/StateTree%20%E6%B5%8B%E8%AF%95%E6%89%A7%E8%A1%8C%E6%8C%87%E5%8D%97.md)
- [测试模块说明](Documents/%E6%B5%8B%E8%AF%95%E6%A8%A1%E5%9D%97%E8%AF%B4%E6%98%8E.md)
- [BehaviorTree 测试执行指南](Documents/BehaviorTree%20%E6%B5%8B%E8%AF%95%E6%89%A7%E8%A1%8C%E6%8C%87%E5%8D%97.md)

如果你只想快速验证插件是否工作，最短路径是：

1. 打开 `MAP_TsnDemo_Siege`
2. 运行 PIE 观察单位接近、分环、站桩
3. 再用自己的 Character / AIController 按上面的最小模板接入

