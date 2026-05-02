# Proposal: make-dev-plan-for-tech-plan-document-v2

## Summary

将《技术方案文档 V2》中设计的全部 C++ 类从 **零实现** 状态落地到可编译的完整插件代码。本变更涵盖 `TireflySquadNavigation` 插件的四层架构：接口层、组件层、子系统层、行为树节点层，以及对应的 NavArea 类和 Build.cs 模块依赖更新；其中站桩排斥力注入路径遵循 V2 已定稿的**至少 1 帧延迟消费**契约。

## Motivation

当前插件状态：**预实现阶段**——Public/Private 目录为空，仅有模块存根代码（`TireflySquadNavigation.h/.cpp`）。技术方案文档 V2 已给出所有类的完整接口定义与实现代码参考；本提案将其系统性地转化为实际文件，保证：
1. 每个文件可通过 UBT 编译（SquadNavDevEditor Win64 Development）
2. 命名规范、架构模式与技术文档 V2 完全一致
3. 各能力模块之间依赖关系正确，无循环引用

## Change ID

`make-dev-plan-for-tech-plan-document-v2`

## Scope

本提案覆盖以下七个能力域（每个对应一个 spec delta）：

| 能力域 | 核心类 | 描述 |
|--------|--------|------|
| `tactical-unit-interface` | `ITsnTacticalUnit` / `UTsnTacticalUnit` | 宿主接口，插件与宿主项目通信边界 |
| `tactical-movement-component` | `UTsnTacticalMovementComponent` | CMC 子类，合法排斥力注入 |
| `engagement-slot-component` | `UTsnEngagementSlotComponent` | 多环角度槽位分配与管理 |
| `stance-obstacle-component` | `UTsnStanceObstacleComponent` + `ETsnMobilityStance` + `ETsnNavModifierMode` | 站桩障碍物状态切换 |
| `stance-repulsion-subsystem` | `UTsnStanceRepulsionSubsystem` | Moving↔Stance 双阶段排斥力世界子系统 |
| `unit-separation-component` | `UTsnUnitSeparationComponent` | 仅 Moving↔Moving 的局部分离力 |
| `engagement-slot-subsystem` | `UTsnEngagementSlotSubsystem` | 全局槽位注册表，保证单目标槽位约束 |
| `behavior-tree-nodes` | 5 个 BT Task + 1 个 BT Decorator | 完整战斗 AI 行为树节点 |
| `nav-area-stance-unit` | `UTsnNavArea_StanceUnit` | 站桩单位高代价导航区域 |
| `tsn-log` | `LogTireflySquadNav` | 插件统一日志分类 |
| `build-cs-module-deps` | `TireflySquadNavigation.Build.cs` | 模块依赖更新 |

## Out of Scope

- StateTree 节点（文档预留扩展位，本次不实现）
- 宿主项目测试场景（可在完成插件后独立搭建）
- 自动化测试框架（当前验证方式为 UBT 编译 + 编辑器运行）
- 性能优化（插件定位为小规模战斗，优先正确性）
- 网络同步 / 多人模式：本实现仅针对单人或服务器权威模式（AI Pawn 无需客户端预测），不处理 Replicate 或客户端校正

## Key Decisions

