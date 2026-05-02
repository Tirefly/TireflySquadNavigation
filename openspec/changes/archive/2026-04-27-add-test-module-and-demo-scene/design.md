## Context

插件 `TireflySquadNavigation` 的四层架构（决策→槽位→导航→移动）已全部编码完成（21 个 C++ 文件，0 编译错误）。下一步需要：

1. 验证所有系统在运行时的正确性（NavMesh 寻路、DetourCrowd 避障、多环槽位分配、站桩双层防线、排斥力注入）
2. 提供一个可视化的演示场景，作为插件发布后的功能展示
3. 评估自动化测试的适用性

测试 / 演示代码不应污染核心插件模块，因此放在独立的 Editor-only 模块中。

## Goals / Non-Goals

**Goals:**
- 验证插件四层架构的完整行为正确性
- 提供直观的可视化演示场景，展示自走棋式 AI 移动
- 场景可直接作为插件功能展示 / 教学用例
- 有选择地引入可重复执行的自动化测试

**Non-Goals:**
- 不追求 100% 代码覆盖率（插件面向视觉行为，不是纯逻辑库）
- 不做性能压测（设计定位是小规模战斗）
- 不做跨平台测试（仅 Win64 Editor）
- 不修改任何现有插件核心代码

## Decisions

### 1. 独立 Editor-only 模块

- **决定**：新建 `TireflySquadNavigationTest` 模块，`Type = EditorNoCommandlet`，不打入 Shipping 包
- **原因**：测试 / 演示代码与核心插件隔离，避免发布时带入测试依赖
- **替代方案**：放在宿主项目 Source 中 → 不利于随插件分发

### 2. 测试用 C++ 类而非纯蓝图

- **决定**：核心测试 Character / AIController / 接口实现用 C++ 编写，蓝图仅做参数覆盖层
- **原因**：C++ 层可被自动化测试引用；蓝图层方便设计师在编辑器内调参
- **替代方案**：纯蓝图 → 难以在 Automation 框架中驱动

### 3. 自动化测试策略：功能测试（Functional Test）为主，单元测试为辅

- **决定**：
  - **功能测试（Automation Spec + Functional Test Actor）**：加载 `MAP_TsnDemo_Siege` → 清理预放置展示 Actor → 生成单位 → 等待行为完成 → 验证结果状态（槽位分配、站桩状态、无重叠）
  - **单元测试**：仅覆盖纯计算逻辑（槽位角度计算 `CalculateWorldPosition`、排斥力向量计算）
- **原因**：插件核心价值在于**运行时多系统协同的视觉行为**，单独 Mock 单个组件意义有限；功能测试在真实 NavMesh 环境下验证更贴近实际使用
- **不做的**：不对 BT 节点做隔离单元测试（BT 节点逻辑简单，核心正确性取决于运行时系统协同）

### 4. 演示地图布局

- **决定**：将原本“单图三场景”的演示内容拆分为 3 张独立演示地图：`MAP_TsnDemo_Siege`、`MAP_TsnDemo_Skirmish`、`MAP_TsnDemo_MovingTarget`；每张地图都使用扁平地面、独立 NavMesh 和独立 PlayerStart
- **原因**：`UTsnTestBTTask_SelectTarget` 当前按全 World 搜索目标，且 `ATsnTestTargetDummy` 始终视为有效目标。在同一张地图内共存多个场景时，只能做到软隔离；拆成三张地图可以在不修改 C++ 逻辑的前提下实现硬隔离

## Risks / Trade-offs

- **风险**：Editor-only 模块无法在 Shipping 包中运行自动化测试 → **缓解**：自动化测试仅在 Editor / CI 中运行，符合 UE 标准实践
- **风险**：NavMesh 生成时间影响自动化测试稳定性 → **缓解**：使用静态 NavMesh（地图预烘焙），不依赖运行时动态生成
- **权衡**：功能测试运行较慢（需加载地图）→ 可接受，插件规模小（十几到几十单位），测试执行时间可控

## Open Questions

- ~~是否需要在演示地图中加入简单的 HUD / UI 层（如实时显示槽位占用、排斥力向量）？~~ → **不做 UMG HUD**，全部用 DrawDebug + OnScreenDebugMessage
- ~~是否需要提供一个"一键重置"按钮让用户反复观察不同配置？~~ → 通过 Spawner 的 `bAutoSpawnOnBeginPlay` 开关控制，关闭后可手动在编辑器中调参再 PIE 重启

---

## 行为树详细设计

### Blackboard 定义（BB_Tsn_Test）

你在编辑器中创建 Blackboard Asset 时，添加以下两个键：

| 键名 | 类型 | 用途 |
|------|------|------|
| `TargetActor` | Object（Base Class = Actor） | 当前攻击目标，所有插件 BT 节点通过此键读取目标 |
| `HasTarget` | Bool（默认 false） | 是否有有效目标，用作 Decorator 条件分支 |

### 行为树结构（BT_Tsn_Test）

完整节点连线图：

```
Root
│
└─ Selector                                          ← 顶层：有目标就打，没目标就找
   │
   ├─ [分支 1] Sequence                              ← 有目标时的完整战斗循环
   │  │
   │  ├─ Decorator: Blackboard (HasTarget == true)   ← 引擎内置装饰器
   │  │   FlowControl = Self                         ← 仅守护自身 Sequence
   │  │   Observer Aborts = Lower Priority           ← HasTarget 变 false 时中断低优先级分支
   │  │                                                 （即中断分支2的 SelectTarget）
   │  │
   │  ├─ TsnBTTask_ChaseEngagementTarget             ← 阶段1：远距离追击
   │  │   TargetKey = TargetActor
   │  │   PreEngagementRadiusMultiplier = 1.5
   │  │
   │  ├─ TsnBTTask_MoveToEngagementSlot              ← 阶段2：预占位接近
   │  │   TargetKey = TargetActor
   │  │   AcceptanceRadius = 30
   │  │   RePathCheckInterval = 0.1
   │  │   RePathDistanceThreshold = 50
   │  │   MaxApproachTime = 5.0
   │  │
   │  ├─ TsnBTTask_EnterStanceMode                   ← 阶段3：进入站桩
   │  │
   │  ├─ UTsnTestBTTask_Attack                       ← 模拟攻击（测试模块提供）
   │  │   AttackDuration = 2.0
   │  │
   │  ├─ TsnBTTask_ReleaseEngagementSlot             ← 释放槽位
   │  │   TargetKey = TargetActor
   │  │
   │  └─ TsnBTTask_ExitStanceMode                    ← 退出站桩
   │
   └─ [分支 2] UTsnTestBTTask_SelectTarget           ← 无目标时：搜索最近敌人
      TargetActorKey = TargetActor
      HasTargetKey = HasTarget
```

