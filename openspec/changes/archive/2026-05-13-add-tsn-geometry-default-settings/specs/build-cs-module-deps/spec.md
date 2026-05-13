## MODIFIED Requirements

### Requirement: 构建系统 SHALL 添加导航与 AI 相关模块依赖

插件的 `TireflySquadNavigation.Build.cs` SHALL 按以下规则添加新依赖：

- **PublicDependencyModuleNames**：新增 `NavigationSystem`。原因：`UTsnNavArea_StanceUnit`（公共头文件）继承自 `UNavArea`；宿主项目 `#include` 插件公共头时需要该模块的传递可见性，必须放 Public。
- **PrivateDependencyModuleNames**：新增 `AIModule`（`UCrowdFollowingComponent`、BT 基类）、`GameplayTasks`（`UBTTaskNode` 异步执行基础）、`StateTreeModule`（TSN 自定义 StateTree 任务 / 条件 / 评估器基类与执行上下文）、`DeveloperSettings`（TSN 插件级 `UDeveloperSettings` 类型定义与运行时访问）。这些依赖仅在插件实现层使用，放 Private 防止污染宿主项目公共 API。

#### Scenario: 不添加 StateTreeModule 时编译失败

- **GIVEN** 插件开始包含 `StateTreeTaskBase.h`、`StateTreeExecutionContext.h` 等 StateTree 头文件
- **WHEN** `Build.cs` 未声明 `StateTreeModule` 私有依赖
- **THEN** UBT 报告相关 StateTree 头文件或符号不可见，Build 失败

#### Scenario: 不添加 DeveloperSettings 时编译失败

- **GIVEN** 插件新增了继承 `UDeveloperSettings` 的 TSN 配置类型
- **WHEN** `Build.cs` 未声明 `DeveloperSettings` 私有依赖
- **THEN** UBT 报告 `DeveloperSettings` 相关头文件或符号不可见，Build 失败

#### Scenario: 添加 StateTreeModule 和 DeveloperSettings 后运行时模块可以编译

- **GIVEN** `Build.cs` 已新增 `StateTreeModule` 与 `DeveloperSettings` 私有依赖
- **WHEN** 执行 Editor Development Build
- **THEN** 自定义 TSN StateTree 节点与 TSN 插件级 settings 相关的模块依赖错误不再出现