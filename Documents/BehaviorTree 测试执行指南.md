# TireflySquadNavigation BehaviorTree 测试执行指南（UE 5.7）

## 1. 目标与范围

本指南用于你在 **不再新增 C++ 代码** 的前提下，仅通过 UE5 编辑器完成以下工作：

- 搭建 3 张演示关卡（`MAP_TsnDemo_Siege`、`MAP_TsnDemo_Skirmish`、`MAP_TsnDemo_MovingTarget`）
- 创建并配置黑板/行为树资产
- 创建并配置蓝图子类（棋子/靶标/生成器）
- 在 Session Frontend 中完成自动化测试

> 关键说明：当前测试模块的 `FullBattleCycleTest` 代码路径明确依赖 `BT_Tsn_Test + BB_Tsn_Test`。  
> 如果你要走 StateTree 路径，请直接看 `StateTree 测试执行指南.md`；本文只覆盖 Blackboard + BehaviorTree 流程。

---

## 2. 前置检查（5 分钟）

1. 启动工程 `SquadNavDev.uproject`（UE 5.7）。
2. 确认插件已启用：Edit -> Plugins -> 搜索 `TireflySquadNavigation`（Enabled）。
3. 打开 Content Browser，确认能看到 C++ 基类（若不可见，启用 View Options -> Show C++ Classes）。
4. 确认测试模块已编译（你之前已经通过编译，这一步通常只需目视确认无编译弹窗错误）。

---

## 3. 建议的资产目录结构

当前仓库使用如下目录布局：

- `/TireflySquadNavigation/FunctionShowcase/AI/BehaviorTree/`
- `/TireflySquadNavigation/FunctionShowcase/Blueprint/ChessPiece/`
- `/TireflySquadNavigation/FunctionShowcase/Blueprint/Spawner/`
- `/TireflySquadNavigation/FunctionShowcase/Blueprint/TargetDummy/`
- `/TireflySquadNavigation/FunctionShowcase/Map/`

如果你要在编辑器里重建或扩展这些资产，建议继续沿用这套目录结构。

---

## 4. 创建 Blackboard（BB_Tsn_Test）

1. 在 `FunctionShowcase/AI/BehaviorTree` 右键 -> Artificial Intelligence -> Blackboard。命名：`BB_Tsn_Test`。
2. 打开 `BB_Tsn_Test`，添加键：
- `TargetActor`：Type = Object，Base Class = Actor
- `HasTarget`：Type = Bool（默认 false）
3. 保存。

检查点：键名必须完全一致（区分大小写），否则测试节点读不到。

---

## 5. 创建 Behavior Tree（BT_Tsn_Test）

1. 在 `FunctionShowcase/AI/BehaviorTree` 右键 -> Artificial Intelligence -> Behavior Tree。命名：`BT_Tsn_Test`。
2. 打开 BT，右上角 Blackboard Asset 设为 `BB_Tsn_Test`。
3. Root 下添加 `Selector`。
4. `Selector` 左分支添加 `Sequence`，并挂 `Blackboard` Decorator：
- Key = `HasTarget`
- Query = Is Set（或等效 true 判断）
- Observer Aborts = Lower Priority
5. 在左分支 `Sequence` 按顺序添加任务：
- `Tsn BT Task Chase Engagement Target`（TargetKey = TargetActor）
- `Tsn BT Task Move To Engagement Slot`（TargetKey = TargetActor）
- `Tsn BT Task Enter Stance Mode`
- `Tsn Test BT Task Attack`（AttackDuration = 2.0）
- `Tsn BT Task Release Engagement Slot`（TargetKey = TargetActor）
- `Tsn BT Task Exit Stance Mode`
6. `Selector` 右分支添加：`Tsn Test BT Task Select Target`
- TargetActorKey = TargetActor
- HasTargetKey = HasTarget
7. 保存。

检查点：
- `SelectTarget` 必须放在右分支（低优先级兜底）。
- `HasTarget` Decorator 必须在战斗 Sequence 上，不要挂在 Root。

---

## 6. 创建蓝图子类

在 `FunctionShowcase/Blueprint` 下创建以下蓝图：

### 6.1 棋子蓝图

基类都选 `ATsnTestChessPiece`：

- `BP_TsnTestChessPiece_Melee`
- `BP_TsnTestChessPiece_Spear`
- `BP_TsnTestChessPiece_Range`

分别设置：

- Melee：
  - EngagementRange = 150
  - CharacterMovement.MaxWalkSpeed = 400
- Spear：
  - EngagementRange = 300
  - CharacterMovement.MaxWalkSpeed = 350
- Ranged：
  - EngagementRange = 600
  - CharacterMovement.MaxWalkSpeed = 300

通用建议：
- `bDrawDebugSlotInfo = true`
- 保持 `Auto Possess AI = Placed in World or Spawned`

### 6.2 靶标蓝图

1. 基类选 `ATsnTestTargetDummy`，在 `FunctionShowcase/Blueprint/TargetDummy` 下命名 `BP_TsnTestTargetDummy`。
2. 建议默认：
- `bDrawDebugSlotRings = true`
- `bPatrolAlongSpline = false`（场景三再开）