### 编辑器中的连线步骤

1. **创建 BT 资产**：Content Browser → 右键 → Artificial Intelligence → Behavior Tree → 命名 `BT_Tsn_Test`
2. **设置 Blackboard**：BT 编辑器右上角 → Blackboard Asset 选择 `BB_Tsn_Test`
3. **Root 下拉 Selector**：从 Root 拖出 → Composites → Selector
4. **Selector 左侧（高优先级）拖出 Sequence**：
   - 右键 Sequence → Add Decorator → Blackboard
   - Blackboard Key = `HasTarget`，Key Query = `Is Set`
   - Observer Aborts = `Lower Priority`
5. **Sequence 下依次拖出 6 个子节点**（从左到右）：
   - Tasks → `Tsn BT Task Chase Engagement Target` → 设置 TargetKey = `TargetActor`
   - Tasks → `Tsn BT Task Move To Engagement Slot` → 设置 TargetKey = `TargetActor`
   - Tasks → `Tsn BT Task Enter Stance Mode`
   - Tasks → `Tsn Test BT Task Attack` → 设置 AttackDuration = 2.0
   - Tasks → `Tsn BT Task Release Engagement Slot` → 设置 TargetKey = `TargetActor`
   - Tasks → `Tsn BT Task Exit Stance Mode`
6. **Selector 右侧（低优先级）拖出 SelectTarget**：
   - Tasks → `Tsn Test BT Task Select Target` → 设置 TargetActorKey = `TargetActor`，HasTargetKey = `HasTarget`

### 行为树执行流程说明

**正常循环**：

```
SelectTarget 找到目标 → 设置 HasTarget=true + TargetActor=X
  ↓
Selector 重新评估 → 分支1 的 Decorator 通过（HasTarget==true）
  ↓
Chase(追击) → Succeeded → MoveToSlot(占位) → Succeeded → EnterStance → Attack(2s) → ReleaseSlot → ExitStance
  ↓
Sequence 全部 Succeeded → Selector Succeeded → Root 重启
  ↓
回到 Selector → 分支1 Decorator: HasTarget 仍为 true（上一轮未清除）
  ↓
重新 Chase 同一目标... 或目标已死亡 → Chase Failed → Sequence Failed
  ↓
Selector fallback 到分支2 → SelectTarget 搜索新目标
```

**目标死亡**：
- 如果在 `MoveToEngagementSlot` 执行期间目标被销毁 → `OnTargetDestroyed` 回调 → 释放槽位 + 停止移动 + `Failed`
- Sequence Failed → Selector fallback → SelectTarget 搜索新目标
- SelectTarget 如果找不到任何存活目标 → 设置 HasTarget=false → 分支1 Decorator 不通过 → AI 空闲

**MoveToSlot 的三种退出**：
- 到达槽位 → `Succeeded` → 进入站桩
- 目标主动靠近到攻击距离内 → `Succeeded` → 进入站桩（就地站桩，不需要精确到达槽位）
- 超时 (5s) → `Failed` → Sequence Failed → 重新选目标

### 测试用 BT 节点设计（C++ 实现）

**`UTsnTestBTTask_SelectTarget`**：

```
功能：遍历场景中所有 ATsnTestTargetDummy / ATsnTestChessPiece（敌方阵营），选最近的存活目标
参数：
  - TargetActorKey: FBlackboardKeySelector  → 写入选中的目标 Actor
  - HasTargetKey: FBlackboardKeySelector    → 写入 bool 表示是否找到
  - SearchRadius: float = 5000.0            → 最大搜索半径
返回：
  - 找到目标 → 写入黑板 → Succeeded
  - 无可用目标 → HasTarget=false → Failed
同步完成（无 InProgress）
```

**`UTsnTestBTTask_Attack`**：

```
功能：模拟攻击过程，等待指定时长后返回
参数：
  - AttackDuration: float = 2.0  → 攻击持续时长（秒）
返回：
  - 计时结束 → Succeeded
  - Pawn 无效 → Failed
异步执行（InProgress → TickTask 计时 → Succeeded）
```

---

## 调试可视化系统设计

### 核心原则

**两层可视化策略**，不做 UMG Widget：

1. **组件层（静态身份表示）**：AI Pawn 和 TargetDummy 使用 ACharacter 自带的 `UCapsuleComponent`（`bHiddenInGame = false`），通过 `ShapeColor` 区分阵营——BeginPlay 设置一次后不再修改，无需 Tick。
2. **DrawDebug 层（动态几何叠加）**：纯粹的运行时几何信息（槽位环、连线、文字标签、排斥力箭头）使用 `DrawDebug*` + `GEngine->AddOnScreenDebugMessage()`。

### 组件层：ACharacter 自带胶囊体

`ATsnTestChessPiece` 和 `ATsnTestTargetDummy` 均继承自 `ACharacter`，已自带 `UCapsuleComponent` 作为根组件。**直接配置这个胶囊体为可见**即可，无需创建额外组件：

```cpp
// ATsnTestChessPiece 构造函数
UCapsuleComponent* Capsule = GetCapsuleComponent();
Capsule->SetHiddenInGame(false);            // 游戏中可见
Capsule->SetVisibility(true);
Capsule->ShapeColor = FColor::Red;          // 默认颜色，BeginPlay 中根据 TeamID 覆写
```

**阵营颜色**（在 `BeginPlay` 中一次性设置，之后不再修改）：

| TeamID | ShapeColor | 含义 |
|--------|-----------|------|
| 0 | `FColor::Red` | 红方 |
| 1 | `FColor::Blue` | 蓝方 |

胶囊体颜色**仅表示阵营身份**，BeginPlay 后不再变化，不需要 Tick。

阶段信息（CHASE / APPROACH / STANCE / IDLE）属于动态运行时状态，通过 DrawDebug 层的**文字标签**和**连线颜色**传达（见下方 §阶段信息叠加），不污染胶囊体颜色。

**TargetDummy 的胶囊体**：同理配置 `SetHiddenInGame(false)`，`ShapeColor = FColor::White`（白色），与攻击者明显区分。

### DrawDebug 层：动态几何叠加

