# test-module Specification

## Purpose
TBD - created by archiving change add-test-module-and-demo-scene. Update Purpose after archive.
## Requirements
### Requirement: Test Module Infrastructure

The plugin SHALL include an Editor-only module `TireflySquadNavigationTest` that contains all test and demo code, isolated from the core `TireflySquadNavigation` module. This module SHALL NOT be included in Shipping builds.

#### Scenario: Module registration
- **WHEN** the plugin is loaded in UE Editor
- **THEN** the `TireflySquadNavigationTest` module SHALL be loaded alongside the core module
- **AND** the module SHALL have access to all public headers of `TireflySquadNavigation`

#### Scenario: Shipping exclusion
- **WHEN** the project is packaged for Shipping
- **THEN** the `TireflySquadNavigationTest` module and its assets SHALL NOT be included in the package

### Requirement: Test Chess Piece Character

The test module SHALL provide a concrete `ATsnTestChessPiece` (ACharacter subclass) that implements `ITsnTacticalUnit` and pre-configures all plugin components (`TsnTacticalMovementComponent`, `TsnStanceObstacleComponent`, `TsnUnitSeparationComponent`) for immediate use in demo scenarios.

#### Scenario: Component setup
- **WHEN** an `ATsnTestChessPiece` is spawned
- **THEN** it SHALL have `TsnTacticalMovementComponent` as its CharacterMovementComponent
- **AND** it SHALL have `TsnStanceObstacleComponent`, `TsnUnitSeparationComponent`, and `TsnEngagementSlotComponent` attached
- **AND** `TsnEngagementSlotComponent` SHALL allow other units to request engagement slots around this chess piece (enabling chess-piece-vs-chess-piece combat in Scene 2)
- **AND** `ITsnTacticalUnit::GetEngagementRange()` SHALL return a configurable value (default 150 cm)
- **AND** `ITsnTacticalUnit::IsInStanceMode()` SHALL delegate to `TsnStanceObstacleComponent::GetMobilityStance()`

### Requirement: Test AI Controller with Crowd Following

The test module SHALL provide `ATsnTestAIController` that uses `UCrowdFollowingComponent` for path following and disables the built-in RVO avoidance on the controlled pawn's movement component.

#### Scenario: Crowd integration
- **WHEN** `ATsnTestAIController` possesses an `ATsnTestChessPiece`
- **THEN** `UCrowdFollowingComponent` SHALL be the active path following component
- **AND** `bUseRVOAvoidance` on the movement component SHALL be set to `false`

### Requirement: Test Target Dummy

The test module SHALL provide `ATsnTestTargetDummy` (ACharacter subclass) with `TsnEngagementSlotComponent` pre-attached, serving as the attack target for chess pieces in demo scenarios.

#### Scenario: Slot component ready
- **WHEN** an `ATsnTestTargetDummy` is spawned
- **THEN** it SHALL have a `TsnEngagementSlotComponent` ready to accept slot requests from attacking units

### Requirement: Demo Behavior Tree

The test module SHALL provide a complete Behavior Tree (`BT_Tsn_Test`) that implements the full three-phase combat cycle using the plugin's BT nodes: chase → pre-occupy slot → stance attack → loop.

#### Scenario: Full combat cycle
- **WHEN** an AI chess piece runs `BT_Tsn_Test` with a valid target in Blackboard
- **THEN** it SHALL execute: `TsnBTTask_ChaseEngagementTarget` (phase 1) → `TsnBTTask_MoveToEngagementSlot` (phase 2) → `TsnBTTask_EnterStanceMode` (phase 3) → attack → `TsnBTTask_ReleaseEngagementSlot` → `TsnBTTask_ExitStanceMode` → loop

### Requirement: Demo Maps

The test module SHALL include three isolated demonstration maps: `MAP_TsnDemo_Siege`, `MAP_TsnDemo_Skirmish`, and `MAP_TsnDemo_MovingTarget`. Each map SHALL provide a flat arena, full NavMesh coverage, and only the actors required for its own scenario so that cross-scene target selection cannot occur. Functional automation tests SHALL reuse `MAP_TsnDemo_Siege` as a base map and clear pre-placed showcase actors before spawning test actors.

#### Scenario: Siege demo map loads and runs
- **WHEN** `MAP_TsnDemo_Siege` is opened in PIE
- **THEN** NavMesh SHALL cover the arena floor
- **AND** pre-placed spawners SHALL generate melee / spear / ranged units that autonomously surround a single target dummy

#### Scenario: Skirmish demo map loads and runs
- **WHEN** `MAP_TsnDemo_Skirmish` is opened in PIE
- **THEN** NavMesh SHALL cover the arena floor
- **AND** red and blue spawners SHALL generate units that autonomously select opposing-team targets without unrelated scene actors present

#### Scenario: Moving-target demo map loads and runs
- **WHEN** `MAP_TsnDemo_MovingTarget` is opened in PIE
- **THEN** NavMesh SHALL cover the arena floor
- **AND** pre-placed spawners SHALL generate units that autonomously pursue a spline-moving target dummy

#### Scenario: Automation tests sanitize the base showcase map
- **WHEN** `MAP_TsnDemo_Siege` is loaded by automation tests
- **THEN** the tests SHALL destroy pre-placed `ATsnTestSpawner`, `ATsnTestChessPiece`, and `ATsnTestTargetDummy` actors before test setup
- **AND** all test actors used for verification SHALL still be spawned and destroyed by test code

### Requirement: Automated Functional Tests

The test module SHALL include Automation Spec tests for pure calculation logic and Functional Test Actors for runtime behavior verification, executable via the UE Session Frontend Automation panel with zero manual intervention beyond clicking "Start Tests".

