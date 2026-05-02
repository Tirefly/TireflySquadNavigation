# Project Context

## OpenSpec Scope

- 本 OpenSpec 工作区的根目录是 `Plugins/TireflySquadNavigation/`。
- 除非用户明确指定其他目标，否则所有 OpenSpec 的 proposal/spec/change/archive 都默认针对 `TireflySquadNavigation` 插件。
- 使用 OpenSpec CLI 时，默认先切到 `Plugins/TireflySquadNavigation` 再执行命令。
- 在本工作区内描述路径时，默认以插件根目录为基准；只有在需要指向宿主工程根目录时才显式写出 `SquadNavDev/` 或绝对路径。

## Purpose

`TireflySquadNavigation` 是一个 UE 插件，实现战场小队导航系统——类似《云顶之弈》自走棋 AI 战斗单位的移动行为：自动导航至攻击目标附近，进入攻击距离后停止并攻击；移动中平滑绕开其他单位，不推挤、不重叠、不卡死。

本仓库 (`SquadNavDev`) 是该插件的开发工程。当前状态：**预实现阶段**（架构设计完成，Public/Private 目录为空，仅有模块存根代码）。

权威技术文档：`Documents/战场小队导航系统 - 技术方案文档 V2.md`

## Tech Stack

- **引擎**：Unreal Engine 5.7（位于 `E:\UnrealEngine\UE_5.7`）
- **语言**：C++20（MSVC 14.44，VS2022）
- **构建系统**：Unreal Build Tool (UBT)
- **关键引擎模块**：
  - `NavigationSystem`（RecastNavMesh 全局寻路）
  - `AIModule`（DetourCrowd 群体避障、BehaviorTree 决策）
  - `GameplayTasks`（BT Task 异步执行基础）
- **插件模块依赖**：
  - Public: `Core`
  - Private: `CoreUObject`, `Engine`, `Slate`, `SlateCore`
  - 按需追加: `NavigationSystem`, `AIModule`, `GameplayTasks`

## Project Conventions

### Code Style

- **插件缩写**：`TSN`；**C++ 类前缀**：`Tsn`
- **API 宏**：`TIREFLYSQUADNAVIGATION_API`
- **三类语义前缀不可混用**：
  - `Tactical` — 插件整体战术定位（`TsnTacticalMovementComponent`, `ITsnTacticalUnit`）
  - `Stance` — 静止战斗姿态/站桩（`TsnStanceObstacleComponent`, `TsnStanceRepulsionSubsystem`, `TsnNavArea_StanceUnit`, `ETsnMobilityStance`）
  - `Engagement` — 交战行为（`TsnEngagementSlotComponent`, BT 节点）
- **注释要求**：类/结构体/枚举声明、成员变量与函数、超过 2 参数的函数需逐参注释、有返回值函数注释返回含义
- **PCH 模式**：`UseExplicitOrSharedPCHs`

### Architecture Patterns

插件采用**四层架构**：

| 层次 | 核心类 | 职责 |
|------|-------|------|
| 决策层 | BT Tasks / StateTree | 选目标 → 追击 → 申请槽位 → 移动 → 站桩攻击 → 循环 |
| 槽位层 | `TsnEngagementSlotComponent`（挂在被攻击目标上） | 多环角度槽位分配，按攻击距离分环 |
| 导航层 | NavMesh + DetourCrowd | 全局寻路 + 群体局部避障 |
| 移动层 | `TsnTacticalMovementComponent` | 重写 `CalcVelocity` / `RequestDirectMove`，合法注入排斥力 |

**站桩单位管理（双层防线）**：
1. NavModifier（`TsnStanceObstacleComponent`）— 路径规划层，标记 `TsnNavArea_StanceUnit`
2. 排斥力（`TsnStanceRepulsionSubsystem`）— 运行时兜底，双阶段力场

**三阶段行为模型**：
1. 远距离追击（dist > PreEngagementRadius）→ 自由追击
2. 预占位接近（AttackRange < dist ≤ PreEngagementRadius）→ 申请槽位移动
3. 就地站桩（dist ≤ AttackRange）→ 停止移动，攻击

**接口隔离**：宿主项目实现 `ITsnTacticalUnit` 接口（`GetEngagementRange()`, `IsInStanceMode()`），插件内严禁 Cast 到任何宿主类。

### Testing Strategy

暂未建立自动化测试框架。当前验证方式：UBT 编译 + 编辑器内运行测试。

编译命令：
```bash
# 刷新项目文件（新增文件后必须先执行）
"E:\UnrealEngine\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="E:\Projects_Unreal\SquadNavDev\SquadNavDev.uproject" -game -rocket -progress

# 编译 Editor (Development)
"E:\UnrealEngine\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" SquadNavDevEditor Win64 Development -Project="E:\Projects_Unreal\SquadNavDev\SquadNavDev.uproject" -rocket -progress
```

### Git Workflow

未明确定义。使用 OpenSpec 进行 spec-driven 开发——重大功能变更通过当前插件工作区内的 `openspec/changes/` 下的提案流程管理。

常用命令应在插件目录执行：

```bash
Push-Location "E:\Projects_Unreal\SquadNavDev\Plugins\TireflySquadNavigation"
openspec list
openspec list --specs
openspec validate --strict --no-interactive
Pop-Location
```

如果只想表达命令本体，也默认以上述插件目录为当前工作目录。

## Domain Context

- **自走棋战斗 AI**：单位自动选择目标 → 导航移动 → 就位攻击 → 目标死亡后重新选择
- **两种单位状态**：移动中（Crowd Agent）、站桩战斗中（动态障碍物，速度 = 0）
- **核心难点**：RVO/DetourCrowd 对零速度单位失效；多攻击距离围攻同一目标的分环站位；排斥力不能覆盖路径跟随
- **目标规模**：十几到几十个单位，优先正确性

## Important Constraints

1. **规模约束**：面向小规模（10-50 单位），优先正确性，不追求大规模性能优化
2. **复用约束**：跨项目插件，不假设宿主项目实现 → 严禁 Cast 宿主类，仅通过 `ITsnTacticalUnit` 接口通信
3. **地图约束**：主要面向棋盘格/开阔地 → NavModifier 默认不可通行，保留高代价模式供复杂地形覆盖
4. **引擎约束**：启用 DetourCrowd 后必须关闭 CMC 内置 RVO Avoidance（防双重避障）；站桩单位设为 `ECrowdSimulationState::ObstacleOnly` 而非 `Disabled`
5. **排斥力注入**：只能通过 `SetRepulsionVelocity()` → `PendingRepulsionVelocity`，在 `CalcVelocity`/`RequestDirectMove` 中消费，禁止直接写 `Velocity`

## External Dependencies

- **Unreal Engine 5.7** — 路径：`E:\UnrealEngine\UE_5.7`
- **RecastNavMesh** — 引擎内置全局路径规划
- **DetourCrowd** — 引擎内置群体避障（`UCrowdFollowingComponent`）
- **BehaviorTree / StateTree** — 引擎内置 AI 决策框架
- **NavModifierComponent** — 引擎内置运行时导航区域标记
- 无外部第三方库依赖