以下信息是运行时持续变化的，使用 `DrawDebug*`（Duration=0，每帧绘制）：

### 可视化开关

```cpp
// ATsnTestChessPiece
UPROPERTY(EditAnywhere, Category = "TsnTest|Debug")
bool bDrawDebugSlotInfo = true;     // 绘制头顶文字 + 单位到目标/槽位的连线

// ATsnTestTargetDummy
UPROPERTY(EditAnywhere, Category = "TsnTest|Debug")
bool bDrawDebugSlotRings = true;    // 绘制槽位环（多环圆圈 + 占用标记 + 文字）
```

插件核心组件已有的 Debug 开关（直接利用，无需重复实现）：
- `TsnTacticalMovementComponent::bDrawDebugRepulsion` — 排斥力向量橙色箭头
- `TsnStanceObstacleComponent` 已内置 NavModifier 区域可视化（引擎 `Show Navigation` 即可看到）

### 各层可视化内容

**1. 槽位层可视化**（在 `ATsnTestTargetDummy::Tick` 中绘制，受 `bDrawDebugSlotRings` 控制）：

```
绘制内容                          函数                    颜色         持续时间
─────────────────────────────────────────────────────────────────────────────
每个环的圆圈                      DrawDebugCircle         红/绿/蓝     Duration=0（每帧）
已占用槽位世界位置                 DrawDebugSphere(R=15)   同环颜色     Duration=0
从目标中心到槽位的射线             DrawDebugLine           白色半透明   Duration=0
槽位旁文字 "#1 Melee 45° R=130"  DrawDebugString         白色         Duration=0
目标头顶 "Slots: 4/12"           DrawDebugString         黄色         Duration=0
```

示意：
```
         蓝色圆环 R=580（远程）
      ·──────────────────────·
     ·    绿色圆环 R=280      ·
    ·   ·─────────────────·   ·
   ·   ·  红色圆环 R=130   ·   ·
   ·  ·  ●  [TARGET]  ●  ·  ·      ● = DrawDebugSphere（已占用槽位）
   ·   ·───────────────●──·   ·      ─ = DrawDebugLine（射线）
    ·   ·─────────────────·   ·
     ·────────────●────────·
      ·──────────────────────·
```

**2. 阶段信息叠加**（在 `ATsnTestChessPiece::Tick` 中绘制，受 `bDrawDebugSlotInfo` 控制）：

胶囊体颜色固定为阵营色，阶段信息完全通过 **文字标签颜色 + 连线颜色** 传达：

```
绘制内容                          函数              颜色                 条件
──────────────────────────────────────────────────────────────────────────────
头顶阶段文字                      DrawDebugString   与阶段同色            每帧
  CHASE: "[#1 Melee] CHASE dist=1847"
  APPROACH: "[#1 Melee] APPROACH slot=45° R=130"
  STANCE: "[#1 Melee] STANCE"
单位到目标/槽位的连线              DrawDebugLine     CHASE=黄,APPROACH=青  每帧
```

**3. 站桩防线可视化**（利用已有开关 + 引擎功能）：

- `TsnTacticalMovementComponent::bDrawDebugRepulsion = true` → 排斥力橙色箭头（插件已实现）
- 编辑器菜单 `Show → Navigation` → NavMesh 覆盖区域绿色/灰色（引擎内置）
- NavModifier 标记的 `TsnNavArea_StanceUnit` 区域会显示为不同颜色

**4. Spawner 状态显示**（在 `ATsnTestSpawner::Tick` 中绘制）：

```
DrawDebugString：Spawner 头顶显示 "Spawner [TeamA] Melee×2 + Ranged×2 | Alive: 3/4"
```

---

## Spawner 阵型生成设计

### ATsnTestSpawner 配置参数

```cpp
UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
TSubclassOf<ATsnTestChessPiece> ChessPieceClass;   // 要生成的棋子类（或蓝图子类）

UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
int32 SpawnCount = 4;                               // 生成数量

UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
ETsnTestFormation Formation = ETsnTestFormation::Line; // 阵型类型

UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
float FormationSpacing = 200.0f;                     // 阵型间距 (cm)

UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
bool bAutoSpawnOnBeginPlay = true;                   // 是否 BeginPlay 自动生成

UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
UBehaviorTree* BehaviorTreeAsset;                     // 分配给 AI 的行为树

UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
UBlackboardData* BlackboardAsset;                     // 分配给 AI 的黑板

UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
AActor* InitialTarget;                               // 初始攻击目标（可选，拖入场景中的 TargetDummy）

UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
int32 SpawnTeamID = 0;                               // 生成的单位会被设为此 TeamID
```

### 阵型枚举

```cpp
UENUM(BlueprintType)
enum class ETsnTestFormation : uint8
{
    Line,       // 横排一字排开
    Column,     // 纵列排开
    Grid,       // 方阵（自动计算行列数）
    Circle,     // 圆形环绕 Spawner 中心
};
```

### 阵型计算逻辑

以 Spawner 的 `ActorLocation` 为中心，根据阵型类型计算每个单位的偏移：

**Line（横排）**：
```
索引 i → 偏移 = (0, (i - (N-1)/2) × Spacing, 0)

N=4, Spacing=200:
  (-300, 0)  (-100, 0)  (100, 0)  (300, 0)
       ●         ●         ●         ●
```

**Column（纵列）**：
```
索引 i → 偏移 = ((i - (N-1)/2) × Spacing, 0, 0)

同 Line 但沿 X 轴排列
```

**Grid（方阵）**：
```
Cols = ceil(sqrt(N)), Rows = ceil(N / Cols)
索引 i → Row = i / Cols, Col = i % Cols
偏移 = ((Row - (Rows-1)/2) × Spacing, (Col - (Cols-1)/2) × Spacing, 0)

N=6, Spacing=200:
  ●  ●  ●
  ●  ●  ●
```

**Circle（圆形）**：
```
索引 i → Angle = 360° × i / N
偏移 = (cos(Angle) × Spacing, sin(Angle) × Spacing, 0)

N=6, Spacing=300:
      ●
   ●     ●
   ●     ●
      ●
```

### Spawner 生成流程