1. **文件严格按文档 V2 指定路径创建**：Public/Interfaces/、Public/Components/、Public/Subsystems/、Public/BehaviorTree/、Public/NavAreas/，Private 镜像相同结构。
2. **Build.cs 先于具体类文件更新**：确保新增模块依赖（`NavigationSystem`（Public）、`AIModule`、`GameplayTasks`）在编译时已就位。
3. **注释规范严格遵循项目约定**：类/结构体/枚举声明、成员变量与函数均需注释；超过 2 参数的函数逐参注释；有返回值的函数注明返回含义。
4. **TsnBTTask_EnterStanceMode / ExitStanceMode / ReleaseEngagementSlot 的实现体写在头文件**：内联实现较短，符合文档 V2 中的写法。
5. **站桩排斥力不追求同帧生效**：`UTsnStanceRepulsionSubsystem` 在帧 N 只负责写入 `UTsnTacticalMovementComponent` 的延迟缓冲，消费固定发生在帧 N+1 的 `CalcVelocity`（`Super` 之后）。UE 5.7 中 `RequestDirectMove` 只是输入端（写入 `RequestedVelocity`），最终都由 `CalcVelocity` 消费，因此排斥力只需在 `CalcVelocity` 统一注入一次，不再另行重写 `RequestDirectMove`。
6. **SetRepulsionVelocity 累加语义 + RepulsionSubsystem 内部合并**：RepulsionSubsystem 在内部遍历所有 Stance 单位后先合并排斥力，再对每个 Moving 单位**只调用一次** `SetRepulsionVelocity`；`ensureMsgf` 仅在**不同系统**同帧调用时触发警告。
7. **RVO Avoidance 在 TsnTacticalMovementComponent 构造函数中自动关闭**：从根源消除与 DetourCrowd 的双重避障冲突。
8. **RepulsionSubsystem 遍历范围收窄**：通过 MovementComponent 注册机制维护战术单位列表，避免遍历场景中所有 Character。
9. **UnitSeparation 多 Stance 单位裁剪采用累加方向后单次裁剪**：避免遍历顺序依赖和过度衰减。
10. **BT 节点 OnDestroyed 委托在 OnTaskFinished 中统一解绑**：防止委托泄漏。
11. **所有 Public 类使用 `TIREFLYSQUADNAVIGATION_API` 模块导出宏**：确保宿主项目正确链接。
12. **BT 节点不使用 FallbackEngagementRange**：当 Pawn 未实现 `ITsnTacticalUnit` 或接口返回非法值（≤ 0）时，BT Task 直接返回 `Failed`，由 BT 树结构的失败分支处理恢复逻辑。
13. **TsnStanceObstacleComponent 提供 UpdateStanceUnitParams() 运行时更新接口**：宿主项目可在运行时修改 ObstacleRadius / RepulsionRadius / RepulsionStrength 后调用此方法，同步更新 NavModifier 和 RepulsionSubsystem 中缓存的参数。
14. **MoveToEngagementSlot 支持对象池回收场景**：除 `OnDestroyed` 外，额外监听 `OnEndPlay`，确保目标被对象池回收（SetActorHiddenInGame + 禁用碰撞而非真正销毁）时也能正确释放槽位并结束任务。
15. **排斥力与分离力系统提供 `#if ENABLE_DRAW_DEBUG` 调试可视化**：RepulsionSubsystem、TacticalMovementComponent、UnitSeparationComponent 均包含调试绘制支持，方便运行时参数调优。
16. **单目标槽位约束**：同一攻击者同一时间只能在一个目标上持有槽位。通过 `UTsnEngagementSlotSubsystem`（`UWorldSubsystem`）维护全局 `Requester → SlotComponent` 映射。`RequestSlot` 内部查询子系统，若请求者已在其他目标上持有槽位，自动释放旧槽位后再分配新槽位。
17. **Stance 状态下不重新申请槽位**：攻击者进入 Stance（站桩攻击）后，除非目标死亡或自身受到强制切换目标的控制效果（如魅惑、嘲讽），否则不应重新触发 `RequestSlot`。BT 树结构负责保证此约束。
18. **ObstacleRadius 编辑器约束**：`TsnStanceObstacleComponent` 的 `ObstacleRadius` 默认值为 `60.f`（约等于标准胶囊体半径），并添加 `ClampMin = "10.0"` 元数据，防止配置为零或负值。`RepulsionRadius` 默认值为 `120.f`，添加 `ClampMin = "20.0"`。
19. **外层排斥力系数可配置**：`TsnStanceRepulsionSubsystem` 中的外层排斥力阶段系数（原硬编码 `0.1f`）提升为 `UPROPERTY`（`OuterRepulsionRatio`，默认 `0.1f`），宿主项目可在编辑器中调整外层/内层的力度比例。
20. **站桩穿透防护（Stance Penetration Guard）**：`TsnTacticalMovementComponent` 在 `CalcVelocity` 的最终速度确定后，增加第三层防线——对每个处于 `ObstacleRadius` 内的 Stance 单位，裁剪最终速度中朝向该单位的分量至零。这是一个硬约束：无论路径跟随、分离力、排斥力如何合成，单位在 ObstacleRadius 边界处不可能有向内速度分量。
21. **排斥力方向混合比例可配置**：`TsnStanceRepulsionSubsystem` 的切向混合比例（原硬编码 70%/30%）提升为 `UPROPERTY`（`TangentBlendRatio`，默认 `0.7f`，`ClampMin = 0.0`，`ClampMax = 1.0`），宿主项目可调整切向引导与径向推开的比例。
22. **对象池回收支持**：所有持有运行时状态的组件（`TsnTacticalMovementComponent`、`TsnStanceObstacleComponent`、`TsnEngagementSlotComponent`、`TsnUnitSeparationComponent`）提供 `OnOwnerReleased()` BlueprintCallable 方法，宿主项目对象池回收时调用，清理运行时状态而不依赖 EndPlay。
23. **CachedRepulsionSubsystem 指针缓存**：`TsnTacticalMovementComponent` 在 `BeginPlay` 时缓存 `UTsnStanceRepulsionSubsystem` 指针，`ApplyStancePenetrationGuard` 中直接使用缓存引用，避免每帧 `GetSubsystem` 查询。
24. **SetRepulsionVelocity WriteFrame 首次写入语义**：`DeferredRepulsionWriteFrame` 仅在首次写入（`bHasDeferredRepulsion == false`）时更新帧号。UE 5.7 中 `UTickableWorldSubsystem`（`FTickableGameObject`）在所有 TickGroup 执行完毕后才调度，RepulsionSubsystem 天然晚于 CMC（`TG_PrePhysics`），1 帧延迟由引擎调度顺序保证；首次写入保护是额外防御性措施，防止未来 Tick 机制变更时产生同帧覆盖。
25. **RequestSlot 攻击距离变化检测**：攻击者再次调用 `RequestSlot` 时，若新的 `AttackRange` 计算出的 Radius 与已持有槽位的 Radius 之差超过 `SameRingRadiusTolerance`，释放旧槽位并在新环上重新分配。
26. **`bUseAccelerationForPaths` 兼容性**：插件不强制设置 `bUseAccelerationForPaths`，对 `true`/`false` 两种模式均兼容。排斥力在 `CalcVelocity` 的 `Super` 之后注入，不依赖基类用哪种方式计算 Velocity。宿主项目根据移动体感需求自行选择。
27. **BT 节点采用 `bCreateNodeInstance = true` 单独实例化策略**：有运行时状态的 BT Task（`ChaseEngagementTarget`、`MoveToEngagementSlot`）在构造函数中设置 `bCreateNodeInstance = true`，使每个 `UBehaviorTreeComponent` 拥有独立的节点实例。运行时状态（缓存数据、委托绑定等）存储为类成员变量，不使用 `NodeMemory`。无运行时状态的瞬时 Task（`EnterStanceMode`、`ExitStanceMode`、`ReleaseEngagementSlot`）和 Decorator（`IsTargetStationary`）不需要设置此标志。

