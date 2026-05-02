# TireflySquadNavigation StateTree 测试执行指南（UE 5.7）

适用范围：用 StateTree 复现 Siege / Skirmish / MovingTarget 三个演示场景。

前提：

1. 项目已能正常编译。
2. `TireflySquadNavigation`、`StateTree`、`GameplayStateTree` 已启用。
3. 已按 `纯蓝图测试执行指南.md` 建好基础资产：棋子蓝图、TargetDummy、Spawner、三张演示地图。

## 1. 创建 `ST_Tsn_Test`

1. 在 `/TireflySquadNavigation/FunctionShowcase/AI/StateTree/` 新建 `State Tree` 资产。
2. Schema 选择 **State Tree AI Component Schema**。
3. 打开资产，确认 `Context Actor Class = Pawn`。
4. 新建 Parameters：
   - `CurrentTargetActor`：Actor
   - `HasTarget`：Bool = `false`
   - `SearchRadius`：Float = `5000`
   - `PreEngagementRadiusMultiplier`：Float = `1.5`
   - `AttackDuration`：Float = `2.0`
   - `ApproachAcceptanceRadius`：Float = `30`
   - `RePathCheckInterval`：Float = `0.1`
   - `RePathDistanceThreshold`：Float = `50`
   - `MaxApproachTime`：Float = `5.0`
5. 新建 Global Evaluator：`TSN Combat Context`
   - `TargetActor -> Parameters.CurrentTargetActor`
   - `PreEngagementRadiusMultiplier -> Parameters.PreEngagementRadiusMultiplier`
6. 搭状态图：
   - `Acquire`
     - Task：`TSN Test Select Target`
     - `SearchRadius -> Parameters.SearchRadius`
     - `TargetActor -> Parameters.CurrentTargetActor`
     - `bHasTarget -> Parameters.HasTarget`
     - `On Succeeded -> Engage`
     - `On Failed -> Acquire`
   - `Engage/Chase`
     - Task：`TSN Chase Engagement Target`
     - `TargetActor -> Parameters.CurrentTargetActor`
     - `PreEngagementRadiusMultiplier -> Parameters.PreEngagementRadiusMultiplier`
     - `On Succeeded -> Approach`
     - `On Failed -> Acquire`
   - `Engage/Approach`
     - Task：`TSN Move To Engagement Slot`
     - `TargetActor -> Parameters.CurrentTargetActor`
     - `AcceptanceRadius -> Parameters.ApproachAcceptanceRadius`
     - `RePathCheckInterval -> Parameters.RePathCheckInterval`
     - `RePathDistanceThreshold -> Parameters.RePathDistanceThreshold`
     - `MaxApproachTime -> Parameters.MaxApproachTime`
     - `On Succeeded -> EnterStance`
     - `On Failed -> Acquire`
   - `Engage/EnterStance`
     - Task：`TSN Enter Stance Mode`
     - `On Succeeded -> Attack`
     - `On Failed -> ReleaseAndExit`
   - `Engage/Attack`
     - Task：`TSN Test Attack`
     - `AttackDuration -> Parameters.AttackDuration`
     - `On Succeeded -> ReleaseAndExit`
     - `On Failed -> ReleaseAndExit`
   - `Engage/ReleaseAndExit`
     - Task 1：`TSN Release Engagement Slot`
     - `TargetActor -> Parameters.CurrentTargetActor`
     - Task 2：`TSN Exit Stance Mode`
     - `On Succeeded -> Acquire`
7. Compile 并保存为 `ST_Tsn_Test`。

## 2. 创建 StateTree 控制器与棋子

1. 新建 `BP_TsnTestAIController_ST`，父类为 `ATsnTestAIController`。
2. 给它添加 `StateTree AI Component`。
3. 在组件上设置：
   - `State Tree = ST_Tsn_Test`
   - `Start Logic Automatically = false`
4. Compile + Save。

说明：当前测试控制器会在 `Possess` 后自动启动 BrainComponent，关闭 `Start Logic Automatically` 可以避免 `BeginPlay` 早启时的上下文错误。

5. 分别基于原棋子蓝图创建：
   - `BP_TsnTestChessPiece_Melee_ST`
   - `BP_TsnTestChessPiece_Spear_ST`
   - `BP_TsnTestChessPiece_Range_ST`
6. 在每个 `_ST` 棋子蓝图的 `Class Defaults` 里设置：
   - `AI Controller Class = BP_TsnTestAIController_ST`
   - `Auto Possess AI = Placed in World or Spawned`
7. 不要在棋子蓝图上添加 `StateTreeAIComponent`。
8. Compile + Save。

## 3. 修改三张 `_ST` 地图

### Siege

1. 打开 `MAP_TsnDemo_Siege`，另存为 `MAP_TsnDemo_Siege_ST`。
2. 3 个 Spawner 的 `ChessPieceClass` 改成对应 `_ST` 棋子。
3. 3 个 Spawner 的 `BehaviorTreeAsset = None`。
4. 其他参数保持不变。
5. `Build Paths`，保存关卡。

### Skirmish

1. 打开 `MAP_TsnDemo_Skirmish`，另存为 `MAP_TsnDemo_Skirmish_ST`。
2. 4 个 Spawner 的 `ChessPieceClass` 改成对应 `_ST` 棋子。
3. 4 个 Spawner 的 `BehaviorTreeAsset = None`。
4. `SpawnTeamID` 保持原值，`InitialTarget` 保持空。
5. `Build Paths`，保存关卡。

### MovingTarget

1. 打开 `MAP_TsnDemo_MovingTarget`，另存为 `MAP_TsnDemo_MovingTarget_ST`。
2. `Spawner_C` 设置：
   - `ChessPieceClass = BP_TsnTestChessPiece_Melee_ST`
   - `BehaviorTreeAsset = None`
3. `SpawnCount`、`Formation`、`InitialTarget`、`SpawnTeamID` 保持原值。
4. TargetDummy 的巡逻配置保持原值。
5. `Build Paths`，保存关卡。

## 4. 验证

1. 打开任意 `_ST` 地图并运行 PIE。
2. 预期流程：`Acquire -> Chase -> Approach -> EnterStance -> Attack -> ReleaseAndExit -> Acquire`。
3. 在 PIE Console 输入 `showdebug statetree` 可查看实时状态。
4. 需要查看控制器生成与占有诊断时，启动参数加：`-TsnDebugControllerFlow`。

## 5. 最小排查项

如果单位不动，只检查这 4 项：

1. `BP_TsnTestAIController_ST` 上确实挂了 `StateTree AI Component`，且 `State Tree = ST_Tsn_Test`。
2. `StateTree AI Component` 的 `Start Logic Automatically = false`。
3. `_ST` 棋子蓝图的 `AI Controller Class = BP_TsnTestAIController_ST`。
4. 地图里 Spawner 实例的 `BehaviorTreeAsset = None`，并且 NavMesh 已重建。
