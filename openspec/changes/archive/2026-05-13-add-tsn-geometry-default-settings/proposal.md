# Change: add TSN geometry default settings

## Why

`UTsnStanceObstacleComponent` 当前把 obstacle / nav modifier / repulsion 的默认值直接硬编码在组件属性上。这会让插件默认几何预算既不容易在编辑器中集中调节，也无法明确区分“插件默认值”和“单个单位的局部特殊值”。

LegendsTD 这次集成已经明确提出需要一层 TSN 插件自己的开发者设置默认值，同时仍允许单位组件做覆盖。这个能力应该先在插件工作区里被正式建模，而不是只停留在宿主项目文档里。

## What Changes

- 新增 TSN 插件级 `UDeveloperSettings`，作为 stance obstacle / nav modifier / repulsion 默认几何参数的编辑器入口。
- 修改 `UTsnStanceObstacleComponent` 的参数解析方式：默认跟随 TSN 插件默认值，支持逐组件显式 override。
- 保留现有运行时 sanitize 与调试读取接口，但把“默认值来源”从硬编码属性转为 settings + override。
- 更新插件 README 与说明文档，明确配置层级和 LegendsTD 近战几何约束。

## Impact

- Affected specs:
  - `stance-obstacle-component`
  - `build-cs-module-deps`
- Affected code:
  - `Source/TireflySquadNavigation/TireflySquadNavigation.Build.cs`
  - `Source/TireflySquadNavigation/Public|Private/Components/TsnStanceObstacleComponent.*`
  - `Source/TireflySquadNavigation/Public|Private/Settings/*`
  - `README.md`
  - `Documents/战场小队导航系统 - 技术方案文档 V2.md`