```
BeginPlay()
  if (bAutoSpawnOnBeginPlay)
    SpawnAll()

SpawnAll()
  for i = 0..SpawnCount-1:
    Offset = CalculateFormationOffset(i)
    SpawnLocation = GetActorLocation() + Offset
    ChessPiece = SpawnActor<ATsnTestChessPiece>(ChessPieceClass, SpawnLocation)
    ChessPiece->TeamID = SpawnTeamID
    ChessPiece->SpawnDefaultController()  // 自动创建 ATsnTestAIController
    AIController = ChessPiece->GetController<ATsnTestAIController>()
    AIController->RunBehaviorTree(BehaviorTreeAsset)
    if (InitialTarget)
      AIController->GetBlackboardComponent()->SetValueAsObject("TargetActor", InitialTarget)
      AIController->GetBlackboardComponent()->SetValueAsBool("HasTarget", true)
    SpawnedUnits.Add(ChessPiece)
```

---

## 演示地图详细设计

### 地图拆分原则

为避免三个演示场景在同一 World 中发生跨场景选目标，演示内容拆分为三张互相独立的地图：

| 地图 | 用途 | 隔离收益 |
|------|------|---------|
| `MAP_TsnDemo_Siege` | 多单位围攻单目标 | 不会被两组对战单位或移动靶标抢走目标 |
| `MAP_TsnDemo_Skirmish` | 红蓝双方对战 | SelectTarget 只会在本局对战单位中挑选目标 |
| `MAP_TsnDemo_MovingTarget` | 追击移动目标 | 不会误锁定其它地图中的 Dummy 或敌方棋子 |

每张地图都独立包含：

- 扁平地面（4000×4000 cm 级别）
- `NavMeshBoundsVolume` 全覆盖
- `PlayerStart`（便于 PIE 观察）
- 仅当前场景需要的 Spawner / Dummy / Spline Actor

这样即使 `UTsnTestBTTask_SelectTarget` 继续按全 World 搜索，也只能看到当前地图内的测试 Actor，不会串场。

---

### MAP_TsnDemo_Siege：多单位围攻单目标（验证多环槽位分配）

**目的**：验证不同攻击距离的单位能否正确分配到对应环的槽位，无重叠，角度均匀分布。

**地图基础**：

- 地面：平坦 Plane，约 4000×4000 cm
- `NavMeshBoundsVolume`：覆盖整块地面
- `PlayerStart`：建议放在 `(0, -1200, 0)` 左右，朝向场景中心

**场景中摆放的 Actor**：

| Actor | 位置 | 配置 |
|-------|------|------|
| `TargetDummy_1`（BP_TsnTestTargetDummy） | (0, 0, 0) | 静止不动，bDrawDebugSlotRings=true |
| `Spawner_A`（BP_TsnTestSpawner） | (-1400, 0, 0) | 见下方配置 |

**Spawner_A 配置**（在编辑器 Details 面板设置）：

```
ChessPieceClass = BP_TsnTestChessPiece_Melee    ← 第一个 Spawner（近战×2）
SpawnCount = 2
Formation = Line
FormationSpacing = 150
InitialTarget = TargetDummy_1
BehaviorTree = BT_Tsn_Test
Blackboard = BB_Tsn_Test
```

**需要 3 个 Spawner**（因为每个 Spawner 只能配一个棋子蓝图类）：

| Spawner | 棋子类 | 数量 | 位置偏移 |
|---------|--------|------|---------|
| Spawner_A1 | BP_TsnTestChessPiece_Melee | 2 | (-1400, -200) |
| Spawner_A2 | BP_TsnTestChessPiece_Spear | 2 | (-1400, 0) |
| Spawner_A3 | BP_TsnTestChessPiece_Range | 2 | (-1400, 200) |

**PIE 运行时预期行为**：

```
时间线：
0s    6 个棋子生成，开始行为树
0-2s  全部进入 CHASE 阶段，向 TargetDummy_1 移动（头顶黄色文字 + 黄色连线）
2-4s  进入预战斗距离，申请槽位，切换 APPROACH（头顶青色文字 + 青色连线到槽位）
      TargetDummy 周围出现三层圆环（红/绿/蓝）
4-6s  各自到达槽位，进入 STANCE（头顶红色文字，脚下 NavModifier 区域）
      模拟攻击 2 秒
6-8s  释放槽位 → 退出站桩 → 重新 Chase（循环）
```

**手动观察检查清单**：

- [ ] 三层圆环可见（红=近战 R≈130，绿=长矛 R≈280，蓝=远程 R≈580）
- [ ] 每层圆环上的槽位球分布在不同角度（不堆叠）
- [ ] 近战棋子站在最内层，远程站在最外层
- [ ] 攻击后释放槽位 → 圆环上的球消失 → 循环后重新出现

---

### MAP_TsnDemo_Skirmish：两组对战（验证完整战斗循环 + 目标切换）

**目的**：验证双方互为攻击目标时的完整战斗循环——选目标、追击、占位、站桩、攻击、目标死亡后切换。

**地图基础**：

- 地面：平坦 Plane，约 4000×4000 cm
- `NavMeshBoundsVolume`：覆盖整块地面
- `PlayerStart`：建议放在 `(0, -1200, 0)` 左右，朝向战场中央

**场景中摆放的 Actor**：

| Actor | 位置 | 配置 |
|-------|------|------|
| `Spawner_Red`（BP_TsnTestSpawner） | (-1400, 0, 0) | 红方 |
| `Spawner_Blue`（BP_TsnTestSpawner） | (1400, 0, 0) | 蓝方 |

**注意**：此场景中不使用 TargetDummy，而是棋子之间互为攻击目标。`UTsnTestBTTask_SelectTarget` 需要能搜索敌方阵营的棋子。

**阵营区分设计**：在 `ATsnTestChessPiece` 上添加一个 `TeamID` 属性：

```cpp
UPROPERTY(EditAnywhere, Category = "TsnTest")
int32 TeamID = 0;    // 0=红方, 1=蓝方
```

`UTsnTestBTTask_SelectTarget` 搜索时跳过同 TeamID 的单位。

**Spawner 配置**：

| Spawner | 棋子类 | 数量 | 阵型 | 位置 | TeamID |
|---------|--------|------|------|------|--------|
| Spawner_Red1 | BP_TsnTestChessPiece_Melee | 2 | Line | (-1400, -200) | 0 |
| Spawner_Red2 | BP_TsnTestChessPiece_Range | 2 | Line | (-1400, 200) | 0 |
| Spawner_Blue1 | BP_TsnTestChessPiece_Melee | 2 | Line | (1400, -200) | 1 |
| Spawner_Blue2 | BP_TsnTestChessPiece_Range | 2 | Line | (1400, 200) | 1 |