## Risks

- `UCrowdFollowingComponent` 在 AIController Possess 前可能为 null，`CacheComponents()` 需要在 `EnterStanceMode` 调用时补缓存（文档 V2 已处理）。
- `UNavModifierComponent` 运行时创建需要 `RegisterComponent()` 和 `SetCanEverAffectNavigation(false)` 正确配对，避免 NavMesh 意外重建。
- 每次新增文件后需先运行 UBT `-projectfiles` 刷新，再编译。
- `ExitStanceMode` → `EnterStanceMode` 快速切换时，NavModifier 延迟关闭 Timer 的竞态须由 `ActivateNavModifier` 内部 `ClearTimer` 正确处理（V2 文档已包含此逻辑，实现时须确认）。
- `TsnBTTask_MoveToEngagementSlot` 的 `OnDestroyed` / `OnEndPlay` 委托绑定必须在所有退出路径统一解绑。由于采用 `bCreateNodeInstance = true`，委托绑定在独立实例上，不存在跨 Agent 覆盖问题，但仍需在 `OnTaskFinished` 中统一清理防止悬挂引用。
- `TsnEngagementSlotSubsystem` 生命周期：作为 `UWorldSubsystem`，在关卡拆除时先于组件 `EndPlay` 被销毁。`OnOwnerReleased` 和 `ReleaseSlot` 中访问子系统时需 null-guard。
- UE 5.7 中 `bUseAccelerationForPaths` 默认为 `false`，CrowdFollowingComponent 走 `RequestDirectMove` 分支——但该路径只写入 `RequestedVelocity`，最终由下一帧 `CalcVelocity` → `ApplyRequestedMove` 消费。经分析，即使宿主项目将其设为 `true`（走 `RequestPathMove` + 加速度模式），排斥力注入点（`CalcVelocity` 的 `Super` 之后）仍然有效，因此此风险已降级为"体感差异"而非"功能失效"。

## Approval Gate

所有 spec delta 验证通过后，由人工审阅本提案，批准后方可进入实现阶段。
