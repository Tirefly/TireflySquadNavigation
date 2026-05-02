## Context

当前 TSN 的决策层能力由 `behavior-tree-nodes` 提供，运行时战斗循环已经稳定：`Chase` → `MoveToEngagementSlot` → `EnterStance` → 攻击 / 业务逻辑 → `ReleaseSlot` → `ExitStance`。文档中同时把 StateTree 列为目标决策层之一，但源码目录仍只有 `BehaviorTree/`，`StateTree/` 目录仍为空，纯蓝图指南也明确写了“若要完全复用现有 C++ BT 节点能力，需要后续补一层 StateTree Task 包装”。

因此，这次变更不是新增一套新的战斗语义，而是把现有已验证的运行时行为与关键观测量一并暴露给 StateTree 用户，让 StateTree 可以既复用 TSN 动作能力，也复用 TSN 的距离 / 目标状态判断。

## Goals / Non-Goals

**Goals:**
- 为宿主项目提供最小可用的 TSN StateTree 任务、评估器、条件集合，覆盖核心三阶段战斗循环
- 让 StateTree 任务在成功 / 失败 / 中断语义上尽量与现有 BT 节点对齐
- 让 StateTree transition 可以直接消费 TSN 共享上下文，而不必把全部判断都埋进任务内部
- 避免把 StateTree 支持做成一套平行系统，继续复用现有组件 / 子系统 / AIController 行为

**Non-Goals:**
- 不在 *运行时插件* 中引入新的战斗策略节点，例如选目标、攻击模拟、宿主专属目标选择逻辑
- 不把当前 BT 自动化测试改写成 StateTree 自动化测试
- 不新增插件自定义 StateTree Schema；优先复用引擎现有 AI StateTree 运行环境
- 不移除或替换现有 BT 节点

**测试模块例外（demo-only）**：为了让纯蓝图指南中的三个演示场景也能用 StateTree 跑通，本次变更允许在 *测试模块* `TireflySquadNavigationTest` 内新增两个 demo-only StateTree 任务（`FTsnTestStateTreeTask_SelectTarget`、`FTsnTestStateTreeTask_Attack`），与 `UTsnTestBTTask_SelectTarget`/`UTsnTestBTTask_Attack` 一一对应。这两个节点不会出现在运行时插件，也不会暴露给宿主项目作为通用能力。

## Decisions

### 1. 以“最小决策闭环”起步，而不是追求 BT 全量镜像

- **决定**：首批提供“5 个任务 + 2 个评估器 + 5 个条件”的最小 StateTree 决策闭环：
  - `TsnStateTreeTask_ChaseEngagementTarget`
  - `TsnStateTreeTask_MoveToEngagementSlot`
  - `TsnStateTreeTask_EnterStanceMode`
  - `TsnStateTreeTask_ExitStanceMode`
  - `TsnStateTreeTask_ReleaseEngagementSlot`
  - `TsnStateTreeEvaluator_CombatContext`
  - `TsnStateTreeEvaluator_TargetMotion`
  - `TsnStateTreeCondition_HasValidTarget`
  - `TsnStateTreeCondition_IsInPreEngagementRange`
  - `TsnStateTreeCondition_IsInEngagementRange`
  - `TsnStateTreeCondition_IsTargetStationary`
  - `TsnStateTreeCondition_IsInStanceMode`
- **原因**：只有任务时，StateTree 仍会退化成“顺序执行器”；补上共享评估器和轻条件后，宿主项目才能真正用 transition 表达 TSN 的距离分段和目标状态判断。
- **替代方案**：继续只做任务。这个范围更小，但 StateTree 资产仍会把大量判断埋进 Task 生命周期里，无法发挥 StateTree 决策图的优势。

### 2. StateTree 任务直接调用底层能力，不包装 BT 节点

- **决定**：StateTree 任务直接调用现有低层接口，例如 `AIController::MoveToActor/MoveToLocation`、`UTsnEngagementSlotComponent::RequestSlot/ReleaseSlot`、`UTsnStanceObstacleComponent::EnterStanceMode/ExitStanceMode`。
- **原因**：现有 BT 节点依赖 Blackboard、BT 节点生命周期和节点内存布局；在 StateTree 中再嵌套一层 BT 不但语义别扭，也会引入多套任务调度与中断模型。
- **替代方案**：创建“StateTree 调 BT 适配层”。该方案复用表面代码更多，但运行时耦合更深，调试和中断行为更差。

### 3. 用 StateTree Instance Data 与共享评估器替代 Blackboard Key

