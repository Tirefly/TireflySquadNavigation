# state-tree-nodes Specification

## Purpose
TBD - created by archiving change add-state-tree-nodes. Update Purpose after archive.
## Requirements
### Requirement: 系统 SHALL 提供 TSN 核心战斗循环的 StateTree 任务节点

插件 SHALL 在 `Public/StateTree/` + `Private/StateTree/` 下提供一组运行时 `StateTree` 任务，覆盖 TSN 当前决策层的核心动作：

- `TsnStateTreeTask_ChaseEngagementTarget`
- `TsnStateTreeTask_MoveToEngagementSlot`
- `TsnStateTreeTask_EnterStanceMode`
- `TsnStateTreeTask_ExitStanceMode`
- `TsnStateTreeTask_ReleaseEngagementSlot`

这些任务 SHALL 直接复用现有低层运行时能力（AI MoveTo、`ITsnTacticalUnit`、`UTsnEngagementSlotComponent`、`UTsnStanceObstacleComponent`），而不是包装现有 Behavior Tree 节点。

任务 SHALL 定义显式 `InstanceData` 输入契约：

- `TsnStateTreeTask_ChaseEngagementTarget`：至少包含 `TargetActor`、`PreEngagementRadiusMultiplier`
- `TsnStateTreeTask_MoveToEngagementSlot`：至少包含 `TargetActor`、`AcceptanceRadius`、`RePathCheckInterval`、`RePathDistanceThreshold`、`MaxApproachTime`
- `TsnStateTreeTask_ReleaseEngagementSlot`：至少包含 `TargetActor`
- `TsnStateTreeTask_EnterStanceMode` / `TsnStateTreeTask_ExitStanceMode`：不要求额外编辑器参数

#### Scenario: StateTree 资产可拼出最小 TSN 决策链

- **WHEN** 设计师在 StateTree 资产中组装 TSN 决策流程
- **THEN** 至少可以用 `Chase -> MoveToEngagementSlot -> EnterStance -> ReleaseEngagementSlot -> ExitStance` 这组任务表达最小战斗清理链
- **AND** 不需要借助 BT Blackboard 或 BT Task 包装器

### Requirement: 系统 SHALL 提供 TSN 通用战斗上下文的 StateTree 评估器

插件 SHALL 提供两类共享 `StateTree` 评估器：

- `TsnStateTreeEvaluator_CombatContext`：暴露 `TargetActor` 是否有效、当前 2D 距离、`EngagementRange`、`PreEngagementRadius`
- `TsnStateTreeEvaluator_TargetMotion`：暴露目标速度与“是否静止”状态

这些评估器 SHALL 只输出 TSN 通用观测量，不承接宿主项目的目标选择策略、阵营逻辑或攻击策略。

评估器 SHALL 定义显式 `InstanceData` 契约：

- `TsnStateTreeEvaluator_CombatContext`
	- 输入：`TargetActor`、`PreEngagementRadiusMultiplier`
	- 输出：`bHasValidTarget`、`DistanceToTarget2D`、`EngagementRange`、`PreEngagementRadius`、`bIsInEngagementRange`、`bIsInPreEngagementRange`
- `TsnStateTreeEvaluator_TargetMotion`
	- 输入：`TargetActor`、`SpeedThreshold`
	- 输出：`TargetSpeed2D`、`bIsTargetStationary`

#### Scenario: CombatContext 在有效目标存在时输出距离区间数据

- **GIVEN** StateTree 执行上下文中存在有效 Pawn 与 `TargetActor`
- **WHEN** `TsnStateTreeEvaluator_CombatContext` 更新
- **THEN** 输出当前 2D 距离、`EngagementRange` 与 `PreEngagementRadius`
- **AND** `PreEngagementRadius` 由 `EngagementRange × PreEngagementRadiusMultiplier` 推导

#### Scenario: CombatContext 在目标失效时不保留陈旧有效状态

- **GIVEN** 上一帧目标有效，但当前帧 `TargetActor` 已为空或不可用
- **WHEN** `TsnStateTreeEvaluator_CombatContext` 更新
- **THEN** `HasValidTarget` 类观测量被清为无效
- **AND** 不继续对外暴露上一帧的有效目标距离结果

#### Scenario: TargetMotion 复用 TSN 静止判断语义

- **GIVEN** `TargetActor` 有效
- **WHEN** `TsnStateTreeEvaluator_TargetMotion` 更新
- **THEN** 输出目标速度
- **AND** 按与现有 `TsnBTDecorator_IsTargetStationary` 一致的阈值语义给出“是否静止”结果

