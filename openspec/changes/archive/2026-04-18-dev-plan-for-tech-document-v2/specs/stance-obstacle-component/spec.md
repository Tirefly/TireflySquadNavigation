# Spec Delta: stance-obstacle-component

## ADDED Requirements

### Requirement: 系统 SHALL 新增枚举 ETsnNavModifierMode 和 ETsnMobilityStance

系统 SHALL 在 `Public/Components/TsnStanceObstacleComponent.h` 中定义两个枚举：

- `ETsnMobilityStance`：`Moving`（正常 Crowd Agent）/ `Stance`（动态障碍物）
- `ETsnNavModifierMode`：`Impassable`（NavArea_Null，棋盘格推荐）/ `HighCost`（TsnNavArea_StanceUnit，复杂地形）

#### Scenario: ETsnMobilityStance 仅描述导航身份，不区分物理速度

**Given** 单位处于 Idle/待机状态，物理速度为零  
**When** 查询 `GetMobilityStance()`  
**Then** 返回 `ETsnMobilityStance::Moving`（Idle 单位仍作为正常 Crowd Agent，非 Stance 状态）

---

### Requirement: 系统 SHALL 新增 UTsnStanceObstacleComponent 状态管理组件

系统 SHALL 提供 `UTsnStanceObstacleComponent`，挂载在每个 AI 战斗单位身上。管理 Moving ↔ Stance 切换时的三个系统状态变更：
1. DetourCrowd 状态（`Enabled` ↔ `ObstacleOnly`）
2. NavModifier 开关（延迟关闭）
3. `TsnStanceRepulsionSubsystem` 的注册/注销

该组件还负责保证双层防线的半径不变量：`RepulsionRadius > ObstacleRadius`。若宿主配置非法，
组件必须在运行时做 sanitize，并以合法化后的半径继续驱动 NavModifier 和 RepulsionSubsystem。

**默认属性值与编辑器约束**：
- `ObstacleRadius = 60.f`（约等于标准胶囊体半径），`UPROPERTY` 元数据 `ClampMin = "10.0"`
- `NavModifierExtraRadius = 45.f`，`UPROPERTY` 元数据 `ClampMin = "0.0"`；仅放大路径规划层的 NavModifier 影响半径，不改变排斥力内层边界
- `RepulsionRadius = 150.f`，`UPROPERTY` 元数据 `ClampMin = "10.0"`
- `RepulsionStrength = 800.f`，`UPROPERTY` 元数据 `ClampMin = "0.0"`

**关联能力**：`stance-repulsion-subsystem`（被注册/注销）、`nav-area-stance-unit`（NavArea 类选择）、`tactical-movement-component`（RVO 已在其构造函数中自动关闭；本组件不再承担额外的 RVO 二次确认职责）

#### Scenario: EnterStanceMode 幂等——重复调用无副作用

**Given** 单位已处于 Stance 状态（`CurrentMobilityStance == ETsnMobilityStance::Stance`）  
**When** `EnterStanceMode()` 再次被调用（BT 重试或逻辑重入）  
**Then** 立即 return，不重复执行 Crowd 状态切换、NavModifier 开启或 RepulsionSubsystem 注册

#### Scenario: ExitStanceMode 幂等——重复调用无副作用

**Given** 单位已处于 Moving 状态（`CurrentMobilityStance == ETsnMobilityStance::Moving`）  
**When** `ExitStanceMode()` 再次被调用  
**Then** 立即 return，不重复执行任何系统操作

#### Scenario: EnterStanceMode 触发三个系统变更

**Given** 单位当前为 Moving 状态，`TsnStanceObstacleComponent` 已附加  
**When** `EnterStanceMode()` 被调用  
**Then**  
  1. `CurrentMobilityStance = ETsnMobilityStance::Stance`  
  2. `CrowdFollowingComp->SetCrowdSimulationState(ECrowdSimulationState::ObstacleOnly)` 执行  
  3. 若 `bUseNavModifier == true`，NavModifier 被激活（清理挂起的延迟关闭 Timer 后设置 Navigation Relevancy）  
  4. `TsnStanceRepulsionSubsystem::RegisterStanceUnit` 被调用，传入合法化后的 RepulsionRadius / RepulsionStrength / ObstacleRadius

#### Scenario: BeginPlay 时按需预创建 NavModifier

**Given** `bUseNavModifier == true`  
**When** `BeginPlay` 执行  
**Then** 组件先执行 `CacheComponents()`，再调用 `InitNavModifier()` 预创建 NavModifier，且其初始状态为关闭

#### Scenario: 半径配置非法时运行时自动修正

**Given** `ObstacleRadius = 80`，`RepulsionRadius = 60`  
**When** `EnterStanceMode()` 被调用  
**Then**  
  1. 组件发出 ensure/warning，指出 `RepulsionRadius` 必须大于 `ObstacleRadius`  
  2. 运行时使用 `EffectiveRepulsionRadius >= 110`（至少 `ObstacleRadius + 30`）继续执行  
  3. NavModifier 与 `TsnStanceRepulsionSubsystem::RegisterStanceUnit` 都使用合法化后的 effective 半径

#### Scenario: ExitStanceMode 延迟关闭 NavModifier

**Given** 单位当前为 Stance 状态  
**When** `ExitStanceMode()` 被调用  
**Then**  
  1. `CurrentMobilityStance = ETsnMobilityStance::Moving`  
  2. `CrowdFollowingComp->SetCrowdSimulationState(ECrowdSimulationState::Enabled)` 立即执行  
  3. NavModifier **不立即关闭**，而是设置 Timer（`NavModifierDeactivationDelay` 秒后执行 `DeactivateNavModifier`）  
  4. `TsnStanceRepulsionSubsystem::UnregisterStanceUnit` 立即执行

