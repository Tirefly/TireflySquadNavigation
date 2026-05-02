# engagement-slot-subsystem Specification

## Purpose
TBD - created by archiving change dev-plan-for-tech-document-v2. Update Purpose after archive.
## Requirements
### Requirement: 系统 SHALL 新增 UTsnEngagementSlotSubsystem 全局槽位占用注册表

系统 SHALL 提供 `UTsnEngagementSlotSubsystem`，继承 `UWorldSubsystem`（**非** `UTickableWorldSubsystem`，无需 Tick），自动随关卡创建，无需手动实例化。

该子系统维护全局的 `Requester → SlotComponent` 映射（`TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UTsnEngagementSlotComponent>>`），保证同一攻击者同一时间只能在一个目标上持有槽位。

`UTsnEngagementSlotComponent::RequestSlot` 在分配槽位前查询本子系统，若攻击者已在其他目标上持有槽位，自动释放旧槽位后再分配新槽位。`ReleaseSlot` 在释放时注销映射。

**设计意图**：替代早期"遍历场景中所有 SlotComponent"的方案，将查找开销降低为 O(1) 哈希查找。子系统无 Tick、无计算，仅作为纯注册表。

**关联能力**：`engagement-slot-component`（RequestSlot / ReleaseSlot 内部调用注册/注销）

#### Scenario: 注册槽位占用

**Given** 攻击者 A 在目标 T1 的 `TsnEngagementSlotComponent` 上成功分配槽位  
**When** `RegisterSlotOccupancy(A, T1.SlotComp)` 被调用  
**Then** `OccupancyMap` 中添加 `A → T1.SlotComp` 映射

#### Scenario: 注销槽位占用

**Given** 攻击者 A 释放目标 T1 上的槽位  
**When** `UnregisterSlotOccupancy(A)` 被调用  
**Then** `OccupancyMap` 中移除 A 的条目

#### Scenario: 查找已占用的 SlotComponent

**Given** 攻击者 A 当前在目标 T1 上持有槽位  
**When** `FindOccupiedSlotComponent(A)` 被调用  
**Then** 返回 T1 的 `UTsnEngagementSlotComponent*`

#### Scenario: 查找无占用时返回 nullptr

**Given** 攻击者 A 当前未持有任何槽位  
**When** `FindOccupiedSlotComponent(A)` 被调用  
**Then** 返回 `nullptr`

#### Scenario: 弱引用失效时自动清理

**Given** 攻击者 A 持有目标 T1 上的槽位，但 T1 的 SlotComponent 所在 Actor 已被销毁（`TWeakObjectPtr` 失效）  
**When** `FindOccupiedSlotComponent(A)` 被调用  
**Then**  
  1. 检测到 `TWeakObjectPtr<UTsnEngagementSlotComponent>` 已失效  
  2. 自动从 `OccupancyMap` 中移除该条目  
  3. 返回 `nullptr`

#### Scenario: 重复注册覆盖旧映射

**Given** 攻击者 A 在目标 T1 上持有槽位（`OccupancyMap[A] = T1.SlotComp`）  
**When** `RegisterSlotOccupancy(A, T2.SlotComp)` 被调用  
**Then** `OccupancyMap[A]` 被更新为 `T2.SlotComp`（`TMap::Add` 自动覆盖）

#### Scenario: 关卡拆除时子系统自动销毁

**Given** 关卡切换或 PIE 结束  
**When** `UWorldSubsystem::Deinitialize()` 执行  
**Then** `OccupancyMap` 随子系统销毁自动清空，无需手动清理

#### Scenario: null-guard 防护

**Given** `UTsnEngagementSlotComponent` 在 `RequestSlot` 中尝试获取 `UTsnEngagementSlotSubsystem`  
**When** 子系统不存在（Editor Preview、无 UWorld 等场景）  
**Then** `GetWorld()->GetSubsystem<UTsnEngagementSlotSubsystem>()` 返回 `nullptr`，跳过子系统相关逻辑而非崩溃

---

