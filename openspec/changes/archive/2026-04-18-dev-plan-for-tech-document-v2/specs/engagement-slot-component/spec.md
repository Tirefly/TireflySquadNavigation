# Spec Delta: engagement-slot-component

## ADDED Requirements

### Requirement: 系统 SHALL 新增 UTsnEngagementSlotComponent 多环角度槽位分配

系统 SHALL 提供 `UTsnEngagementSlotComponent`，挂载在**被攻击目标**身上，位于 `Public/Components/TsnEngagementSlotComponent.h` + `Private/Components/TsnEngagementSlotComponent.cpp`。

核心能力：
1. **多环支持**：不同 `AttackRange` 的攻击者站在不同的环上，按半径分组冲突检测
2. **就近分配**：以请求者当前方位角为理想角，优先近的角度，减少路径扭曲
3. **容错**：槽位满时退化为合理位置，不阻塞攻击流程
4. **槽位身份稳定**：`Slots` 中保存的是目标本地极坐标（`AngleDeg + Radius`），世界坐标按目标当前位置实时换算
5. **世界空间方位角**：`AngleDeg` 是世界空间方位角（由 `atan2(Requester - Target)` 计算），不随目标旋转。对自走棋类场景无影响——攻击者只需走到正确的世界空间位置即可
6. **单目标槽位约束**：同一攻击者同一时间只能在一个目标上持有槽位。`RequestSlot` 内部通过 `UTsnEngagementSlotSubsystem::FindOccupiedSlotComponent()` 查询请求者是否已在其他目标上持有槽位，若是则自动释放旧槽位后再分配新槽位。分配成功后调用 `RegisterSlotOccupancy`；`ReleaseSlot` 释放时调用 `UnregisterSlotOccupancy`
7. **GameThread 假设**：所有 `RequestSlot` / `ReleaseSlot` 调用假定在 GameThread 上执行，无线程竞争风险

**默认属性值**：
- `MaxSlots = 12`：最大同时交战单位数
- `SlotRadiusOffset = -20.f`：槽位半径相对攻击距离的偏移量（负值确保站位在攻击范围内）
- `MinSlotSpacing = 80.f`：同一环上相邻槽位的最小间距（cm），约等于角色碰撞直径
- `SameRingRadiusTolerance = 60.f`：半径差在此范围内的攻击者被视为同一环参与角度冲突检测

#### Scenario: 请求者首次请求槽位——就近分配

**Given** 目标上挂有 `TsnEngagementSlotComponent`，当前无任何槽位被占用  
  且请求者位于目标正东方  
**When** `RequestSlot(Requester, 200.f)` 被调用  
**Then**  
  1. 计算理想角度 ≈ 0°（正东）  
  2. 同环无冲突，直接分配 AngleDeg ≈ 0°  
  3. 返回目标东侧 `200 + SlotRadiusOffset` 距离处、基于当前目标位置换算出的世界快照  
  4. Slots 数组中增加一条记录

#### Scenario: 同环已有攻击者时角度偏移

**Given** 同环（半径=200-20=180）已有攻击者在 0°  
  且 MinSlotSpacing = 80，Radius = 180，MinAngularGap ≈ atan2(80/180) ≈ 25°  
**When** 新请求者也在正东方，理想角 0°，调用 `RequestSlot`  
**Then** 从 ±SearchStep 开始搜索，分配第一个与 0° 间距 ≥ 25° 的角度（例如 25° 或 -25°）

#### Scenario: 目标移动后重新请求返回更新坐标

**Given** 请求者已持有槽位，AngleDeg = 30°  
  且目标已从原始位置移动了 500 cm  
**When** 再次调用 `RequestSlot(Requester, 200.f)`  
**Then** 进入 FindExistingSlot 分支，**复用原 AngleDeg/Radius**，仅重新计算当前世界快照，而不是分配新的本地槽位

#### Scenario: 攻击距离改变后重新分配槽位

**Given** 请求者已持有槽位，Radius = 180（基于 AttackRange = 200）  
  宿主项目通过 buff 修改了请求者的 AttackRange 为 350  
  新的 SlotRadius = 350 + SlotRadiusOffset = 330  
  |330 - 180| = 150 > SameRingRadiusTolerance(60)  
**When** 再次调用 `RequestSlot(Requester, 350.f)`  
**Then** 原槽位已不属于同环，释放旧槽位并在新环上重新分配

#### Scenario: 槽位满时不阻塞攻击

**Given** Slots.Num() >= MaxSlots（默认 12）  
**When** 新请求者调用 `RequestSlot`  
**Then** 返回目标与请求者连线方向上的合理坐标（Fallback），不修改 Slots 数组

#### Scenario: 不同半径环之间不互相冲突检测

**Given** 内环攻击者（半径 80 cm）占据 0°，外环攻击者（半径 300 cm）申请 0°  
  且 SameRingRadiusTolerance = 60  
  此时 |300 - 80| = 220 > 60  
**When** 外环调用 `FindUnoccupiedAngle(0°, 300)`  
**Then** 内环的 0° 不计入冲突检测，外环直接分配 0°

#### Scenario: 占用者销毁后槽位自动清理

**Given** 某攻击者 Actor 被销毁（WeakPtr 失效）  
**When** 下一次 `RequestSlot` 或 `ReleaseSlot` 触发 `CleanupInvalidSlots`  
**Then** 失效记录从 Slots 数组移除，槽位开放给新的攻击者

#### Scenario: 请求者已在其他目标上持有槽位时自动释放旧槽位

**Given** 攻击者 A 当前在目标 T1 上持有槽位  
  BT 切换目标为 T2  
**When** `T2->SlotComp->RequestSlot(A, 200.f)` 被调用  
**Then**  
  1. `RequestSlot` 内部通过 `UTsnEngagementSlotSubsystem::FindOccupiedSlotComponent(A)` 查询到 A 在 T1 上持有槽位  
  2. 自动调用 `T1->SlotComp->ReleaseSlot(A)` 释放旧槽位（含 `UnregisterSlotOccupancy`）  
  3. 然后在 T2 上分配新槽位并调用 `RegisterSlotOccupancy(A, T2.SlotComp)`

#### Scenario: OnOwnerReleased 对象池回收清理

**Given** 目标 Actor 被对象池回收，宿主项目调用 `OnOwnerReleased()`  
**When** `OnOwnerReleased()` 执行  
**Then**  
  1. 遍历所有 Slots，对每个有效占用者调用 `UTsnEngagementSlotSubsystem::UnregisterSlotOccupancy`  
  2. 清空 Slots 数组  
  3. 下次重新激活时可以接受新的槽位请求

#### Scenario: OnOwnerReleased 在关卡拆除期间调用时安全退出

**Given** 切关卡时，`UTsnEngagementSlotSubsystem` 先于组件被销毁（UE 正常 Subsystem 拆除顺序），对象池同帧批量回收对象并调用 `OnOwnerReleased()`  
**When** `OnOwnerReleased()` 执行  
**Then**  
  1. `GetWorld()` 返回 null **或** `World->GetSubsystem<UTsnEngagementSlotSubsystem>()` 返回 null  
  2. 跳过子系统注销，直接清空 Slots 数组  
  3. 不 crash，无内存访问违规

---

## 文件影响

| 文件 | 操作 |
|------|------|
| `Public/Components/TsnEngagementSlotComponent.h` | ADDED |
| `Private/Components/TsnEngagementSlotComponent.cpp` | ADDED |
| `Public/Subsystems/TsnEngagementSlotSubsystem.h` | ADDED（关联，由 `engagement-slot-subsystem` spec 定义） |