### Requirement: 系统 SHALL 提供 TSN 轻量 StateTree 条件节点

插件 SHALL 提供以下 `StateTree` 条件，用于配合 TSN 任务和评估器构建状态切换：

- `TsnStateTreeCondition_HasValidTarget`
- `TsnStateTreeCondition_IsInPreEngagementRange`
- `TsnStateTreeCondition_IsInEngagementRange`
- `TsnStateTreeCondition_IsTargetStationary`
- `TsnStateTreeCondition_IsInStanceMode`

其中距离与目标运动相关条件 SHOULD 优先消费共享评估器输出；`IsInStanceMode` 可以直接查询 `UTsnStanceObstacleComponent`，无需为此单独引入新的评估器。

条件 SHALL 定义显式 `InstanceData` 输入契约：

- `TsnStateTreeCondition_HasValidTarget`：至少包含 `bHasValidTarget` 与可选 `bInvert`
- `TsnStateTreeCondition_IsInPreEngagementRange`：至少包含 `DistanceToTarget2D`、`PreEngagementRadius` 与可选 `bInvert`
- `TsnStateTreeCondition_IsInEngagementRange`：至少包含 `DistanceToTarget2D`、`EngagementRange` 与可选 `bInvert`
- `TsnStateTreeCondition_IsTargetStationary`：至少包含 `bIsTargetStationary` 与可选 `bInvert`
- `TsnStateTreeCondition_IsInStanceMode`：可只包含可选 `bInvert`，其余运行时状态直接从 `UTsnStanceObstacleComponent` 查询

#### Scenario: 设计师可通过 InstanceData 绑定复用评估器输出

- **GIVEN** StateTree 资产中同时存在 `CombatContext` 评估器与 `IsInEngagementRange` 条件
- **WHEN** 设计师为条件绑定 `DistanceToTarget2D` 和 `EngagementRange`
- **THEN** 条件可以直接消费评估器输出
- **AND** 不需要在条件内部重新计算距离与攻击范围

#### Scenario: HasValidTarget 在目标为空时返回 false

- **GIVEN** `TargetActor` 为空，或相关战斗上下文已标记为无效
- **WHEN** `TsnStateTreeCondition_HasValidTarget` 被评估
- **THEN** 条件结果为 `false`

#### Scenario: IsInPreEngagementRange 按预战斗距离阈值判断

- **GIVEN** `CombatContext` 已输出当前距离与 `PreEngagementRadius`
- **WHEN** `TsnStateTreeCondition_IsInPreEngagementRange` 被评估
- **THEN** 当 `Distance2D <= PreEngagementRadius` 时返回 `true`
- **AND** 否则返回 `false`

#### Scenario: IsInEngagementRange 按攻击距离阈值判断

- **GIVEN** `CombatContext` 已输出当前距离与 `EngagementRange`
- **WHEN** `TsnStateTreeCondition_IsInEngagementRange` 被评估
- **THEN** 当 `Distance2D <= EngagementRange` 时返回 `true`
- **AND** 否则返回 `false`

#### Scenario: IsTargetStationary 消费目标运动观测量

- **GIVEN** `TargetMotion` 已输出目标速度与静止结果
- **WHEN** `TsnStateTreeCondition_IsTargetStationary` 被评估
- **THEN** 条件结果与 `TargetMotion` 的静止判断保持一致

#### Scenario: IsInStanceMode 直接反映 TSN 组件状态

- **GIVEN** Pawn 挂载了 `UTsnStanceObstacleComponent`
- **WHEN** `TsnStateTreeCondition_IsInStanceMode` 被评估
- **THEN** 条件结果与组件当前 `MobilityStance == Stance` 保持一致

### Requirement: TsnStateTreeTask_ChaseEngagementTarget SHALL 复用 TSN 当前追击语义

`TsnStateTreeTask_ChaseEngagementTarget` SHALL 通过绑定的 `TargetActor` 和宿主 Pawn 的 `GetEngagementRange()` 计算预战斗距离，并使用 AI MoveTo 追击目标；其成功 / 失败语义应与现有 BT 版本保持一致。

#### Scenario: 启动时已在预战斗距离内立即成功

- **GIVEN** `TargetActor` 有效，且当前距离 `<= AttackRange × PreEngagementRadiusMultiplier`
- **WHEN** `TsnStateTreeTask_ChaseEngagementTarget` 进入状态
- **THEN** 任务立即返回 `Succeeded`
- **AND** 不发起新的 MoveTo 请求

