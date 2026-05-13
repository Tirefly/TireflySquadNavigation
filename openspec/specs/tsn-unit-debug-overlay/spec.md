# tsn-unit-debug-overlay Specification

## Purpose
定义 `TireflySquadNavigation` 当前已经提供的运行时单位调试绘制能力，包括统一的多半径 overlay、命令行控制，以及单单位 focus 下的目标侧补充显示。

## Requirements
### Requirement: 系统 SHALL 提供统一的多半径单位调试 overlay

系统 SHALL 通过 `UTsnDebugDrawSubsystem` 提供统一的运行时单位调试 overlay，能够在一个入口中读取并绘制战术单位的 capsule、engagement、TSN stance/nav/repulsion、Crowd、separation 与目标侧 slot 相关信息。

该 overlay SHALL 采用固定 lane 做 Z 向错层，而不是把所有圆压在单位本地同一平面。

#### Scenario: Overview 预设以低噪声方式显示多个单位

**Given** 运行时场上存在多个接入 TSN 的战术单位  
**When** 用户开启 `tsn.debug.DrawUnitOverlay 1` 且 preset 保持默认 `overview`  
**Then** 系统以低噪声方式为多个单位绘制关键半径，并避免同一单位的所有圆完全重叠在同一高度

#### Scenario: Focus 预设显示单单位的完整调试信息

**Given** 一个战术单位已接入 TSN 运行时  
**When** 用户把 overlay 收敛到该单位并切换到 `focus` 预设  
**Then** 系统必须绘制该单位的 capsule、engagement、TSN stance/nav/repulsion、Crowd、separation、文本标签与关键速度向量信息

---

### Requirement: 系统 SHALL 支持 preset、mask、过滤器与目标对补充显示

统一 overlay SHALL 提供 `overview`、`focus`、`tsn`、`crowd` 四类主要预设，并支持 layer mask 覆盖。

统一 overlay SHALL 支持 `all`、`nearest`、`name` 三种范围过滤；目标对信息 MUST 作为单单位 focus 下的可选补充显示，而不是独立过滤模式。

在 `all` 过滤模式下，系统 SHALL 继续为每个单位绘制其自身已认领槽位的占据圈与指向箭头；但目标侧完整 slot ring 补画 MUST 仅在最终只绘制 1 个单位时启用，避免全场图例失控。

#### Scenario: 按最近单位聚焦并额外查看目标侧接战信息

**Given** 最近的战术单位当前存在合法目标  
**When** 用户切换到最近单位过滤，并启用目标对补充显示  
**Then** 系统保留该单位自身 overlay，并额外绘制其当前目标一侧的 slot ring、已认领槽位位置与相关标签

#### Scenario: 按名称过滤时未开启目标对补充显示

**Given** 用户通过名称过滤把绘制范围收敛到一个单位  
**When** 目标对补充显示开关未开启  
**Then** 系统只绘制该单位自身 overlay，而不自动补画目标侧接战槽信息

#### Scenario: 全场模式下查看所有单位的自身槽位指向

**Given** 运行时场上存在多个接入 TSN 的战术单位，且用户不想把调试收敛到某一个单位  
**When** 用户使用 `all` 过滤模式并开启包含 `slots` layer 的 preset 或 mask  
**Then** 系统必须为每个已占槽单位绘制自己的槽位占据圈和“单位 -> 槽位中心”的指向箭头

#### Scenario: 全场模式下不补画每个单位目标侧完整 slot ring

**Given** 用户正在 `all` 过滤模式下绘制多个单位  
**When** `UnitOverlayTargetPair` 开关处于开启状态  
**Then** 系统仍不应为每个单位额外补画其目标侧完整 slot ring，而应只保留每个单位自身的槽位占据圈与指向箭头，避免全场图例过度拥挤

---

### Requirement: 系统 SHALL 通过命令行控制统一 overlay

统一 overlay MUST 通过 `tsn.debug.*` 控制台变量和命令提供运行时开关、preset、layer mask、文本标签、目标对补充显示、过滤范围、lane 间距和最大单位数量控制，而不依赖修改 Actor 或组件上的编辑器属性。

