# Spec Delta: behavior-tree-nodes

## ADDED Requirements

### Requirement: 系统 SHALL 新增 TsnBTTask_ChaseEngagementTarget（追击目标至预战斗距离）

系统 SHALL 提供 `TsnBTTask_ChaseEngagementTarget`，位于 `Public/BehaviorTree/TsnBTTask_ChaseEngagementTarget.h` + `Private/.cpp`。

通过 `MoveToActor`（接受半径 = `AttackRange × PreEngagementRadiusMultiplier`）追击目标，进入预战斗距离时 Succeeded；启动时已在范围内立即 Succeeded；不分配任何槽位。

当 Pawn 未实现 `ITsnTacticalUnit` 接口或 `GetEngagementRange()` 返回 ≤ 0 时，直接返回 `Failed` 并输出 Warning 日志。

**关联能力**：`tactical-unit-interface`（GetEngagementRange）

#### Scenario: Pawn 未实现接口时返回 Failed

**Given** Pawn 未实现 `ITsnTacticalUnit` 接口  
**When** `ExecuteTask` 执行  
**Then** 输出 `UE_LOG(LogTireflySquadNav, Warning, ...)` + 返回 `EBTNodeResult::Failed`

#### Scenario: GetEngagementRange 返回非法值时返回 Failed

**Given** Pawn 实现了 `ITsnTacticalUnit`，但 `GetEngagementRange()` 返回 -10  
**When** `ExecuteTask` 执行  
**Then** 输出 `UE_LOG(LogTireflySquadNav, Warning, ...)` + 返回 `EBTNodeResult::Failed`

#### Scenario: 启动时已在预战斗距离内立即 Succeeded

**Given** Pawn 与目标距离 = 100 cm，AttackRange = 150，PreEngagementRadiusMultiplier = 1.5  
  PreEngagementRadius = 225 cm；100 ≤ 225  
**When** `ExecuteTask` 执行  
**Then** 立即返回 `EBTNodeResult::Succeeded`，不发起 MoveTo

#### Scenario: TickTask 检测进入距离后停止

**Given** 任务已 InProgress，目标持续移动  
**When** 某帧距离降至 ≤ PreEngagementRadius  
**Then** `AICon->StopMovement()` + `FinishLatentTask(Succeeded)`

#### Scenario: 目标丢失时 Failed 退出

**Given** TargetKey 指向的 Actor 被销毁或 Key 被清空  
**When** TickTask 中 GetTargetActor 返回 null  
**Then** `AICon->StopMovement()` + `FinishLatentTask(Failed)`

#### Scenario: AbortTask 仅做清理

**Given** 追击任务正在 InProgress  
**When** BT 中断当前任务（高优先级分支激活或外部 Abort）  
**Then** `AICon->StopMovement()` + 返回 `EBTNodeResult::Aborted`（不做目标切换等决策，由 BT 树结构处理恢复逻辑）

---

### Requirement: 系统 SHALL 新增 TsnBTTask_MoveToEngagementSlot（预占位接近）

系统 SHALL 提供 `TsnBTTask_MoveToEngagementSlot`，位于 `Public/BehaviorTree/TsnBTTask_MoveToEngagementSlot.h` + `Private/.cpp`。

任务追踪的是**槽位的世界空间快照**，而不是固定缓存的世界点。槽位身份由 `engagement-slot-component`
中的本地极坐标维持；当目标移动时，本任务周期性刷新当前快照并重发 MoveTo。

当 Pawn 未实现 `ITsnTacticalUnit` 接口或 `GetEngagementRange()` 返回 ≤ 0 时，直接返回 `Failed` 并输出 Warning 日志。

三个退出条件（满足任意一个即完成，不阻塞后续攻击）：
- **(a)** 到达槽位（距离 ≤ AcceptanceRadius）
- **(b)** 中途进入攻击距离（目标向自己靠近）
- **(c)** 超时（ElapsedApproachTime ≥ MaxApproachTime）

**关联能力**：`engagement-slot-component`（RequestSlot / ReleaseSlot），`tactical-unit-interface`（GetEngagementRange）

#### Scenario: 进入时已在攻击距离内立即 Succeeded（不分配槽位）

**Given** Pawn 距目标 ≤ AttackRange 时 ExecuteTask 被调用  
**When** 执行开始  
**Then** 立即返回 Succeeded，不调用 `RequestSlot`

#### Scenario: 正常分配槽位并移动至槽位

**Given** Pawn 在预战斗距离内但在攻击距离外  
  目标有 TsnEngagementSlotComponent  