### 6.3 生成器蓝图

1. 基类选 `ATsnTestSpawner`，在 `FunctionShowcase/Blueprint/Spawner` 下命名 `BP_TsnTestSpawner`。
2. 默认模板参数建议：
- SpawnCount = 2
- Formation = Line
- FormationSpacing = 150
- bAutoSpawnOnBeginPlay = true
- BehaviorTreeAsset = `BT_Tsn_Test`
- BlackboardAsset = `BB_Tsn_Test`

---

## 7. 搭建 3 张演示地图

> 这里不再把 3 个演示场景堆在同一张地图里。  
> 原因很直接：当前 `SelectTarget` 是按全 World 搜索目标的，而 `TargetDummy` 又始终视为有效目标。  
> 如果三套场景共存在同一张地图里，只能做到“尽量不干扰”，做不到“严格不干扰”。拆成 3 张地图后，才能在不改 C++ 的前提下实现硬隔离。

### 7.1 `MAP_TsnDemo_Siege`

1. File -> New Level -> Basic。
2. 保存为：`/TireflySquadNavigation/FunctionShowcase/Map/MAP_TsnDemo_Siege`。
3. 放置一个 Plane 作为地面，缩放到约 4000x4000cm。
4. 放置 `NavMeshBoundsVolume` 覆盖整张地面。
5. 放置 `PlayerStart` 在 `(0, -1600, 0)` 左右，朝向场景中心。
6. Build -> Build Paths（或 Build All），按 `P` 确认 NavMesh 完整。
7. 放置 `BP_TsnTestTargetDummy` 在 `(0, 0, 0)`。
8. 放置 3 个 `BP_TsnTestSpawner`：
- A1：`(-1400, -200, 0)`，ChessPieceClass = `BP_TsnTestChessPiece_Melee`
- A2：`(-1400, 0, 0)`，ChessPieceClass = `BP_TsnTestChessPiece_Spear`
- A3：`(-1400, 200, 0)`，ChessPieceClass = `BP_TsnTestChessPiece_Range`
9. 三个 Spawner 共同配置：
- SpawnCount = 2
- InitialTarget = 该地图中的 TargetDummy
- BehaviorTreeAsset = `BT_Tsn_Test`
- BlackboardAsset = `BB_Tsn_Test`
- SpawnTeamID = 0
10. 建议打开：
- ChessPiece 的 `bDrawDebugSlotInfo = true`
- TargetDummy 的 `bDrawDebugSlotRings = true`

### 7.2 `MAP_TsnDemo_Skirmish`

1. File -> New Level -> Basic。
2. 保存为：`/TireflySquadNavigation/FunctionShowcase/Map/MAP_TsnDemo_Skirmish`。
3. 放置地面 Plane、`NavMeshBoundsVolume`、`PlayerStart`，步骤同上。
4. 放置 4 个 `BP_TsnTestSpawner`：
- Red1：`(-1400, -200, 0)`，ChessPieceClass = `BP_TsnTestChessPiece_Melee`，SpawnTeamID = 0
- Red2：`(-1400, 200, 0)`，ChessPieceClass = `BP_TsnTestChessPiece_Range`，SpawnTeamID = 0
- Blue1：`(1400, -200, 0)`，ChessPieceClass = `BP_TsnTestChessPiece_Melee`，SpawnTeamID = 1
- Blue2：`(1400, 200, 0)`，ChessPieceClass = `BP_TsnTestChessPiece_Range`，SpawnTeamID = 1
5. 四个 Spawner 共同配置：
- SpawnCount = 2
- InitialTarget 留空
- BehaviorTreeAsset = `BT_Tsn_Test`
- BlackboardAsset = `BB_Tsn_Test`
6. 建议打开：
- ChessPiece 的 `bDrawDebugSlotInfo = true`
- MovementComponent 的 `bDrawDebugRepulsion = true`（若已暴露到蓝图/细节面板）

### 7.3 `MAP_TsnDemo_MovingTarget`

1. File -> New Level -> Basic。
2. 保存为：`/TireflySquadNavigation/FunctionShowcase/Map/MAP_TsnDemo_MovingTarget`。
3. 放置地面 Plane、`NavMeshBoundsVolume`、`PlayerStart`，步骤同上。
4. 放置一个 Spline Actor，路径建议从 `(-1200, 0, 0)` 到 `(1200, 0, 0)`。
  不要使用 `SplineMeshActor` 作为 `SplineActor` 输入；`TargetDummy` 需要的是一个真正持有 `SplineComponent` 的 Actor。