**如何设置 TeamID**：通过 Spawner 的 `SpawnTeamID` 属性，在 `SpawnAll()` 中覆写生成单位的 `TeamID`（见 §Spawner 阵型生成设计）。

**PIE 运行时预期行为**：

```
时间线：
0s    双方各 4 棋子生成，BT 启动（红方红色胶囊，蓝方蓝色胶囊）
0-1s  SelectTarget 选中距离最近的敌方单位
1-3s  全部向各自目标追击（头顶黄色 CHASE 文字 + 黄色连线对冲）
3-5s  预战斗距离内申请槽位，围绕各自目标展开（头顶青色 APPROACH 文字）
5-7s  站桩攻击（头顶红色 STANCE 文字 + NavModifier 区域出现）
      此时场景中间形成一团交错的站桩单位
7+s   攻击完成 → 循环 → 部分目标"死亡"（如果实现 HP）→ 切换目标
```

**手动观察检查清单**：

- [ ] 双方单位不穿过对方站桩单位（排斥力/NavModifier 生效）
- [ ] 近战和远程站在不同距离的环上
- [ ] DetourCrowd 群体避障：多个单位同时移动时不互相卡住
- [ ] 对冲时的路径平滑，无剧烈抖动
- [ ] （如实现 HP 机制）目标死亡后切换到下一个最近敌人

---

### MAP_TsnDemo_MovingTarget：移动目标追击（验证槽位追踪与重算）

**目的**：验证目标持续移动时，攻击者的槽位快照能否正确重算，追击路径不扭曲。

**地图基础**：

- 地面：平坦 Plane，约 4000×4000 cm
- `NavMeshBoundsVolume`：覆盖整块地面
- `PlayerStart`：建议放在 `(0, -1200, 0)` 左右，朝向样条路径中心

**场景中摆放的 Actor**：

| Actor | 位置 | 配置 |
|-------|------|------|
| `TargetDummy_Moving`（BP_TsnTestTargetDummy） | (-1200, 0, 0) | 沿 Spline 巡逻 |
| `Spline_Path`（Spline Component / SplineActor） | — | 从 (-1200,0) 到 (1200,0) 的直线/弧线 |
| `Spawner_C`（BP_TsnTestSpawner） | (-1900, -600, 0) | 追击单位 |

**移动目标实现**：在 `ATsnTestTargetDummy` 上添加可选的巡逻功能：

```cpp
UPROPERTY(EditAnywhere, Category = "TsnTest|Patrol")
bool bPatrolAlongSpline = false;           // 是否沿 Spline 巡逻

UPROPERTY(EditAnywhere, Category = "TsnTest|Patrol")
AActor* SplineActor;                       // 持有 SplineComponent 的 Actor

UPROPERTY(EditAnywhere, Category = "TsnTest|Patrol")
float PatrolSpeed = 200.0f;               // 巡逻速度 (cm/s)
```

在 Tick 中沿 Spline 匀速移动（简单 `SetActorLocation` + SplineComponent 插值）。

**Spawner_C 配置**：

| Spawner | 棋子类 | 数量 | 阵型 | 位置 |
|---------|--------|------|------|------|
| Spawner_C | BP_TsnTestChessPiece_Melee | 4 | Grid | (-1900, -600) |

InitialTarget = TargetDummy_Moving

**PIE 运行时预期行为**：

```
时间线：
0s    TargetDummy 开始沿 Spline 向右移动 (200cm/s)
      4 个近战棋子开始 Chase
2-4s  棋子追上目标，进入预战斗距离，申请槽位
      此时目标仍在移动 → 槽位世界快照随目标偏移
4-6s  MoveToEngagementSlot 的 TickTask 检测到目标移动 > RePathDistanceThreshold
      → 刷新槽位快照 → 重新 MoveTo（新世界坐标）
6+s   棋子不断追踪移动中的目标：
      - 如果追上 → 短暂站桩攻击 → 目标走远 → ExitStance → 重新 Chase
      - 如果追不上 → 持续 APPROACH 阶段
```

**手动观察检查清单**：

- [ ] 追击路径平滑，不出现"追到旧位置后再拐弯"的折线
- [ ] 槽位环跟随 TargetDummy 移动（圆环中心与目标同步）
- [ ] 多个追击单位不互相碰撞（DetourCrowd 有效）
- [ ] 偶尔追上目标时能正常进入站桩 → 攻击 → 目标走远 → 退出站桩 → 继续追
- [ ] MoveToSlot 不会因目标快速移动导致超时 Failed 过于频繁

---

### 手动验证总检查清单

分别 PIE 运行三张演示地图后的完整检查项：

**四层架构验证**：

| # | 检查项 | 在哪张地图观察 | 预期表现 |
|---|--------|---------------|---------|
| 1 | NavMesh 路径规划 | 三张演示地图 | 单位不穿墙、不走出地面 |
| 2 | DetourCrowd 群体避障 | `MAP_TsnDemo_Skirmish` | 多单位同时移动时平滑绕开彼此，无卡死 |
| 3 | 多环槽位分配 | `MAP_TsnDemo_Siege` | 三层圆环可见，不同距离的单位站不同环 |
| 4 | 同环角度间距 | `MAP_TsnDemo_Siege` | 同一环上的槽位均匀分布，无重叠 |
| 5 | NavModifier 障碍 | `MAP_TsnDemo_Skirmish` | Show Navigation 可见站桩单位脚下不可通行区域 |
| 6 | 排斥力防穿模 | `MAP_TsnDemo_Skirmish` | 移动单位从不穿过站桩单位（开启 bDrawDebugRepulsion 观察） |
| 7 | 站桩切换正确性 | 三张演示地图 | EnterStance 后单位停止，ExitStance 后恢复移动 |
| 8 | 目标移动时槽位追踪 | `MAP_TsnDemo_MovingTarget` | 追击路径平滑，圆环跟随目标 |
| 9 | BT 行为循环 | `MAP_TsnDemo_Siege` / `MAP_TsnDemo_Skirmish` | Chase→Slot→Stance→Attack→Release→Exit 循环反复执行 |
| 10 | 目标切换 | `MAP_TsnDemo_Skirmish` | 目标"消失"后 AI 自动搜索新目标 |

**异常情况验证**：

