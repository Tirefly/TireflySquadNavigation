## MODIFIED Requirements

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