- **决定**：所有 StateTree 任务使用明确的 `InstanceData` 输入字段，例如 `TargetActor`、`AcceptanceRadius`、`RePathCheckInterval`、`MaxApproachTime`；距离区间和目标运动状态优先由共享评估器产出，而不是继续沿用 Blackboard Key 选择器。
- **原因**：StateTree 的强项就是结构化参数绑定；继续模拟 Blackboard 只会把 StateTree 资产变成“换壳 BT”。
- **结果**：规范不仅要定义任务输入，还要定义评估器输出，以及条件如何消费这些输出。

### 4. 共享评估器只暴露 TSN 通用观测量，不承接宿主策略

- **决定**：本次只新增两个通用评估器：
  - `CombatContext`：负责暴露 `TargetActor` 有效性、2D 距离、`EngagementRange`、`PreEngagementRadius`
  - `TargetMotion`：负责暴露目标速度和“是否静止”
- **原因**：这些数据已经在 BT 节点中反复被计算，属于 TSN 通用运行时观测量，最适合提升为 StateTree 共享上下文。
- **替代方案**：新增 `SelectTarget`、攻击进度、宿主阵营感知等评估器。它们更接近具体项目策略，不应在这次 runtime 通用能力提案里一起落地。

### 5. 轻量条件优先消费评估器结果，必要时才直接访问组件

- **决定**：`HasValidTarget`、`IsInPreEngagementRange`、`IsInEngagementRange`、`IsTargetStationary` 优先消费评估器输出；`IsInStanceMode` 直接查询 `UTsnStanceObstacleComponent`，不为此再新增一个独立 `UnitState` 评估器。
- **原因**：这能在避免重复计算的同时，保持首版范围克制；`IsInStanceMode` 只是一项简单组件状态查询，不值得为了它单独引入第三个评估器。
- **替代方案**：额外增加 `UnitState` 评估器。这个方向并非错误，但首版收益有限，且会把“任务 + 2 评估器 + 5 条件”的最小组合继续外扩。

### 6. `MoveToEngagementSlot` 成功后保留槽位，占位生命周期由显式 Release 节点结束

- **决定**：`TsnStateTreeTask_MoveToEngagementSlot` 在成功时不自动释放槽位；槽位继续由攻击者持有，直到后续 `TsnStateTreeTask_ReleaseEngagementSlot` 执行，或任务因失败 / 中断 / 目标失效而主动清理。
- **原因**：这与当前 BT 方案的资源生命周期一致，也符合 StateTree 中“占位状态完成后进入攻击状态”的常见状态机建模。
- **风险**：StateTree 的 `ExitState` 会在状态切换时自动调用，若无额外约束，容易把“成功离开状态”和“异常中断”都当成同一种清理路径。
- **缓解**：实现阶段需要显式区分“成功退出保留槽位”和“失败/中断退出释放槽位”。

### 7. 初版不新增插件自定义 AI Schema

- **决定**：插件只提供 StateTree 任务本身，不新增自定义 `UStateTreeSchema` 或插件专属 StateTree Component。
- **原因**：UE 5.7 已提供 `StateTreeModule` 的任务基类，以及 `GameplayStateTreeModule` 中的 AI 运行组件 / Schema 生态。TSN 当前需求只是暴露可复用任务，不需要把运行宿主环境也封装掉。
- **替代方案**：新增 `TsnStateTreeSchema`。这会增加更多公共 API、更多依赖，以及更高的宿主耦合度。

## Proposed Node Data Contracts

### Tasks

#### `TsnStateTreeTask_ChaseEngagementTarget`

**Editor-exposed input fields:**
- `TargetActor`：当前追击目标，由宿主 StateTree 绑定
- `PreEngagementRadiusMultiplier`：默认 `1.5`，与现有 BT 节点语义保持一致

**Runtime-only derived values:**
- `EngagementRange`
- `PreEngagementRadius`

#### `TsnStateTreeTask_MoveToEngagementSlot`

**Editor-exposed input fields:**
- `TargetActor`
- `AcceptanceRadius`
- `RePathCheckInterval`：默认 `0.1s`
- `RePathDistanceThreshold`：默认 `50cm`
- `MaxApproachTime`：默认 `5.0s`

**Runtime-only internal state:**
- 当前已持有槽位的目标组件引用
- 上一次槽位世界快照 / 上一次目标位置
- 已累计接近时长
- 是否需要在退出时释放槽位

#### `TsnStateTreeTask_EnterStanceMode`

**Editor-exposed input fields:**
- 无额外参数；直接消费当前执行 Pawn

#### `TsnStateTreeTask_ExitStanceMode`

**Editor-exposed input fields:**
- 无额外参数；直接消费当前执行 Pawn

#### `TsnStateTreeTask_ReleaseEngagementSlot`

**Editor-exposed input fields:**
- `TargetActor`

### Evaluators

#### `TsnStateTreeEvaluator_CombatContext`

