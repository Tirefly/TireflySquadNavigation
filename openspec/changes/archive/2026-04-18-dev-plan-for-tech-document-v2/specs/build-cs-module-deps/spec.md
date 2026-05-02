# Spec Delta: build-cs-module-deps

## ADDED Requirements

### Requirement: 构建系统 SHALL 添加导航与 AI 相关模块依赖

插件的 `TireflySquadNavigation.Build.cs` SHALL 按以下规则添加新依赖：

- **PublicDependencyModuleNames**：新增 `NavigationSystem`。原因：`UTsnNavArea_StanceUnit`（公共头文件）继承自 `UNavArea`；宿主项目 `#include` 插件公共头时需要该模块的传递可见性，必须放 Public。
- **PrivateDependencyModuleNames**：新增 `AIModule`（`UCrowdFollowingComponent`、BT 基类）、`GameplayTasks`（`UBTTaskNode` 异步执行基础）。两者仅在插件 .cpp 内使用，放 Private 防止污染宿主项目公共 API。

#### Scenario: 不添加 NavigationSystem 时编译失败

**Given** 当前 Build.cs 中无 NavigationSystem 依赖  
**When** 代码包含 `#include "NavAreas/NavArea.h"` 或 `#include "NavigationSystem.h"`  
**Then** UBT 报告找不到头文件，Build 失败

#### Scenario: 添加依赖后成功编译

**Given** Build.cs 已添加所有必要模块依赖  
**When** 执行 `SquadNavDevEditor Win64 Development Build`  
**Then** 与模块依赖相关的编译错误不再出现

---

## 文件影响

| 文件 | 操作 |
|------|------|
| `TireflySquadNavigation.Build.cs` | MODIFIED：PublicDependencyModuleNames 加 `NavigationSystem`；PrivateDependencyModuleNames 加 `AIModule`、`GameplayTasks` |
