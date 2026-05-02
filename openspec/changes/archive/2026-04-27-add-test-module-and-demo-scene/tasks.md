## 1. 模块与构建基础设施 `[CODE]`

- [x] 1.1 新建 `TireflySquadNavigationTest` 模块目录结构（Build.cs / Module .h/.cpp）
- [x] 1.2 在 `TireflySquadNavigation.uplugin` 中注册新模块（Type=EditorNoCommandlet）
- [x] 1.3 配置 Build.cs 模块依赖（TireflySquadNavigation、AIModule、NavigationSystem、GameplayTasks 等）

## 2. 测试用 C++ 类 `[CODE]`

- [x] 2.1 `ATsnTestChessPiece`（ACharacter 子类）—— 实现 `ITsnTacticalUnit`，挂载全部插件组件（含 `TsnEngagementSlotComponent` 使棋子可作为攻击目标），含 TeamID 属性 + 阶段可视化（DrawDebugString + DrawDebugLine）
- [x] 2.2 `ATsnTestAIController`（AAIController 子类）—— 构造函数中替换为 `UCrowdFollowingComponent`
- [x] 2.3 `ATsnTestTargetDummy`（ACharacter 子类）—— 被攻击目标，挂载 `TsnEngagementSlotComponent`，含槽位环 DrawDebug 可视化 + 可选 Spline 巡逻
- [x] 2.4 `ATsnTestSpawner`（AActor 子类）—— 批量生成器，支持 ETsnTestFormation 四种阵型（Line/Column/Grid/Circle），含 SpawnTeamID 配置
- [x] 2.5 `ETsnTestFormation` 枚举 —— Line / Column / Grid / Circle

## 3. AI 行为树与黑板

- [x] 3.1 `[MANUAL]` 创建 Blackboard Asset（BB_Tsn_Test）—— 定义 TargetActor(Object, BaseClass=Actor)、HasTarget(Bool) 两个键
- [x] 3.2 `[MANUAL]` 创建主行为树 Asset（BT_Tsn_Test）—— 连线方式详见 design.md §行为树详细设计（含完整节点树 + 每步参数 + 编辑器操作步骤）
- [x] 3.3 `[CODE]` 实现 `UTsnTestBTTask_SelectTarget` —— 遍历场景中敌方阵营（TeamID 不同）的 ChessPiece/TargetDummy，选最近存活目标，写入 TargetActorKey + HasTargetKey
- [x] 3.4 `[CODE]` 实现 `UTsnTestBTTask_Attack` —— 异步计时模拟攻击（InProgress → TickTask 倒计时 → Succeeded）

## 4. 蓝图子类与资产 `[MANUAL]`

- [x] 4.1 创建 BP_TsnTestChessPiece_Melee（近战棋子蓝图，EngagementRange=150，MaxWalkSpeed=400）
- [x] 4.2 创建 BP_TsnTestChessPiece_Spear（长矛棋子蓝图，EngagementRange=300，MaxWalkSpeed=350）
- [x] 4.3 创建 BP_TsnTestChessPiece_Range（远程棋子蓝图，EngagementRange=600，MaxWalkSpeed=300）
- [x] 4.4 创建 BP_TsnTestTargetDummy（靶标蓝图）
- [x] 4.5 创建 BP_TsnTestSpawner（生成器蓝图，可配置参数暴露到细节面板）

## 5. 演示地图搭建 `[MANUAL]`（详见 design.md §演示地图详细设计）

- [x] 5.1 创建 `MAP_TsnDemo_Siege` 演示地图（扁平场地 4000×4000，NavMeshBoundsVolume 全覆盖，含 PlayerStart）
- [x] 5.2 在 `MAP_TsnDemo_Siege` 布置围攻单目标场景 —— 1 个 TargetDummy + 3 个 Spawner（Melee×2 / Spear×2 / Range×2），InitialTarget 指向 TargetDummy
- [x] 5.3 创建 `MAP_TsnDemo_Skirmish` 演示地图（扁平场地 4000×4000，NavMeshBoundsVolume 全覆盖，含 PlayerStart）
- [x] 5.4 在 `MAP_TsnDemo_Skirmish` 布置两组对战场景 —— 红方 2 个 Spawner（Melee×2 + Range×2, TeamID=0）、蓝方 2 个 Spawner（同配，TeamID=1），间距 1400cm，无 InitialTarget（由 BT SelectTarget 自动搜索）
- [x] 5.5 创建 `MAP_TsnDemo_MovingTarget` 演示地图（扁平场地 4000×4000，NavMeshBoundsVolume 全覆盖，含 PlayerStart）
- [x] 5.6 在 `MAP_TsnDemo_MovingTarget` 布置移动目标追击场景 —— 1 个 Spline Actor（直线/弧线路径）+ 1 个 TargetDummy（bPatrolAlongSpline=true, SplineActor 指向 Spline）+ 1 个 Spawner（Melee×4）
- [x] 5.7 在三张演示地图中统一配置 NavMesh 参数（Agent Radius=34 / Height=88）、调试开关（ChessPiece 的 bDrawDebugSlotInfo、TargetDummy 的 bDrawDebugSlotRings、MovementComponent 的 bDrawDebugRepulsion），并确认根胶囊体的 bHiddenInGame=false

## 6. 自动化测试

- [x] 6.1 `[CODE]` 功能测试复用 `MAP_TsnDemo_Siege` 作为自动化底图，并在测试初始化阶段自动清理预放置展示 Actor
- [x] 6.2 `[CODE]` 单元测试：`TsnSlotCalculationTest` Automation Spec —— 验证 `CalculateWorldPosition` 角度/距离输出（7 个用例）
- [x] 6.3 `[CODE]` 单元测试：`TsnRepulsionVectorTest` Automation Spec —— 验证排斥力方向、双阶段衰减、边界情况（8 个用例）
- [x] 6.4 `[CODE]` 功能测试：`SlotAssignmentTest`（IMPLEMENT_COMPLEX_AUTOMATION_TEST）—— 自动加载地图 → 直接调用 RequestSlot API → 验证分环 + 角度 → DrawDebug 可视化 → 自动清理
- [x] 6.5 `[CODE]` 功能测试：`StanceAvoidanceTest`（IMPLEMENT_COMPLEX_AUTOMATION_TEST）—— 自动加载地图 → MoveToLocation 驱动 → 采样最小距离 → DrawDebug 轨迹 → 自动清理
- [x] 6.6 `[CODE]` 功能测试：`FullBattleCycleTest`（IMPLEMENT_COMPLEX_AUTOMATION_TEST）—— 自动加载地图 → BT 驱动完整战斗循环 → 追踪阶段变化 → DrawDebug 可视化 → 自动清理

## 7. 文档与收尾

- [x] 7.1 `[CODE]` 在插件 Documents/ 下新增测试说明文档
- [x] 7.2 `[CODE]` 编译验证全部代码（0 errors, 0 warnings）
- [x] 7.3 `[MANUAL]` 在编辑器中完成演示关卡回归验证，并确认 `NavArea_Null` 与 `HighCost` 两种模式均可正常运行