| # | 检查项 | 如何触发 | 预期表现 |
|---|--------|---------|---------|
| 11 | 目标被销毁 | PIE 中手动删除一个 TargetDummy | 持有该目标槽位的单位 Failed → 选新目标 |
| 12 | 所有目标消失 | 删除所有 TargetDummy | AI 空闲（SelectTarget Failed → HasTarget=false） |
| 13 | 槽位满 | 放超过 MaxSlots(12) 个攻击者 | 多余单位 MoveToSlot Failed → 重新尝试或兜底 |
| 14 | 站桩穿透防护 | 高速单位冲向站桩单位 | PenetrationGuard 裁剪速度分量，不穿入 |

所有测试通过 UE 编辑器的 **Session Frontend → Automation** 面板运行。测试注册在 `TireflySquadNavigation.Test` 分类下。

**你的操作步骤**：
1. 打开 UE 编辑器
2. Window → Developer Tools → Session Frontend → Automation 标签页
3. 在左侧树中展开 `TireflySquadNavigation` → `Test`
4. 勾选想运行的测试（或全选）
5. 点击 **Start Tests**
6. 结果面板显示绿色 ✓（Pass）或红色 ✗（Fail）+ 详细日志

**功能测试（测试 3-5）额外可视化**：运行时会自动加载 `MAP_TsnDemo_Siege` 地图，并在初始化阶段清理预放置展示 Actor，再动态生成所有测试 Actor。测试结束后自动销毁。3D 视口中全程用 `DrawDebug*` 函数渲染调试图形。

### 功能测试底图：MAP_TsnDemo_Siege

功能测试直接复用现有展示地图：
1. 使用 `/TireflySquadNavigation/FunctionShowcase/Map/MAP_TsnDemo_Siege`
2. 测试初始化阶段自动销毁预放置的 `ATsnTestSpawner`、`ATsnTestChessPiece`、`ATsnTestTargetDummy`
3. 之后所有测试 Actor 仍由测试代码自动 Spawn 和 Destroy

### 可视化策略

**统一原则**：全部使用 `DrawDebug*` 系列函数（`DrawDebugSphere`、`DrawDebugCircle`、`DrawDebugLine`、`DrawDebugDirectionalArrow`、`DrawDebugString`、`DrawDebugCapsule`、`DrawDebugBox`），不做任何 UMG/HUD Widget。

- **屏幕文字**：用 `DrawDebugString` 在 3D 世界中绘制（附着在 Actor 头顶或固定世界坐标）
- **测试状态信息**：用 `GEngine->AddOnScreenDebugMessage()` 在屏幕左上角显示当前测试阶段 / 倒计时 / 关键指标
- **日志输出**：关键事件打印到 Output Log（`LogTsnTest` 分类）
- **测试完成后**：最后一帧的 Debug 图形保持 5 秒（`DrawDebug*` 的 `Duration` 参数），方便观察最终状态

---

### 测试 1：TsnSlotCalculationTest（单元测试）

**UE 注册路径**：`TireflySquadNavigation.Test.Unit.SlotCalculation`

**原理**：`TsnEngagementSlotComponent::CalculateWorldPosition(AngleDeg, Radius)` 是一个纯数学函数——给定目标 Actor 位置 + 角度 + 半径，返回世界空间中的槽位坐标。不需要 NavMesh、不需要 AI、不需要地图。

**实现方式**：`IMPLEMENT_SIMPLE_AUTOMATION_TEST`，纯内存测试。在测试中创建一个临时 World + 临时 Actor，挂载 `TsnEngagementSlotComponent`，设置 Actor 位置后调用 `CalculateWorldPosition`。

**测试用例**：

| # | 输入 | 预期输出 | 验证什么 |
|---|------|---------|---------|
| 1 | OwnerPos=(0,0,0), Angle=0°, Radius=150 | (150, 0, 0) | 基础 0° 方向 |
| 2 | OwnerPos=(0,0,0), Angle=90°, Radius=150 | (0, 150, 0) | 90° 正交 |
| 3 | OwnerPos=(0,0,0), Angle=180°, Radius=150 | (-150, 0, 0) | 180° 反向 |
| 4 | OwnerPos=(0,0,0), Angle=360°, Radius=150 | (150, 0, 0) | 360° 回绕等价于 0° |
| 5 | OwnerPos=(0,0,0), Angle=-90°, Radius=150 | (0, -150, 0) | 负角度 |
| 6 | OwnerPos=(500,300,0), Angle=45°, Radius=200 | (500+141.4, 300+141.4, 0) | 非原点偏移 |
| 7 | OwnerPos=(0,0,0), Angle=0°, Radius=0 | (0, 0, 0) | 零半径退化 |

**通过标准**：所有用例的输出与预期在 ±1.0cm 容差内一致 → Pass。任一超出 → Fail + 打印「Case #N: Expected (x,y,z), Got (x,y,z), Delta=d」。

**可视化**：无（纯内存计算）。结果在 Automation 面板的日志中逐条显示。

---

### 测试 2：TsnRepulsionVectorTest（单元测试）

**UE 注册路径**：`TireflySquadNavigation.Test.Unit.RepulsionVector`

**原理**：测试排斥力系统的**数学模型**——不启动完整 Tick 循环，而是直接用 `FTsnStanceObstacle` 参数和单位位置，复现 `UTsnStanceRepulsionSubsystem::Tick` 中的力度/方向计算公式，验证双阶段衰减是否正确。

**实现方式**：`IMPLEMENT_SIMPLE_AUTOMATION_TEST`，纯内存测试。测试代码中提取（复现）子系统 Tick 中的计算公式：

```cpp
// 内层：ForceMagnitude = Strength * (1 - Dist/NavModR)^2
// 外层：ForceMagnitude = Strength * OuterRatio * (1 - OuterDist/OuterRange)
// 方向：Tangent*0.7 + Radial*0.3（归一化）
```

**测试用例**：

| # | 场景 | 输入 | 验证重点 |
|---|------|------|---------|
| 1 | 单位在障碍物正中心 | Dist=0, NavModR=60, RepR=150, Strength=800 | 力度 = Strength（最大值），方向为从障碍物中心往外 |
| 2 | 单位在内层中间 | Dist=30 (NavModR=60) | 力度 = 800 × (1-0.5)² = 200，方向含切线分量 |
| 3 | 单位在内层边界 | Dist=60 (NavModR=60) | 内层力度 = 0（刚好衰减到零） |
| 4 | 单位在外层入口 | Dist=60.01 | 进入外层，力度 = 800 × 0.1 × (1-ε) ≈ 80 |
| 5 | 单位在外层中间 | Dist=105 (NavModR=60, RepR=150) | 力度 = 800 × 0.1 × (1 - 45/90) = 40 |
| 6 | 单位在外层边界 | Dist=150 (RepR=150) | 力度 = 0（刚好出界） |
| 7 | 单位在排斥半径外 | Dist=200 | 力度 = 0（不应受到任何排斥） |
| 8 | 内外层过渡不连续性 | Dist=59.99 vs 60.01 | 内层末端 ≈ 0，外层入口 ≈ 80，确认设计中允许的阶跃 |