**When** ExecuteTask 执行  
**Then** `SlotComp->RequestSlot` 被调用；`AICon->MoveTo(SlotPos)` 发起；返回 InProgress

#### Scenario: 超时强制退出返回 Failed（路径异常履底）

**Given** MaxApproachTime = 5.0f  
  ElapsedApproachTime 达到 5.0f 时仍未到达槽位  
**When** TickTask 执行  
**Then** `AICon->StopMovement()` + `FinishLatentTask(Failed)`（超时表示路径异常，返回 Failed 供 BT 处理失败分支）

#### Scenario: MoveTo 请求路径失败时立即返回 Failed

**Given** Pawn 在预战斗距离内但在攻击距离外，槽位已分配  
  但槽位位置不可达（NavMesh 不连通或目标位置在 NavMesh 外）  
**When** `AICon->MoveTo()` 返回 `EPathFollowingRequestResult::Failed`  
**Then** `SlotComp->ReleaseSlot(Pawn)` + `FinishLatentTask(Failed)`，不等待 MaxApproachTime 超时

#### Scenario: 目标被销毁时立即返回 Failed（不等待超时）

**Given** 任务执行中目标 Actor 被销毁（`OnDestroyed` 触发）  
**When** `OnTargetDestroyed` 回调调用  
**Then** `SlotComp->ReleaseSlot(Pawn)` + `AICon->StopMovement()` + `FinishLatentTask(Failed)`，不等待 MaxApproachTime

#### Scenario: 目标被对象池回收时立即返回 Failed

**Given** 任务执行中目标 Actor 被对象池回收（`OnEndPlay` 触发，但 `OnDestroyed` 未触发）  
**When** `OnTargetEndPlay` 回调调用  
**Then** `SlotComp->ReleaseSlot(Pawn)` + `AICon->StopMovement()` + `FinishLatentTask(Failed)`，行为与目标被销毁时一致

#### Scenario: AbortTask 时释放槽位

**Given** 任务被外部中断（BT 中断或目标切换）  
**When** `AbortTask` 执行  
**Then** `SlotComp->ReleaseSlot(Pawn)` 被调用，`AICon->StopMovement()`

#### Scenario: OnTaskFinished 统一解绑委托

**Given** 任务执行过程中通过 `AActor::OnDestroyed` 和 `AActor::OnEndPlay` 绑定了目标失效回调  
**When** 任务以任意方式结束（Succeeded / Failed / Aborted）  
**Then** `OnTaskFinished` 中统一解绑目标的 `OnDestroyed` 和 `OnEndPlay` 委托，覆盖所有退出路径，防止委托指向已析构的 UBTTaskNode 实例

#### Scenario: 目标移动超阈值时刷新槽位世界快照并重新寻路

**Given** RePathDistanceThreshold = 50，RePathCheckInterval = 0.1s  
  目标在 0.1s 内移动了 60 cm  
**When** 下一次 RePathCheck 触发  
**Then** `RequestSlotAndMove` 重新调用，已持有槽位的 `AngleDeg/Radius` 被复用，新的槽位世界快照更新并发起新的 MoveTo

---

### Requirement: 所有 BT 节点 SHALL 提供友好名称与 Blackboard Key 校验

插件的每个 BT Task / Decorator SHALL 实现以下两个通用能力，以保证设计师用户体验：

1. **`GetNodeName()`**：返回可读名称，显示在行为树编辑器中。所有节点的 `NodeName` 以 `"TSN "` 为前缀，使其在编辑器菜单中成组出现，方便设计师查找。
   - `TsnBTTask_ChaseEngagementTarget` → `"TSN Chase Engagement Target"`
   - `TsnBTTask_MoveToEngagementSlot` → `"TSN Move To Engagement Slot"`
   - `TsnBTTask_EnterStanceMode` → `"TSN Enter Stance Mode"` （inline）
   - `TsnBTTask_ExitStanceMode` → `"TSN Exit Stance Mode"` （inline）
   - `TsnBTTask_ReleaseEngagementSlot` → `"TSN Release Engagement Slot"` （inline）
   - `TsnBTDecorator_IsTargetStationary` → `"TSN Is Target Stationary"`

2. **`InitializeFromAsset` Blackboard Key 校验**：所有使用 `FBlackboardKeySelector` 属性的节点，需在 `InitializeFromAsset` 中调用 `ResolveSelectedKey / IsSet`，如 Key 未正确配置，在编辑器日志中输出 Warning（`UE_LOG(LogTireflySquadNav, Warning, ...)`）而非运行时崩溃。
   - 涉及 Key 的节点：`TsnBTTask_ChaseEngagementTarget`、`TsnBTTask_MoveToEngagementSlot`、`TsnBTDecorator_IsTargetStationary`