5. 放置 `BP_TsnTestTargetDummy` 在 `(-1200, 0, 0)`，并设置：
- `bPatrolAlongSpline = true`
- `SplineActor =` 第 4 步样条 Actor
- `PatrolSpeed = 200`
6. 放置 `BP_TsnTestSpawner`（Spawner_C）在 `(-1900, -600, 0)`，并设置：
- ChessPieceClass = `BP_TsnTestChessPiece_Melee`
- SpawnCount = 4
- Formation = Grid
- InitialTarget = 该地图中的 `TargetDummy_Moving`
- SpawnTeamID = 0
- BehaviorTreeAsset = `BT_Tsn_Test`
- BlackboardAsset = `BB_Tsn_Test`
7. 建议打开：
- ChessPiece 的 `bDrawDebugSlotInfo = true`
- TargetDummy 的 `bDrawDebugSlotRings = true`

### 7.4 为什么要把出生点放这么远

当前 BT 节点的成功条件是这样定义的：

- Chase 阶段：当单位进入 `攻击距离 × 1.5` 后，任务直接成功
- MoveToSlot 阶段：当单位进入 `攻击距离` 后，任务直接成功

这意味着如果出生点放得太近，就会出现“看起来行为树直接跳到攻击”的现象。

按默认兵种参数：

- 近战：`EngagementRange = 150`，PreEngagement = 225
- 长矛：`EngagementRange = 300`，PreEngagement = 450
- 远程：`EngagementRange = 600`，PreEngagement = 900

所以如果你想让三类单位都肉眼可见地先跑起来，再进入后续阶段，初始距离最好大于 `900cm`。上面给你的推荐坐标就是按这个条件倒推的。

### 7.5 如果单位一生成就直接 Attack，先查这 4 项

1. 检查每个 Spawner 的 `ChessPieceClass` 是否真的指向了正确蓝图，而不是 3 个 Spawner 都引用了同一个类。
2. 检查三个棋子蓝图里的 `EngagementRange` 是否分别是 `150 / 300 / 600`，有没有多写一个 0。
3. 检查 TargetDummy 和 Spawner 的实际世界坐标是否确实按上面的距离摆开了。
4. 运行时选中一个 ChessPiece，看头顶调试字：
- 如果一出生就是 `STANCE`，说明前两个节点在运行时都被距离条件短路了。
- 如果显示 `CHASE` 或 `APPROACH` 但身体不动，那才是导航/移动层问题。
- 确认一下 Chase、MoveToSlot 等节点的 TargetKey 是否正确绑定了 `TargetActor`，以及 Blackboard Asset 是否正确设置为 `BB_Tsn_Test`。

---

## 8. 功能测试底图约定

当前 3 个功能测试默认复用 `/TireflySquadNavigation/FunctionShowcase/Map/MAP_TsnDemo_Siege` 作为底图。

运行策略如下：
- 自动化测试先加载 `MAP_TsnDemo_Siege`
- 测试初始化阶段自动销毁关卡里预放置的 `ATsnTestSpawner`、`ATsnTestChessPiece`、`ATsnTestTargetDummy`
- 然后再动态生成本次测试所需 Actor

因此当前仓库不再要求额外维护一张 `MAP_TsnAutoTest`。

---

## 9. 如需 StateTree 路径

如果你要用 StateTree 复现同一组演示场景，不要继续在本文里混配 BT 与 StateTree 流程，直接查看 `StateTree 测试执行指南.md`。

原因很简单：

- `FullBattleCycleTest` 当前明确依赖 `BT_Tsn_Test + BB_Tsn_Test`
- 本文的目标是把 Blackboard / BehaviorTree 路径跑通并稳定复现
- StateTree 现在已经有单独文档，不再和 BT 指南混写

---

## 10. 在编辑器中运行测试

1. 打开 UE 编辑器。
2. 菜单：Window -> Developer Tools -> Session Frontend。
3. 切到 Automation 标签。
4. 展开 `TireflySquadNavigation` -> `Test`。
5. 勾选：
- `Unit.SlotCalculation`
- `Unit.RepulsionVector`
- `Functional.SlotAssignment`
- `Functional.StanceAvoidance`
- `Functional.FullBattleCycle`
6. 点击 Start Tests。

预期：全部绿色通过。

---

## 11. 失败定位顺序（建议）

如果有失败，按这个顺序排查，效率最高：

1. 地图路径是否正确：
- `MAP_TsnDemo_Siege` 是否在 `/TireflySquadNavigation/FunctionShowcase/Map/`
2. 资产命名是否一致：
- `BB_Tsn_Test`
- `BT_Tsn_Test`
3. BT 键绑定是否正确：
- `TargetActor` / `HasTarget`
4. Spawner 是否都绑定 BT/BB
5. NavMesh 是否覆盖完整（按 `P` 检查）

---

## 12. 最终验收清单

- [ ] `MAP_TsnDemo_Siege` 可在 PIE 展示围攻单目标场景
- [ ] `MAP_TsnDemo_Skirmish` 可在 PIE 展示红蓝对冲与目标切换
- [ ] `MAP_TsnDemo_MovingTarget` 可在 PIE 展示移动目标追击
- [ ] `Functional` 3 项测试可复用 `MAP_TsnDemo_Siege` 正常启动
- [ ] Automation 5 项测试全部通过

完成以上条目后，BehaviorTree 测试流程即闭环完成。