插件 README 与相关 spec 文档 SHALL 明确列出这些命令的作用、推荐组合方式，以及 `preset` / `filter` 的可用枚举值，避免用户只能通过翻 C++ 常量来反查调试入口。

#### Scenario: 运行时切换 overlay 参数立即生效

**Given** 项目运行在 Development 或 Debug 环境  
**When** 用户通过命令行切换 preset、layer mask、文本标签、目标对补充显示、过滤范围、lane 间距或最大单位数量  
**Then** 调试绘制结果必须在运行时立即变化，而不需要修改单个 Actor 或组件上的编辑器属性

#### Scenario: 名称过滤通过显式命令设置

**Given** 用户要观察名称包含特定片段的单位  
**When** 用户执行 `tsn.debug.UnitOverlayFocusName <ActorNameSubstring>`  
**Then** 系统将过滤模式切到名称过滤，并只绘制匹配该名称片段的单位

#### Scenario: 文档明确列出 overlay 命令和常用组合

**Given** 用户需要临时排查 TSN 接敌、槽位或 repulsion 问题，但不想阅读实现代码  
**When** 用户查看插件 README 或相关 spec 文档  
**Then** 文档必须能直接说明每个 `tsn.debug.*` 命令的作用、推荐调试方案，以及常用的 focus / target-pair / slot-height / all-units-slot 组合命令

---

### Requirement: 系统 SHALL 只读取运行时状态，不改变 TSN 行为

统一 overlay MUST 只读取现有运行时状态，不能通过 `RequestSlot()`、`EnterStanceMode()`、`ExitStanceMode()` 或新的移动请求来取数或刷新图形。

为满足这一约束，插件 SHALL 提供只读调试访问器，用于读取 stance obstacle、Crowd 和 engagement slot 的调试数据。

#### Scenario: 调试刷新不会重新认领槽位或触发移动

**Given** 一个战术单位处于接敌、脱战或空闲中的任意状态  
**When** 统一 overlay 刷新该单位的调试数据  
**Then** 系统只读取当前已有的 slot、stance、Crowd 与 movement 状态，而不会重新认领槽位、改变 stance 或发出新的移动请求

---

### Requirement: 系统 SHALL 将运行时单位调试收敛到统一 overlay

系统 SHALL 以 `UTsnDebugDrawSubsystem` 作为运行时单位调试绘制的唯一正式入口，并 retire 掉容易造成图例歧义的 legacy raw circles / arrows。旧调试路径中仍有价值的低层状态信息 MUST 迁移进统一 overlay，而不是继续保留另一套平行的运行时图例。

#### Scenario: Focus overlay 显示 nav modifier 状态与模式

**Given** 一个带有 `UTsnStanceObstacleComponent` 的战术单位被收敛到单单位 focus 诊断  
**When** 统一 overlay 刷新该单位的文本层  
**Then** 系统必须显示 nav modifier 是否禁用、当前是否激活，以及对应模式标签

#### Scenario: 运行时只需要一套正式调试图例

**Given** 用户正在排查 TSN 单位的导航、排斥和接敌问题  
**When** 用户通过 `tsn.debug.DrawUnitOverlay` 与 `UnitOverlay*` 控制项切换运行时调试绘制  
**Then** 单位半径、速度向量、slot 补充信息和低层状态都必须通过统一 overlay 的 lane、颜色与文本契约读取，而不需要再额外切换 raw debug 入口

---

### Requirement: 文档 SHALL 解释统一 overlay 的颜色与图案语义

插件 README 与相关 spec 文档 SHALL 解释统一 overlay 中各颜色、圆环、球、箭头、连线与文本层分别代表什么，尤其要明确区分 `engagement`、`slot ring`、`slot occupancy`、`requested velocity` 与 `actual velocity` 这些容易混淆的可视元素。

#### Scenario: 用户可以根据图例判断当前看到的槽位图形含义

**Given** 用户已经在运行时开启统一 overlay，看到目标侧 slot ring、黄色占据圈和若干箭头  
**When** 用户查询插件 README 或相关 spec 文档  
**Then** 文档必须能明确解释这些图形分别对应目标侧接战环、当前单位最终占据范围、单位到槽位中心的指向，以及当前速度 / 请求速度 / repulsion / escape 向量的区别

---