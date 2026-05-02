# build-cs-module-deps Specification

## Purpose
TBD - created by archiving change dev-plan-for-tech-document-v2. Update Purpose after archive.
## Requirements
### Requirement: 构建系统 SHALL 添加导航与 AI 相关模块依赖

插件的 `TireflySquadNavigation.Build.cs` SHALL 按以下规则添加新依赖：

- **PublicDependencyModuleNames**：新增 `NavigationSystem`。原因：`UTsnNavArea_StanceUnit`（公共头文件）继承自 `UNavArea`；宿主项目 `#include` 插件公共头时需要该模块的传递可见性，必须放 Public。
- **PrivateDependencyModuleNames**：新增 `AIModule`（`UCrowdFollowingComponent`、BT 基类）、`GameplayTasks`（`UBTTaskNode` 异步执行基础）、`StateTreeModule`（TSN 自定义 StateTree 任务 / 条件 / 评估器基类与执行上下文）。这些依赖仅在插件实现层使用，放 Private 防止污染宿主项目公共 API。

#### Scenario: 不添加 StateTreeModule 时编译失败

- **GIVEN** 插件开始包含 `StateTreeTaskBase.h`、`StateTreeExecutionContext.h` 等 StateTree 头文件
- **WHEN** `Build.cs` 未声明 `StateTreeModule` 私有依赖
- **THEN** UBT 报告相关 StateTree 头文件或符号不可见，Build 失败

#### Scenario: 添加 StateTreeModule 后运行时模块可以编译自定义 StateTree 节点

- **GIVEN** `Build.cs` 已新增 `StateTreeModule` 私有依赖
- **WHEN** 执行 `SquadNavDevEditor Win64 Development Build`
- **THEN** 自定义 TSN StateTree 任务 / 条件 / 评估器相关的模块依赖错误不再出现