**Editor-exposed input fields:**
- `TargetActor`
- `PreEngagementRadiusMultiplier`：默认 `1.5`

**Output fields:**
- `bHasValidTarget`
- `DistanceToTarget2D`
- `EngagementRange`
- `PreEngagementRadius`
- `bIsInEngagementRange`
- `bIsInPreEngagementRange`

**Design note:**
- 这些输出字段优先服务于 transition 和轻量条件，避免多个任务重复计算距离区间

#### `TsnStateTreeEvaluator_TargetMotion`

**Editor-exposed input fields:**
- `TargetActor`
- `SpeedThreshold`：默认 `10cm/s`，与现有 `TsnBTDecorator_IsTargetStationary` 语义对齐

**Output fields:**
- `TargetSpeed2D`
- `bIsTargetStationary`

### Conditions

#### `TsnStateTreeCondition_HasValidTarget`

**Input fields:**
- `bHasValidTarget`：通常绑定自 `CombatContext`
- `bInvert`：默认 `false`

#### `TsnStateTreeCondition_IsInPreEngagementRange`

**Input fields:**
- `DistanceToTarget2D`
- `PreEngagementRadius`
- `bInvert`：默认 `false`

#### `TsnStateTreeCondition_IsInEngagementRange`

**Input fields:**
- `DistanceToTarget2D`
- `EngagementRange`
- `bInvert`：默认 `false`

#### `TsnStateTreeCondition_IsTargetStationary`

**Input fields:**
- `bIsTargetStationary`：通常绑定自 `TargetMotion`
- `bInvert`：默认 `false`

#### `TsnStateTreeCondition_IsInStanceMode`

**Input fields:**
- `bInvert`：默认 `false`

**Design note:**
- 该条件不依赖共享评估器输出，直接查询 `UTsnStanceObstacleComponent` 当前状态

## Risks / Trade-offs

- **风险**：StateTree 与 BT 的生命周期不同，异步移动任务的“中断”和“状态成功切换”容易混淆。
  - **缓解**：在规范里单独定义 Chase / MoveToSlot 的 Exit 语义和槽位保留规则。
- **风险**：评估器输出如果定义得过宽，会把宿主项目的目标选择或战斗策略也固化进插件。
  - **缓解**：评估器只暴露 TSN 通用观测量，不输出宿主阵营、威胁值、技能偏好等策略字段。
- **风险**：宿主项目可能在非 AI 上下文里误用这些任务。
  - **缓解**：任务与条件都需要在运行时对 AIController / Pawn / `ITsnTacticalUnit` 做明确校验，并以日志 + Failed / false 快速返回。
- **权衡**：首版条件集合仍然克制，没有覆盖目标选择、攻击完成、槽位可用性预测等更高层决策。
  - **原因**：这些能力要么依赖宿主策略，要么容易和运行时占位权威入口发生竞态，适合后续单独提案。

## Test-Module Demo Nodes

这两个节点仅用于让纯蓝图指南中的演示场景在 StateTree 路径下也能闭合，不构成运行时插件能力，永远不进 `TireflySquadNavigation` 模块：

### `FTsnTestStateTreeTask_SelectTarget`

- **位置**：`TireflySquadNavigationTest` 模块。
- **行为**：与 `UTsnTestBTTask_SelectTarget` 一致——遍历 World 中所有 `ATsnTestChessPiece` 与 `ATsnTestTargetDummy`，按 TeamID 过滤敌方，按 2D 距离选最近目标。同步任务，`EnterState` 即返回 Succeeded / Failed。
- **InstanceData**：
  - 输入：`SearchRadius`（默认 5000cm）、Context 注入的 `AIController`
  - 输出（`Category="Output"`）：`TargetActor`（`TObjectPtr<AActor>`）、`bHasTarget`
- **绑定方式**：下游状态的 `TargetActor` 通过 StateTree 绑定连到此 Task 的输出字段。

### `FTsnTestStateTreeTask_Attack`

- **位置**：`TireflySquadNavigationTest` 模块。
- **行为**：与 `UTsnTestBTTask_Attack` 一致——`EnterState` 启动倒计时（默认 2 秒），`Tick` 累计经过时间，到时返回 Succeeded。
- **InstanceData**：
  - 输入：`AttackDuration`（默认 2.0s）、Context 注入的 `AIController`
  - 运行时内部：`ElapsedTime`
- **失败语义**：`AIController` 或 `Pawn` 失效时立即返回 Failed；`ExitState` 不需要清理（无外部资源）。

## Open Questions

- 当前提案不提供官方 Showcase StateTree 资产文件（`ST_Tsn_Test`）。如果后续需要随插件一起发布演示 StateTree 资产，建议单独开 `add-state-tree-showcase-assets` 提案。