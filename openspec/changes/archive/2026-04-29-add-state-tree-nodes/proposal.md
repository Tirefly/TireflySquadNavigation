# Change: 新增 StateTree 决策节点

## Why

当前插件在决策层只提供 Behavior Tree 节点，技术文档和测试指南都把 StateTree 明确标为“预留扩展位”。宿主项目如果想在 StateTree 中复用 TSN 的追击、占位、站姿切换和槽位释放能力，不仅缺少动作任务，还缺少可供 transition 消费的共享战斗上下文与轻量条件判断，只能自行重写一套状态逻辑。

## What Changes

- 新增一组运行时 `StateTree` 任务节点，覆盖 TSN 当前三阶段战斗循环中的核心动作：追击目标、接近交战槽位、进入站姿、退出站姿、释放槽位
- 新增一组 `StateTree` 评估器，向 transition 和任务暴露共享战斗上下文：`CombatContext`、`TargetMotion`
- 新增一组 `StateTree` 条件，覆盖目标有效性、距离区间、目标静止、站姿状态等轻量判断
- 为任务、评估器、条件补齐显式 `InstanceData` 输入 / 输出契约，明确哪些字段由宿主绑定、哪些字段由 TSN 节点产出
- 让这些 StateTree 节点直接复用现有底层能力（`ITsnTacticalUnit`、`UTsnEngagementSlotComponent`、`UTsnStanceObstacleComponent`、AI MoveTo），而不是包装现有 BT 节点
- 扩展 `TireflySquadNavigation.Build.cs` 的私有依赖，使插件运行时模块可以编译并暴露自定义 StateTree 任务 / 条件 / 评估器
- 在 `TireflySquadNavigationTest` 测试模块中新增两个 demo-only StateTree 任务（`SelectTarget`、`Attack`），以便用 StateTree 复现纯蓝图测试指南的三个演示场景；这两个节点严格留在 Test 模块，不进运行时插件
- 补充文档中的 StateTree 接入说明，明确最小推荐状态链、推荐评估器 / 条件绑定方式，以及与现有 BT 自动化测试的边界

## Impact

- Affected specs:
  - 新增 `state-tree-nodes`
  - 修改 `build-cs-module-deps`
  - 修改 `test-module`（新增 demo-only StateTree 任务要求）
- Affected code:
  - 新增 `Plugins/TireflySquadNavigation/Source/TireflySquadNavigation/Public/StateTree/`
  - 新增 `Plugins/TireflySquadNavigation/Source/TireflySquadNavigation/Private/StateTree/`
  - 修改 `Plugins/TireflySquadNavigation/Source/TireflySquadNavigation/TireflySquadNavigation.Build.cs`
  - 新增 `Plugins/TireflySquadNavigation/Source/TireflySquadNavigationTest/Public/StateTree/` 与对应 `Private/StateTree/`
  - 修改 `Plugins/TireflySquadNavigation/Source/TireflySquadNavigationTest/TireflySquadNavigationTest.Build.cs`
  - 更新 `Plugins/TireflySquadNavigation/Documents/战场小队导航系统 - 技术方案文档 V2.md`
  - 更新 `Plugins/TireflySquadNavigation/Documents/BehaviorTree 测试执行指南.md`