**通过标准**：力度值在 ±0.5 容差内匹配 → Pass。方向向量验证：径向分量必须背离障碍物（点积 > 0）。

**可视化**：无（纯内存计算）。日志逐条输出每个用例的「Expected magnitude=X, Got=Y, Direction dot=Z」。

---

### 测试 3：SlotAssignmentTest（功能测试 — 直接调用 API，不依赖 BT）

**UE 注册路径**：`TireflySquadNavigation.Test.Functional.SlotAssignment`

**测什么**：`TsnEngagementSlotComponent` 的多环角度分配逻辑——6 个不同攻击距离的请求者申请槽位后，是否正确分环、不重叠。

**关键设计**：**不使用行为树**。直接在代码中调用 `SlotComponent->RequestSlot(Actor, AttackRange)`，绕开所有 AI/寻路依赖，纯测组件逻辑。

**实现方式**：`IMPLEMENT_COMPLEX_AUTOMATION_TEST` + Latent Command 加载地图。

**完全自动化流程**：

```
Latent Step 1: 加载 MAP_TsnDemo_Siege 并清理预放置展示 Actor

Latent Step 2: Spawn + 请求
  ├─ SpawnActor<ATsnTestTargetDummy>(0, 0, 0)   → 拿到 SlotComp
  ├─ SpawnActor<AActor> × 6 个空 Actor 作为请求者
  │   （不需要是 Character，任何 AActor 均可调用 RequestSlot）
  ├─ SlotComp->RequestSlot(Actor1, 150)  → 返回 WorldPos1（近战环）
  ├─ SlotComp->RequestSlot(Actor2, 150)  → 返回 WorldPos2（近战环）
  ├─ SlotComp->RequestSlot(Actor3, 300)  → 返回 WorldPos3（长矛环）
  ├─ SlotComp->RequestSlot(Actor4, 300)  → 返回 WorldPos4（长矛环）
  ├─ SlotComp->RequestSlot(Actor5, 600)  → 返回 WorldPos5（远程环）
  └─ SlotComp->RequestSlot(Actor6, 600)  → 返回 WorldPos6（远程环）

Latent Step 3: 验证 + 可视化
  ├─ GetOccupiedSlotCount() == 6
  ├─ 近战 Actor 的 Slot.Radius ≈ 130 (150 - SlotRadiusOffset 20) ±60容差
  ├─ 长矛 Actor 的 Slot.Radius ≈ 280 ±60容差
  ├─ 远程 Actor 的 Slot.Radius ≈ 580 ±60容差
  ├─ 同环内任意两槽位 |AngleDiff| > 0°（不完全重叠）
  ├─ 绘制 DrawDebug 可视化（Duration=10s，保持可观察）
  └─ 全部通过 → TestTrue / 任一失败 → TestFalse + 日志

Latent Step 4: 清理
  └─ 销毁所有生成的 Actor
```

**DrawDebug 可视化（Duration=10s，测试结束后保持）**：

- `DrawDebugCircle`：以 TargetDummy 为圆心，画三个环
  - 红色环 R≈130（近战）
  - 绿色环 R≈280（长矛）
  - 蓝色环 R≈580（远程）
- `DrawDebugSphere`(R=15)：每个槽位返回的世界坐标画一个实心球，颜色对应所在环
- `DrawDebugLine`：从 TargetDummy 中心到每个槽位画一条射线（方便观察角度分布）
- `DrawDebugString`：每个槽位球旁边标注 `#1 Melee 45° R=130` / `#3 Spear 120° R=280` 等
- `DrawDebugBox`：TargetDummy 位置画一个黄色方块标记

**为什么不需要 BT**：`RequestSlot` 是一个同步 API，输入 (Requester, AttackRange) → 输出 WorldPos。槽位分配的正确性取决于内部的环归类 + 角度间距算法，这些在 API 调用层就能完整验证。BT 只是"什么时候调用 RequestSlot"的时机控制，不影响分配结果。

---

### 测试 4：StanceAvoidanceTest（功能测试 — 直接 MoveToLocation，不依赖 BT）

**UE 注册路径**：`TireflySquadNavigation.Test.Functional.StanceAvoidance`

**测什么**：移动单位能否正确绕开站桩单位（双层防线验证）——不穿模、不卡住、能到达目标。

**关键设计**：**不使用行为树**。直接调用 `AIController->MoveToLocation()` 驱动移动，只验证导航层 + 移动层的避障效果。

**实现方式**：`IMPLEMENT_COMPLEX_AUTOMATION_TEST` + Latent Command。

**完全自动化流程**：

```
Latent Step 1: 加载 MAP_TsnDemo_Siege 并清理预放置展示 Actor

Latent Step 2: Spawn + 配置
  ├─ 在 (0, -200), (0, 0), (0, +200) 生成 3 个 ATsnTestChessPiece（站桩墙）
  ├─ 对 3 个单位调用 StanceObstacleComp->EnterStanceMode()
  │   （激活 NavModifier + 注册排斥力子系统）
  ├─ 等待 0.5 秒让 NavMesh 局部重建完成
  ├─ 在 (-1500, 0) 生成 1 个 ATsnTestChessPiece（Mover）
  ├─ 为 Mover 配置 ATsnTestAIController
  └─ 初始化采样变量：MinDistToAnyStance = MAX_FLT, TrailPoints = []

Latent Step 3: 执行移动（Tick 轮询）
  ├─ Mover 的 AIController->MoveToLocation(FVector(1500, 0, 0))
  ├─ 每帧（Latent Update）：
  │   ├─ 计算 Mover 与每个站桩单位的 2D 距离
  │   ├─ 更新 MinDistToAnyStance
  │   ├─ 每 10 帧记录一个 TrailPoints
  │   ├─ DrawDebug 实时可视化（见下方，Duration=0 每帧刷新）
  │   ├─ AddOnScreenDebugMessage: "Dist to nearest: XXX | Min: XXX | Elapsed: X.Xs"
  │   └─ 检测终止条件：
  │       ├─ Mover 到达 (1500,0,0) ±100cm → 进入验证
  │       └─ 超时 20s → Fail("Mover 被卡住")

Latent Step 4: 验证
  ├─ Mover 到达目标 → Pass 前提
  ├─ MinDistToAnyStance ≥ ObstacleRadius(60cm)
  │   → 通过：双层防线有效
  │   → 失败：Fail("穿模！最小距离=Xcm < ObstacleRadius=60cm")
  └─ 绘制最终结果 DrawDebug（Duration=10s）

Latent Step 5: 清理
  └─ 销毁所有 Actor
```

