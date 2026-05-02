# Change: 新增测试模组与演示场景

## Why

插件核心代码（`TireflySquadNavigation` 模块）已实现完毕，需要一套完整的测试 / 演示体系来验证四层架构的正确性，同时作为插件发布后的**功能展示场景**内嵌于插件目录中。

## What Changes

- 新增 **Editor-only** 模块 `TireflySquadNavigationTest`，位于插件内部，专门承载测试 / 演示代码与资产
- 实现测试用 C++ 类：AI 棋子 Character、AIController（集成 CrowdFollowing）、`ITsnTacticalUnit` 具体实现
- 设计完整的 AI 行为树（使用插件 6 个 BT 节点 + 引擎标准节点）
- 搭建 **3 张相互隔离** 的演示地图：围攻单目标、两组对战、移动目标追击
- 提供蓝图子类支持在编辑器内快速调参
- 评估并有选择地引入自动化功能测试（Automation Spec）

## Impact

- Affected specs: 无（不修改现有插件功能代码）
- Affected code: 
  - 新增 `Plugins/TireflySquadNavigation/Source/TireflySquadNavigationTest/` 模块
  - 新增 `Plugins/TireflySquadNavigation/Content/FunctionShowcase/` 演示资产（3 张演示地图、BT、BB、蓝图）
  - 修改 `TireflySquadNavigation.uplugin`（注册新模块）
