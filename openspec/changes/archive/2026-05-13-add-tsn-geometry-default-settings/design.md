## Context

当前 TSN 插件里真正承载“默认几何”语义的只有 `UTsnStanceObstacleComponent` 上的几个编辑器属性：`ObstacleRadius`、`NavModifierExtraRadius`、`RepulsionRadius`、`RepulsionStrength`。这使得插件没有一个正式的集中默认值入口，也不利于说明“哪些值是全局默认，哪些值是某个单位的特殊 override”。

这次变更的目标不是把宿主项目的玩法尺度搬进插件，而是给插件补一层明确的默认值来源，让宿主集成能够建立稳定的三层解析顺序：宿主玩法尺度、TSN 默认预算、单位局部 override。

## Goals

- 为 TSN 提供编辑器可见的默认几何配置入口。
- 让 `UTsnStanceObstacleComponent` 默认跟随插件 settings，但保留局部 override 能力。
- 不改变现有运行时 sanitize / NavModifier / RepulsionSubsystem 的基本职责。
- 保持调试 overlay 读取到的是最终 resolved 半径，而不是混杂的原始字段。

## Non-Goals

- 本次不把所有 TSN 组件都迁移到 `DeveloperSettings`。
- 本次不在插件层直接编码宿主项目的格子长度或攻击距离公式。
- 本次不实现新的 debug overlay 图层或命令。

## Decisions

### Decision: Add one plugin-level settings object for TSN default geometry

新增一个 TSN 插件级 `UDeveloperSettings`，集中保存 stance obstacle 默认几何预算。首批字段至少包括：

- `DefaultObstacleRadius`
- `DefaultNavModifierExtraRadius`
- `DefaultRepulsionRadius`
- `DefaultRepulsionStrength`
- `DefaultUseNavModifier`
- `DefaultNavModifierMode`
- `DefaultNavModifierDeactivationDelay`

这层设置只表达“插件默认预算”，不负责推导宿主玩法尺度。

### Decision: Resolve component values through follow-defaults plus local overrides

`UTsnStanceObstacleComponent` 保留当前面向实例的可编辑能力，但不再直接把原始属性当作默认真值。解析顺序改为：

1. 组件显式 override
2. TSN 插件 `DeveloperSettings`
3. 代码内最后 fallback

这样普通单位不需要逐个填值，特殊单位也不必失去单独调参能力。

### Decision: Keep sanitize at the resolved-value boundary

`RepulsionRadius > ObstacleRadius` 等不变量仍然在运行时边界统一处理，但输入值应来自 resolved 配置，而不是散落在各个调用点的原始字段。这样 debug 读取、NavModifier 更新和 repulsion 注册能看到同一套最终值。

### Decision: Document the layering explicitly for host integrations

README 与技术方案文档需要把这层新边界写清楚：宿主项目负责玩法尺度，TSN settings 负责默认预算，组件 override 负责个例。否则仅靠代码实现，后续集成仍会回到“看起来能调，但不知道该在哪层调”的状态。

## Risks / Trade-offs

- 新增 `UDeveloperSettings` 需要引入额外模块依赖；若 `Build.cs` 未同步更新，编译会直接失败。
- 如果 resolved / raw 字段边界写得不清楚，调试时会再次出现“编辑器显示一个值，运行时实际用另一个值”的混乱。
- 组件 override 字段过多会让面板变得啰嗦，因此命名必须明确区分“跟随默认值”和“局部覆写”。

## Validation Plan

1. 先通过 OpenSpec 校验 change 结构和 spec delta。
2. 再编译工程，确认 `DeveloperSettings`、模块依赖和组件改动成立。
3. 最后更新 change 的 tasks 状态，使其与实际完成度一致。