#### Scenario: Exit→Enter 快速切换时 Timer 竞态被正确处理

**Given** 单位处于 Stance 状态，`ExitStanceMode()` 已调用且 NavModifier 延迟关闭 Timer 已挂起  
  延迟关闭 Timer 尚未到期（例如 0.2s 后触发）  
**When** 在 Timer 到期前再次调用 `EnterStanceMode()`  
**Then**  
  1. 幂等检查通过（`CurrentMobilityStance` 已被 Exit 设为 Moving）  
  2. `ActivateNavModifier` 内部首行执行 `ClearTimer(NavModifierDeactivationTimer)`，取消挂起的延迟关闭  
  3. NavModifier 被重新激活，不会被后续到期的 Timer 误关

#### Scenario: BeginPlay 时 CrowdFollowingComponent 可能尚未可用

**Given** AIController 在 BeginPlay 之后才 Possess Pawn  
**When** `BeginPlay` 中调用 `CacheComponents`  
**Then** `CrowdFollowingComp` 可能为 null（可接受）；`EnterStanceMode` 被调用时会再次调用 `CacheComponents` 补缓存

#### Scenario: EndPlay 时自动清理注册状态

**Given** 单位处于 Stance 状态时被销毁（EndPlay）  
**When** `EndPlay` 触发  
**Then** `TsnStanceRepulsionSubsystem::UnregisterStanceUnit` 被调用（**需 null-guard**：关卡拆除时 Subsystem 可能已先于组件销毁）；NavModifier 延迟 Timer 被清除

#### Scenario: bUseNavModifier = false 时跳过 NavModifier 操作

**Given** `bUseNavModifier == false`  
**When** `EnterStanceMode` / `ExitStanceMode` 被调用  
**Then** 不初始化也不操作 NavModifierComp，排斥力子系统注册/注销照常进行

#### Scenario: AIController Unpossess 后 CrowdFollowingComponent 引用失效

**Given** AI 在战斗中被 Unpossess（控制权切换），缓存的 `CrowdFollowingComp` 引用失效  
**When** 随后尝试调用 `EnterStanceMode()` 或 `ExitStanceMode()`  
**Then**  
  1. `CacheComponents` 被重新调用，通过当前 AIController 查询 PathFollowingComponent  
  2. 若 Unpossess 后无 AIController，`CrowdFollowingComp` 返回 null  
  3. Crowd 状态切换步骤被跳过，其他步骤（NavModifier、RepulsionSubsystem）照常执行

#### Scenario: OnOwnerReleased 对象池回收清理

**Given** 宿主项目使用对象池管理单位，回收时调用 `OnOwnerReleased()`  
**When** `OnOwnerReleased()` 执行  
**Then**  
  1. 若当前处于 Stance 状态，强制执行 `ExitStanceMode` 逻辑（注销 RepulsionSubsystem、恢复 Crowd 状态，并立即反激活 NavModifier）  
  2. 清除 NavModifier 延迟关闭 Timer  
  3. 下次重新激活时通过 `EnterStanceMode` 正常流程重新注册

---

### Requirement: 系统 SHALL 新增 UpdateStanceUnitParams 运行时参数更新接口

组件 SHALL 提供 `UpdateStanceUnitParams()` 接口，允许宿主项目在运行时修改 `ObstacleRadius` / `RepulsionRadius` / `RepulsionStrength` 后，
同步更新 NavModifier 和 RepulsionSubsystem 中缓存的参数。NavModifier 实际使用的导航影响半径 SHALL 基于合法化后的 `ObstacleRadius` 再叠加 `NavModifierExtraRadius` 计算。典型场景：buff/debuff 改变单位体型。

**关联能力**：`stance-repulsion-subsystem`（`UpdateStanceUnit`）

#### Scenario: Moving 状态时调用仅做参数合法化

**Given** 单位当前为 Moving 状态，宿主修改了 `ObstacleRadius = 100`、`RepulsionRadius = 200`  
**When** `UpdateStanceUnitParams()` 被调用  
**Then**  
  1. `GetSanitizedRadii` 对新半径做合法化  
  2. 若 `bUseNavModifier == true` 且 NavModifierComp 已初始化，更新其 `FailsafeExtent` 和导航边界，使用新的 EffectiveNavModifierRadius  
  3. 不调用 RepulsionSubsystem（因为 Moving 状态未注册）

#### Scenario: Stance 状态时调用同步更新所有子系统

**Given** 单位当前为 Stance 状态，已注册到 RepulsionSubsystem  
  宿主修改了 `RepulsionStrength = 1200`  
**When** `UpdateStanceUnitParams()` 被调用  
**Then**  
  1. `GetSanitizedRadii` 对新半径做合法化  
  2. NavModifier 的 `FailsafeExtent` 更新为新的 EffectiveNavModifierRadius  
  3. `TsnStanceRepulsionSubsystem::UpdateStanceUnit(GetOwner(), EffectiveRepulsionRadius, RepulsionStrength, EffectiveObstacleRadius)` 被调用  
  4. 后续帧的排斥力计算使用新的参数

#### Scenario: 非法半径配置仍然被合法化

**Given** 宿主修改 `ObstacleRadius = 100`、`RepulsionRadius = 80`（非法）  
**When** `UpdateStanceUnitParams()` 被调用  
**Then** 与 `EnterStanceMode` 一样，发出 ensure/warning 并使用合法化后的 effective 半径

---

## 文件影响

| 文件 | 操作 |
|------|------|
| `Public/Components/TsnStanceObstacleComponent.h` | ADDED |
| `Private/Components/TsnStanceObstacleComponent.cpp` | ADDED |
