## 1. Spec And Design

- [x] 1.1 为 TSN 插件新增 change，并定义 `DeveloperSettings` 默认值与组件 override 的配置层级。
- [x] 1.2 为 `stance-obstacle-component` 写入 spec delta，明确默认值来源和 override 语义。
- [x] 1.3 为 `build-cs-module-deps` 写入 spec delta，反映 `DeveloperSettings` 模块依赖。

## 2. Implementation

- [x] 2.1 新增 TSN 插件级 `UDeveloperSettings` 类型，暴露 stance obstacle / nav modifier / repulsion 的默认几何参数。
- [x] 2.2 修改 `UTsnStanceObstacleComponent`，增加“跟随插件默认值 / 使用局部 override”的解析路径。
- [x] 2.3 让运行时 sanitize、NavModifier 更新、RepulsionSubsystem 注册与 debug 读取统一使用 resolved 几何值，而不是直接读取原始属性。
- [x] 2.4 更新插件 README 与技术方案文档，写明默认值解析顺序与 LegendsTD 近战硬几何语义。

## 3. Validation

- [x] 3.1 在插件目录执行 `openspec validate add-tsn-geometry-default-settings --strict --no-interactive`。
- [x] 3.2 编译 `LegendsTDEditor Win64 Development`，确认新增 settings 类型与组件改动不会引入构建错误。