**DrawDebug 可视化**：

实时（每帧，Duration=0）：
- `DrawDebugCapsule`：每个站桩单位位置画胶囊体（红色）
- `DrawDebugCircle`：每个站桩单位脚下画内层圆（红色, R=ObstacleRadius=60）+ 外层圆（橙色, R=RepulsionRadius=150）
- `DrawDebugSphere`(R=20)：Mover 当前位置（黄色）
- `DrawDebugDirectionalArrow`：Mover 的当前速度方向（黄色箭头）
- `DrawDebugString`：Mover 头顶 `"Dist: 87cm | Min: 62cm"`

最终结果（Duration=10s，测试结束后保持）：
- `DrawDebugLine`：TrailPoints 连线（绿色折线 = 完整运动轨迹）
- `DrawDebugSphere`(R=10)：起点标记（白色）+ 终点标记（绿色）
- `DrawDebugString`：终点旁标注 `"PASS: MinDist=62cm >= 60cm"` 或 `"FAIL: MinDist=48cm < 60cm"`（红色）

---

### 测试 5：FullBattleCycleTest（功能测试 — 唯一使用 BT 的集成测试）

**UE 注册路径**：`TireflySquadNavigation.Test.Functional.FullBattleCycle`

**测什么**：完整三阶段战斗循环端到端集成——追击 → 预占位 → 站桩攻击 → 释放 → 退出站桩。这是唯一需要 BT 的测试，因为它验证的就是整个 BT 驱动的行为管线。

**实现方式**：`IMPLEMENT_COMPLEX_AUTOMATION_TEST` + Latent Command。

**前提**：需要 `BT_Tsn_Test` 和 `BB_Tsn_Test` 资产已创建（`[MANUAL]` 任务 3.1 + 3.2）。测试代码通过 `FSoftObjectPath` 异步加载这些资产。

**完全自动化流程**：

```
Latent Step 1: 加载 MAP_TsnDemo_Siege 并清理预放置展示 Actor

Latent Step 2: 加载 BT/BB 资产
  ├─ LoadObject<UBehaviorTree>(BT_Tsn_Test 路径)
  └─ LoadObject<UBlackboardData>(BB_Tsn_Test 路径)

Latent Step 3: Spawn + 配置
  ├─ 在 (0, 0) 生成 1 个 ATsnTestTargetDummy
  ├─ 在 (-3000, y) 生成 4 个 ATsnTestChessPiece（间距 200cm）
  │   Unit1,2: EngagementRange=150（近战）
  │   Unit3,4: EngagementRange=600（远程）
  ├─ 为每个单位 SpawnDefaultController → ATsnTestAIController
  ├─ 设置黑板 TargetActor → TargetDummy
  ├─ 启动行为树
  └─ 初始化追踪：Phase[4] = CHASE, bEverReachedStance[4] = false, PeakSlotCount = 0

Latent Step 4: 执行战斗循环（Tick 轮询，超时 25s）
  每帧：
  ├─ 对每个单位判断当前阶段：
  │   ├─ Dist > PreEngagementRadius → CHASE
  │   ├─ AttackRange < Dist ≤ PreEngagement → APPROACH
  │   └─ IsInStanceMode() == true → STANCE
  ├─ 记录 bEverReachedStance[i] |= (Phase == STANCE)
  ├─ PeakSlotCount = Max(PeakSlotCount, SlotComp->GetOccupiedSlotCount())
  ├─ DrawDebug 实时可视化（见下方）
  ├─ AddOnScreenDebugMessage:
  │   "Unit#1[Melee] CHASE dist=1847 | Unit#2[Melee] APPROACH dist=200 | ..."
  ├─ 当所有 bEverReachedStance == true → 进入验证
  └─ 超时 25s → Fail("Unit #N 停留在 PHASE")

Latent Step 5: 验证
  ├─ 所有单位达到过 STANCE → Pass
  ├─ PeakSlotCount >= 4 → 槽位系统工作
  ├─ 近战和远程的槽位 Radius 差 > 100cm → 分环正确
  └─ 绘制最终状态 DrawDebug（Duration=10s）

Latent Step 6: 清理
  └─ 销毁所有 Actor
```

**DrawDebug 可视化**：

实时（每帧，Duration=0）：
- `DrawDebugCircle`：TargetDummy 周围画 PreEngagementRadius 圆（白色虚线）
- `DrawDebugCircle`：画各环半径（红/蓝，对应近战/远程）
- `DrawDebugCapsule`：每个单位位置，颜色按阶段变化：
  - CHASE = 黄色
  - APPROACH = 蓝色
  - STANCE = 红色
- `DrawDebugLine`：CHASE 阶段画单位→目标的黄色实线；APPROACH 阶段画单位→槽位的蓝色虚线
- `DrawDebugString`：每个单位头顶显示 `"[#1 Melee] CHASE dist=1847cm"`
- `DrawDebugSphere`：已占用槽位位置画彩色球

`AddOnScreenDebugMessage`（屏幕左上角，每帧刷新）：
```
[FullBattleCycle] 8.3s / 25.0s
Unit#1 [Melee ] STANCE  ✓
Unit#2 [Melee ] APPROACH
Unit#3 [Ranged] STANCE  ✓
Unit#4 [Ranged] CHASE
Slots: 3/4 | Peak: 4
```

最终结果（Duration=10s）：
- 所有单位最终位置的 `DrawDebugCapsule`（绿色 = STANCE 达成，红色 = 未达成）
- `DrawDebugString`：场景中央显示 `"PASS: All 4 units completed battle cycle"` 或 `"FAIL: ..."` 
  ```
- **测试结束**：保持所有 Debug 图形 5 秒不清除，最终状态闪烁绿色（Pass）或红色（Fail）边框
