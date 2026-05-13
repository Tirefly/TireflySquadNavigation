# stance-obstacle-component Specification

## Purpose
定义 `UTsnStanceObstacleComponent` 在 Moving / Stance 两种导航身份之间切换时的当前真值行为，包括 Crowd 状态切换、NavModifier 激活与延迟关闭、站桩排斥力子系统注册、半径合法化，以及运行时参数同步。
## Requirements
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

默认几何参数 SHALL 不再只由组件硬编码属性承担。插件必须提供一层编辑器可见的 TSN 默认值来源；组件实例在未显式 override 时必须跟随该默认值，在显式 override 时才使用自己的局部值。

**默认值来源与解析顺序**：
- 插件 `DeveloperSettings` 提供默认的 `ObstacleRadius`、`NavModifierExtraRadius`、`RepulsionRadius`、`RepulsionStrength`、`bUseNavModifier`、`NavModifierMode` 与 `NavModifierDeactivationDelay`
- `UTsnStanceObstacleComponent` 必须支持“跟随插件默认值 / 局部 override”的解析路径
- NavModifier 与 `TsnStanceRepulsionSubsystem` 的运行时输入必须基于 resolved 值，而不是直接读取原始字段

**关联能力**：`stance-repulsion-subsystem`（被注册/注销）、`nav-area-stance-unit`（NavArea 类选择）、`tactical-movement-component`（RVO 已在其构造函数中自动关闭；本组件不再承担额外的 RVO 二次确认职责）

#### Scenario: 默认配置直接来自插件 DeveloperSettings

**Given** 一个单位挂载了 `UTsnStanceObstacleComponent`，且该组件未显式 override `ObstacleRadius` / `RepulsionRadius` 等参数  
**When** 它进入运行时初始化、进入站姿模式或刷新参数  
**Then** 它必须解析并使用 TSN 插件 `DeveloperSettings` 中的默认几何值

#### Scenario: 特殊单位可以局部 override 插件默认值

**Given** TSN 插件 `DeveloperSettings` 已配置默认 `ObstacleRadius = 90`  
  某个特殊单位的 `UTsnStanceObstacleComponent` 显式 override 为 `ObstacleRadius = 110`  
**When** 该单位进入站姿模式或调用 `UpdateStanceUnitParams()`  
**Then** 该单位必须优先使用自己的 override 值，而不是回退到插件默认值

#### Scenario: Debug 读取与运行时子系统看到同一套 resolved 值

**Given** 一个单位部分参数跟随插件默认值，部分参数使用局部 override  
**When** 调用 `GetDebugRadii()`、`UpdateStanceUnitParams()` 或 `EnterStanceMode()`  
**Then** NavModifier、RepulsionSubsystem 与调试接口必须基于同一套 resolved + sanitized 半径工作，而不是分别读取不同原始字段

#### Scenario: EnterStanceMode 幂等——重复调用无副作用

**Given** 单位已处于 Stance 状态（`CurrentMobilityStance == ETsnMobilityStance::Stance`）  
**When** `EnterStanceMode()` 再次被调用（BT 重试或逻辑重入）  
**Then** 立即 return，不重复执行 Crowd 状态切换、NavModifier 开启或 RepulsionSubsystem 注册

#### Scenario: ExitStanceMode 幂等——重复调用无副作用

**Given** 单位已处于 Moving 状态（`CurrentMobilityStance == ETsnMobilityStance::Moving`）  
**When** `ExitStanceMode()` 再次被调用  
**Then** 立即 return，不重复执行任何系统操作

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