#### Scenario: Slot calculation unit test
- **WHEN** `TsnSlotCalculationTest` Automation Spec runs from Session Frontend
- **THEN** it SHALL verify that `CalculateWorldPosition(AngleDeg, Radius)` returns correct world positions for 7 test cases covering 0°/90°/180°/360°/-90° angles, non-origin offsets, and zero-radius degeneration
- **AND** all results SHALL be within ±1.0cm tolerance

#### Scenario: Repulsion vector unit test
- **WHEN** `TsnRepulsionVectorTest` Automation Spec runs from Session Frontend
- **THEN** it SHALL verify the dual-stage repulsion model: inner zone quadratic falloff, outer zone linear falloff with 10% ratio cap, zero force beyond RepulsionRadius
- **AND** it SHALL verify repulsion direction has positive radial dot product (points away from obstacle)

#### Scenario: Slot assignment functional test
- **WHEN** `SlotAssignmentTest` runs via Automation
- **THEN** it SHALL automatically load `MAP_TsnDemo_Siege`, clear showcase actors, spawn 1 TargetDummy, directly call `RequestSlot()` for 6 actors with mixed attack ranges (no BT dependency)
- **AND** it SHALL draw slot rings and occupied positions using `DrawDebug*` functions with 10s duration
- **AND** it SHALL verify: all 6 slots assigned, units on correct ring by attack range (±60cm tolerance), same-ring slots have angular separation

#### Scenario: Stance avoidance functional test
- **WHEN** `StanceAvoidanceTest` runs via Automation
- **THEN** it SHALL automatically load `MAP_TsnDemo_Siege`, clear showcase actors, spawn 3 stance units + 1 mover, drive movement via `AIController::MoveToLocation()` (no BT dependency)
- **AND** it SHALL draw stance radii, mover trajectory trail, and repulsion vectors using `DrawDebug*` functions
- **AND** it SHALL verify: mover reached destination AND minimum distance to any stance unit >= ObstacleRadius

#### Scenario: Full battle cycle functional test
- **WHEN** `FullBattleCycleTest` runs via Automation
- **THEN** it SHALL automatically load `MAP_TsnDemo_Siege`, clear showcase actors, spawn 4 attackers + 1 target, load `BT_Tsn_Test` and `BB_Tsn_Test`, and run BT-driven three-phase combat cycle
- **AND** it SHALL display phase status per unit using `DrawDebugString` and `AddOnScreenDebugMessage`
- **AND** it SHALL verify: all units reached STANCE phase, slot system peaked at expected count, melee and ranged on different rings, within 25s timeout

### Requirement: Demo StateTree Tasks (Test-Module Only)

测试模块 `TireflySquadNavigationTest` SHALL 提供两个 demo-only StateTree 任务，与 `UTsnTestBTTask_SelectTarget` 与 `UTsnTestBTTask_Attack` 的语义一一对应，使纯蓝图测试指南中的 Siege / Skirmish / MovingTarget 三个演示场景可以在不依赖 Behavior Tree 的前提下用 StateTree 复现。这两个任务 SHALL 仅存在于 Test 模块，运行时插件 `TireflySquadNavigation` 中 SHALL NOT 包含等价节点。

#### Scenario: SelectTarget StateTree task

- **WHEN** `FTsnTestStateTreeTask_SelectTarget` 在一个 AI StateTree 中执行
- **THEN** 该任务 SHALL 遍历当前 World 中的 `ATsnTestChessPiece` 与 `ATsnTestTargetDummy`
- **AND** SHALL 按 `TeamID` 过滤敌方棋子（`TargetDummy` 始终视为有效目标）
- **AND** SHALL 在 `SearchRadius`（默认 5000cm）内选出 2D 距离最近的目标
- **AND** SHALL 把命中目标写入 `Output` 类别的 `TargetActor` 与 `bHasTarget` 字段，供下游状态通过 StateTree 绑定消费
- **AND** 在 `EnterState` 内同步返回 Succeeded（命中）或 Failed（无命中）

#### Scenario: Attack StateTree task

- **WHEN** `FTsnTestStateTreeTask_Attack` 在一个 AI StateTree 中执行
- **THEN** 该任务 SHALL 在 `EnterState` 启动一个时长为 `AttackDuration`（默认 2.0s）的倒计时
- **AND** SHALL 在 `Tick` 中累计经过时间，到时返回 Succeeded
- **AND** 当 `AIController` 或其 `Pawn` 失效时 SHALL 立即返回 Failed
- **AND** SHALL NOT 触碰任何插件运行时资源（不申请 / 释放槽位、不切换站姿模式）

#### Scenario: Demo nodes never leak into runtime plugin

- **GIVEN** 项目以 Shipping 配置打包
- **WHEN** 打包流程剥离 Editor-only 模块
- **THEN** `FTsnTestStateTreeTask_SelectTarget` 与 `FTsnTestStateTreeTask_Attack` SHALL NOT 出现在最终包内
- **AND** 这两个任务的 `USTRUCT` SHALL NOT 在 `TireflySquadNavigation` 运行时模块的任何头文件中被引用

### Requirement: Test Module StateTree Module Dependency

`TireflySquadNavigationTest.Build.cs` SHALL 在私有依赖中声明 `StateTreeModule`，以便测试模块可以编译自定义 StateTree 任务基类。

#### Scenario: StateTree module declared in test build script

- **WHEN** 项目执行 `SquadNavDevEditor Win64 Development Build`
- **THEN** `TireflySquadNavigationTest.Build.cs` 的 `PrivateDependencyModuleNames` SHALL 包含 `StateTreeModule`
- **AND** 测试模块内引用 `StateTreeTaskBase.h` / `StateTreeExecutionContext.h` 的源文件 SHALL 编译通过