#### Scenario: 目标无效或 Pawn 不满足 TSN 接口约束时失败

- **GIVEN** `TargetActor` 为空，或执行上下文无法解析出有效 AIController / Pawn，或 Pawn 未实现 `ITsnTacticalUnit`
- **WHEN** `TsnStateTreeTask_ChaseEngagementTarget` 进入状态
- **THEN** 任务返回 `Failed`
- **AND** 输出明确的 TSN Warning 日志

### Requirement: TsnStateTreeTask_MoveToEngagementSlot SHALL 复用 TSN 当前预占位接近语义

`TsnStateTreeTask_MoveToEngagementSlot` SHALL 在攻击距离外申请目标的交战槽位，追踪槽位的世界快照，并在目标移动时按阈值刷新 MoveTo。退出条件至少包含：到达槽位、中途进入攻击距离、MoveTo 失败、目标失效、超时。

#### Scenario: 已在攻击距离内时跳过槽位申请并成功

- **GIVEN** 当前 Pawn 与目标距离 `<= AttackRange`
- **WHEN** `TsnStateTreeTask_MoveToEngagementSlot` 进入状态
- **THEN** 任务立即返回 `Succeeded`
- **AND** 不调用 `RequestSlot`

#### Scenario: 任务成功时继续持有槽位

- **GIVEN** 任务已成功申请槽位，并因“到达槽位”或“中途进入攻击距离”而结束
- **WHEN** `TsnStateTreeTask_MoveToEngagementSlot` 返回 `Succeeded`
- **THEN** 攻击者继续持有当前槽位
- **AND** 槽位直到后续 `TsnStateTreeTask_ReleaseEngagementSlot` 执行时才释放

#### Scenario: 任务失败或中断时释放槽位

- **GIVEN** 任务执行过程中已经申请到槽位
- **WHEN** 发生 MoveTo 失败、目标失效、超时，或状态被外部中断
- **THEN** 当前槽位被释放
- **AND** 任务以 `Failed` 或中断语义退出，不留下悬挂占位

#### Scenario: 目标移动超过阈值时刷新槽位快照并重发移动

- **GIVEN** 任务仍在运行，且目标位置相对上次快照的移动距离 `>= RePathDistanceThreshold`
- **WHEN** 到达下一次重检查时间点
- **THEN** 任务基于当前持有槽位重新计算世界快照
- **AND** 向 AIController 发起新的 MoveTo 请求

### Requirement: 轻量 StateTree 任务 SHALL 覆盖 TSN 资源切换清理动作

`TsnStateTreeTask_EnterStanceMode`、`TsnStateTreeTask_ExitStanceMode`、`TsnStateTreeTask_ReleaseEngagementSlot` SHALL 提供与当前 BT 轻量节点等价的状态切换能力。

#### Scenario: 进入站姿时调用 TSN 组件

- **GIVEN** 执行上下文可解析出 Pawn，且 Pawn 挂载了 `UTsnStanceObstacleComponent`
- **WHEN** `TsnStateTreeTask_EnterStanceMode` 进入状态
- **THEN** 调用 `EnterStanceMode()`
- **AND** 任务返回 `Succeeded`

#### Scenario: 退出站姿时缺少组件则失败

- **GIVEN** Pawn 未挂载 `UTsnStanceObstacleComponent`
- **WHEN** `TsnStateTreeTask_ExitStanceMode` 进入状态
- **THEN** 任务返回 `Failed`

#### Scenario: 释放槽位时目标已失效也不阻塞流程

- **GIVEN** `TargetActor` 为空，或目标上已无 `UTsnEngagementSlotComponent`
- **WHEN** `TsnStateTreeTask_ReleaseEngagementSlot` 进入状态
- **THEN** 任务返回 `Succeeded`
- **AND** 不因为清理缺席而阻塞状态流转

### Requirement: TSN StateTree 节点 SHALL 具备清晰的编辑器可发现性

TSN StateTree 任务、条件、评估器 SHALL 提供可读的编辑器显示名，并在 StateTree 资产中以统一的 TSN 分类暴露，便于设计师把它们与引擎内置节点区分开。

#### Scenario: 设计师可在 StateTree 编辑器中识别 TSN 节点

- **WHEN** 设计师在 StateTree 编辑器中搜索 `TSN` 或相关节点名
- **THEN** 可以看到语义清晰的节点名称，例如 `TSN Chase Engagement Target`、`TSN Has Valid Target`、`TSN Combat Context`
- **AND** 不需要通过底层类名猜测用途