#### Scenario: GetNodeName 在指定节点显示正确的可读名称

**Given** 行为树编辑器中添加了 `TsnBTTask_MoveToEngagementSlot` 节点  
**When** 设计师浏览行为树  
**Then** 节点显示名称为 `"TSN Move To Engagement Slot"`，而非类名 `TsnBTTask_MoveToEngagementSlot`

#### Scenario: TargetKey 未配置时编辑器输出 Warning

**Given** 设计师在 `TsnBTTask_ChaseEngagementTarget` 节点上未设置 `TargetKey`  
**When** BT Asset 加载，`InitializeFromAsset` 执行  
**Then** 输出 `LogTireflySquadNav Warning`：`"TargetKey is not set..."`，不应登载时 Crash

---

### Requirement: 系统 SHALL 新增 TsnBTTask_EnterStanceMode / ExitStanceMode / ReleaseEngagementSlot（状态切换节点）

系统 SHALL 提供三个轻量 BT Task，均通过头文件 inline 实现，无 .cpp。

| 节点 | 功能 | 失败时 |
|------|------|--------|
| `TsnBTTask_EnterStanceMode` | 找 `TsnStanceObstacleComponent`，调用 `EnterStanceMode()` | 找不到组件返回 Failed |
| `TsnBTTask_ExitStanceMode` | 找 `TsnStanceObstacleComponent`，调用 `ExitStanceMode()` | 找不到组件返回 Failed |
| `TsnBTTask_ReleaseEngagementSlot` | 找目标的 `TsnEngagementSlotComponent`，调用 `ReleaseSlot(Pawn)` | 找不到组件/目标返回 **Succeeded**（不阻塞流程） |

#### Scenario: EnterStanceMode 找不到组件时返回 Failed

**Given** AI Pawn 上没有挂载 TsnStanceObstacleComponent  
**When** `TsnBTTask_EnterStanceMode::ExecuteTask` 执行  
**Then** 返回 `EBTNodeResult::Failed`，BT Sequence 中止

#### Scenario: ReleaseEngagementSlot 目标为 null 时返回 Succeeded

**Given** TargetKey 已清空（无目标）  
**When** `TsnBTTask_ReleaseEngagementSlot::ExecuteTask` 执行  
**Then** 返回 `EBTNodeResult::Succeeded`（无副作用），不阻塞循环

---

### Requirement: 系统 SHALL 新增 TsnBTDecorator_IsTargetStationary（目标静止判断装饰器）

系统 SHALL 提供辅助工具节点，非主流程必须。`CalculateRawConditionValue` 查询目标的移动速度是否 < `SpeedThreshold`（默认 10 cm/s）。

#### Scenario: 目标速度为零时返回 true

**Given** 目标是 ACharacter，CharacterMovementComponent.Velocity.Size2D() = 0  
**When** CalculateRawConditionValue 执行  
**Then** 返回 true（静止）

#### Scenario: 目标不是 ACharacter 时使用 GetVelocity() fallback

**Given** 目标是普通 AActor（无 CharacterMovementComponent）  
**When** CalculateRawConditionValue 执行  
**Then** 调用 `Target->GetVelocity().Size2D() < SpeedThreshold`

---

## 文件影响

| 文件 | 操作 |
|------|------|
| `Public/BehaviorTree/TsnBTTask_ChaseEngagementTarget.h` | ADDED |
| `Private/BehaviorTree/TsnBTTask_ChaseEngagementTarget.cpp` | ADDED |
| `Public/BehaviorTree/TsnBTTask_MoveToEngagementSlot.h` | ADDED |
| `Private/BehaviorTree/TsnBTTask_MoveToEngagementSlot.cpp` | ADDED |
| `Public/BehaviorTree/TsnBTTask_EnterStanceMode.h` | ADDED（inline 实现） |
| `Public/BehaviorTree/TsnBTTask_ExitStanceMode.h` | ADDED（inline 实现） |
| `Public/BehaviorTree/TsnBTTask_ReleaseEngagementSlot.h` | ADDED（inline 实现） |
| `Public/BehaviorTree/TsnBTDecorator_IsTargetStationary.h` | ADDED |
| `Private/BehaviorTree/TsnBTDecorator_IsTargetStationary.cpp` | ADDED |
