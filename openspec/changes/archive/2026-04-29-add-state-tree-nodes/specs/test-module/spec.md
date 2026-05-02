## ADDED Requirements

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
