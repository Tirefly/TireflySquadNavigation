# Spec Delta: tactical-unit-interface

## ADDED Requirements

### Requirement: 系统 SHALL 新增 ITsnTacticalUnit 宿主接口

系统 SHALL 提供 `ITsnTacticalUnit` / `UTsnTacticalUnit`，作为插件与宿主项目之间的唯一通信边界，位于 `Public/Interfaces/ITsnTacticalUnit.h`（仅头文件）。

接口函数均为 `BlueprintNativeEvent + BlueprintCallable`，支持 C++ override 和蓝图实现两种方式。插件内所有读取单位属性的地方统一通过此接口访问，禁止 `Cast` 到任何宿主类。

#### Scenario: 宿主 C++ 类实现接口

**Given** 宿主项目的 `AMyWarrior` 继承 `ACharacter` 并实现 `ITsnTacticalUnit`  
**When** 插件代码执行 `ITsnTacticalUnit::Execute_GetEngagementRange(Pawn)`  
**Then** 返回宿主实现的攻击距离值，不发生 Cast 错误

#### Scenario: Pawn 未实现接口时 BT Task 返回 Failed

**Given** AI Pawn 未实现 `ITsnTacticalUnit`  
**When** BT Task（Chase / MoveToSlot）执行时检查 `Pawn->Implements<UTsnTacticalUnit>()` 返回 false  
**Then** 输出 `UE_LOG(LogTireflySquadNav, Warning, ...)` 并返回 `EBTNodeResult::Failed`，由 BT 树结构的失败分支处理恢复逻辑（Fail-fast，不使用 Fallback 兵底值）

#### Scenario: GetEngagementRange 返回值精度

**Given** 宿主实现返回 `200.f`（远程单位射程）  
**When** `TsnEngagementSlotComponent::RequestSlot` 用此值计算环半径（`200.f + SlotRadiusOffset`）  
**Then** 槽位站位半径为 `200 + SlotRadiusOffset`（默认 -20，即 180 cm），严格在攻击范围内

#### Scenario: IsInStanceMode 返回值与组件状态一致

**Given** `TsnStanceObstacleComponent::GetMobilityStance()` 返回 `ETsnMobilityStance::Stance`  
**When** 宿主实现 `IsInStanceMode()` 直接查询该组件  
**Then** 返回 `true`；`TsnStanceRepulsionSubsystem` 不对该单位施加排斥力（该单位已是站桩单位）

---

## 文件影响

| 文件 | 操作 |
|------|------|
| `Public/Interfaces/ITsnTacticalUnit.h` | ADDED |
