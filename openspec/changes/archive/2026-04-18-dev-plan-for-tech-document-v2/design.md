# Design: make-dev-plan-for-tech-plan-document-v2

## 架构决策记录

### 1. 跨系统依赖方向

```
ITsnTacticalUnit（接口）
        ↑ Implements
    宿主 Actor（外部）
    
TsnStanceObstacleComponent
    ├── 依赖：UCrowdFollowingComponent（AIModule）
    ├── 依赖：UNavModifierComponent（NavigationSystem）
    └── 依赖：TsnStanceRepulsionSubsystem（注册/注销）

TsnStanceRepulsionSubsystem
    ├── 依赖：TsnStanceObstacleComponent（读 MobilityStance）
    └── 依赖：TsnTacticalMovementComponent（SetRepulsionVelocity）

TsnUnitSeparationComponent
    └── 依赖：TsnStanceObstacleComponent（读 MobilityStance）

TsnBTTask_MoveToEngagementSlot
    ├── 依赖：TsnEngagementSlotComponent（RequestSlot / ReleaseSlot）
    └── 依赖：ITsnTacticalUnit（GetEngagementRange）

TsnBTTask_ChaseEngagementTarget
    └── 依赖：ITsnTacticalUnit（GetEngagementRange）

TsnBTTask_EnterStanceMode / ExitStanceMode
    └── 依赖：TsnStanceObstacleComponent

TsnBTTask_ReleaseEngagementSlot
    └── 依赖：TsnEngagementSlotComponent
```

**规则**：组件层之间不直接互相持有引用；通过世界子系统（RepulsionSubsystem）或事件驱动的接口调用解耦。

---

### 2. 排斥力注入路径（核心权衡）

**问题**：`CharacterMovementComponent` 的 `Velocity` 在 `CalcVelocity` 中每帧重算，直接写入会被覆盖。

**UE 5.7 引擎事实（详细机制见 V2 文档 §5.4 / §8）**：  
`RequestDirectMove` / `RequestPathMove` 只是"输入端"，`CalcVelocity` 才是"消费端"，且 `CalcVelocity` 每帧必然运行。因此排斥力只需在 `CalcVelocity`（`Super` 之后）统一注入一次，不再重写 `RequestDirectMove`。

**方案**：在 CMC 子类中设立独立的 `DeferredRepulsionVelocity` 缓冲区，并记录 `DeferredRepulsionWriteFrame`。`TsnStanceRepulsionSubsystem` 在帧 N 写入，MovementComponent 在帧 N+1 的 `CalcVelocity`（`Super` 之后）统一消费并清零。

```
外部系统调用                        CMC 内部
帧 N: SetRepulsionVelocity(V1) → DeferredRepulsionVelocity += V1
                                 if (!bHasDeferredRepulsion):
                                     bHasDeferredRepulsion = true
                                     DeferredRepulsionWriteFrame = N
                                 （若同帧再次调用：仅累加 += V2，ensure 警告）
                                      ↓
帧 N+1: CalcVelocity()
        Super::CalcVelocity()     ← 路径跟随 + AddMovementInput + CrowdVelocity
        若 WriteFrame < CurrentFrame：
            Velocity += Clamp(DeferredRepulsionVelocity, MaxRatio * MaxWalkSpeed)
            DeferredRepulsionVelocity = 0（清零）
        ApplyStancePenetrationGuard()  ← 硬约束裁剪
```

**关键约束**：
- `SetRepulsionVelocity` 采用**累加语义**（`+=`），允许多个系统在同一帧内安全写入。
- `DeferredRepulsionWriteFrame` 仅在**首次写入**（`bHasDeferredRepulsion == false`）时更新帧号。UE 5.7 中 `UTickableWorldSubsystem`（`FTickableGameObject`）在所有 TickGroup 执行完毕后才调度，因此 RepulsionSubsystem 天然晚于 CMC（`TG_PrePhysics`），1 帧延迟由引擎调度顺序保证。首次写入保护是额外防御性措施，防止未来 Tick 机制变更时产生同帧覆盖。

**重要：`TsnStanceRepulsionSubsystem` 必须在内部先合并所有 Stance 单位的排斥力，再对每个 Moving 单位只调用一次 `SetRepulsionVelocity`**。这样 `ensureMsgf` 的语义才正确：它仅在**不同系统**（例如未来的技能击退系统）在同一帧内调用时触发警告，而不是 SubSystem 内部遍历多个 Stance 单位时误触发。

**CachedRepulsionSubsystem 缓存**：`TsnTacticalMovementComponent` 在 `BeginPlay` 时缓存 `UTsnStanceRepulsionSubsystem` 指针，`ApplyStancePenetrationGuard` 和 `EndPlay` 中直接使用缓存引用，避免每帧 `GetSubsystem` 查询。

累加语义为未来扩展（如技能击退）预留安全余量，无额外运行时开销。

#### 2.1 `bUseAccelerationForPaths` 兼容性

插件**不强制设置** `bUseAccelerationForPaths`，对 `true` / `false` 两种模式均兼容：

| | `false`（UE 5.7 默认） | `true` |
|---|---|---|
| CrowdFollowing 调用 | `RequestDirectMove` → 写 `RequestedVelocity` | `RequestPathMove` → 调 `AddInputVector` |
| CalcVelocity 每帧运行 | **是** | **是** |
| 速度计算方式 | `ApplyRequestedMove` 直接设 Velocity | 通过加速度逐步趋近目标速度 |
| 排斥力注入点（Super 之后） | **有效** | **有效** |

**核心论证**：排斥力在 `CalcVelocity` 的 `Super` **之后**直接修改 `Velocity`，不依赖基类用哪种方式算出这个 Velocity。无论是直接设值（`false`）还是加速度积分（`true`），`Super` 返回后 `Velocity` 已经是确定值，在上面叠加即可。

**体感差异**：`true` 模式下基础寻路有加速度平滑，而排斥力注入是帧级瞬时的（`Velocity +=`），排斥响应会比基础移动"更硬"。对防穿模场景，即时响应优先于平滑过渡，这是期望行为。

**设计决策**：插件不应强制设置此值（不像 `bUseRVOAvoidance` 那样必须关闭），留给宿主项目决定移动体感。

---

### 3. 站桩状态的双层防线权衡

| 方案 | 优点 | 缺点 |
|------|------|------|
| 仅 NavModifier（Impassable） | 路径规划层完全绕开，运行时无开销 | NavMesh 更新有延迟（`NavModifierDeactivationDelay`），切换频繁时可能频繁 Tile 重建 |
| 仅排斥力 | 运行时动态，响应即时 | NavMesh 不知道站桩单位，寻路可能穿越 |
| **双层防线（选用）** | 路径规划层绕开 + 运行时兜底，两个失效场景互补 | 实现复杂度稍高，外层排斥力需要严格限幅避免干扰寻路 |

**外层排斥力上限 10%** 是关键约束——这里的 **10% 指 `RepulsionStrength` 的 10%**，
即外层阶段的力度计算为：

```cpp
ForceMagnitude = RepulsionStrength * OuterRepulsionRatio * OuterFactor;
// OuterRepulsionRatio 是 TsnStanceRepulsionSubsystem 的 UPROPERTY，默认 0.1f
```

`OuterRepulsionRatio` 现为可配置的 `UPROPERTY(EditAnywhere, ...)`，宿主项目可在编辑器中调整外层/内层力度比例。

这是一条**子系统侧的阶段系数**，用于表达“外层只做轻推”，**不是** `MaxWalkSpeed` 的 10%，
也**不是**额外引入第二个 MovementComponent 限幅参数。

最终写入 `TsnTacticalMovementComponent` 后，内层/外层排斥力仍然**统一复用**
`MaxRepulsionVelocityRatio` 做最终速度限幅：

```cpp
FinalInjectedVelocity = Clamp(RepulsionVelocity, MaxWalkSpeed * MaxRepulsionVelocityRatio);
```

因此，两层约束的职责划分如下：

- `RepulsionStrength * 0.1f * OuterFactor`：控制**外层阶段**本身的轻推强度
- `MaxRepulsionVelocityRatio`：控制**所有排斥力注入到 MovementComponent 后**的最终速度上限

---

#### 3.1 `UnitSeparation` 与 `StanceRepulsion` 的职责边界

`UTsnUnitSeparationComponent` 与 `UTsnStanceRepulsionSubsystem` 不采用“自然竞争”的关系，而是采用**分层职责**：

- `UTsnStanceRepulsionSubsystem`：`Moving↔Stance` 的**唯一权威来源**，负责 keep-out / anti-penetration
- `UTsnUnitSeparationComponent`：`Moving↔Moving` 的**软协调输入**，只负责局部 spacing

因此，`UnitSeparation` 的 Overlap 结果中若遇到 `Stance` 单位：

1. **不得**把该单位当作 separation 邻居直接产力
2. 可以把它当作一个**约束方向**使用
3. 在最终输出 `AddMovementInput` 之前，必须裁剪任何“朝站桩单位内侧”的分离分量，只保留切向/外向分量

这条规则的目的不是让 `UnitSeparation` 替代 `StanceRepulsion`，而是防止软协调输入在当前帧把单位继续推向 keep-out 区域。

**Stance 裁剪的自洽性**：`UnitSeparation` 需要感知 Stance 状态，根源与 §23 Penetration Guard 存在的原因是同一套逻辑的两面：
- 排斥力被刻意设计为柔性（70/30 切向混合 + 40% 限幅）→ 径向推力天然不足 → Penetration Guard 作为硬约束补偿是必要的（§23）
- Penetration Guard 必然存在 → `UnitSeparation` 必须提前裁剪朝 Stance 方向的分量，否则产生"注入 Super → Super 处理 → 最终被 Guard 裁掉"的无效输入

两者共同构成一个自洽闭环：软通道主动过滤无意义分量，硬通道兜底消除残余。

**可选性**：`UTsnUnitSeparationComponent` 是**完全可选**的——若宿主项目认为 DetourCrowd 的群体避障已能提供足够的 Moving↔Moving 间距，可以不挂载此组件，其余三层（NavModifier、RepulsionSubsystem、Penetration Guard）零影响。

#### 3.2 为什么现在不引入全局力合并器

当前系统只有两类语义明确的力：

- 一个 `Moving↔Stance` 的硬约束通道（`SetRepulsionVelocity`）
- 一个 `Moving↔Moving` 的软协调通道（`AddMovementInput`）

在这种规模下，直接引入通用“全局力合并器”会增加接口、状态缓存与调试复杂度，但无法带来足够收益；用“职责拆分 + 朝内分量裁剪”即可得到确定行为。

若后续又加入更多同层 steering 源，例如编队保持、脚本推挤、技能击退、局部战术偏移，并且需要统一优先级、可视化和可解释仲裁，再将它们收敛到 `UTsnTacticalMovementComponent` 内的统一收集/解析流程，会比继续叠加局部规则更合适。

推荐扩展方向是：在 MovementComponent 内引入统一的 steering contribution 结构与优先级枚举（例如 `HardKeepOut` → `NavigationAssist` → `SoftCoordination`），让各系统先提交贡献项，再由 MovementComponent 按固定顺序解析最终移动结果。

---

### 4. RVO Avoidance 自动关闭策略

**问题**：DetourCrowd 启用时，CMC 内置 RVO Avoidance 必须关闭，否则会产生双重避障冲突。仅在 `TsnStanceObstacleComponent::BeginPlay` 中发 Warning 不够安全——宿主项目可能忘记手动关闭。

**方案**：在 `UTsnTacticalMovementComponent` 构造函数中直接 `bUseRVOAvoidance = false`，从根源消除冲突。宿主项目只要使用了本组件，就不需要额外配置。`TsnStanceObstacleComponent` 的 BeginPlay 中仍保留检测作为二次确认，但不再是唯一防线。

---

### 5. EnterStanceMode 必须取消挂起的 NavModifier 延迟关闭 Timer

**问题**：`ExitStanceMode()` 设置延迟关闭 NavModifier 的 Timer → 延迟期间又调用 `EnterStanceMode()` → 幂等检查通过（`MobilityStance` 已被 Exit 设为 Moving）→ NavModifier 被重新激活 → 但挂起的 Timer 到期后会把刚激活的 NavModifier 关掉。

**方案**：`EnterStanceMode` 在执行 `ActivateNavModifier` 之前，必须先 `ClearTimer(NavModifierDeactivationTimer)`。注意 V2 文档中 `ActivateNavModifier` 内部已包含 ClearTimer 调用，因此只需确保 Enter 路径一定经过 `ActivateNavModifier` 即可——确认现有实现已正确处理此竞态。

---

### 6. RepulsionSubsystem 遍历范围收窄

**问题**：当前 `Tick` 使用 `TActorIterator<ACharacter>` 遍历场景中所有 Character，包括非战斗 NPC、玩家角色等无关 Actor。

**方案**：只遍历拥有 `TsnTacticalMovementComponent` 的单位。在 `TsnTacticalMovementComponent::BeginPlay/EndPlay` 中向 Subsystem 注册/注销，Subsystem 维护 `MovingUnits` 列表，将遍历范围收窄为 O(战术单位数)。十几到几十单位规模下性能差异不大，但能防止与非战斗 Actor 的误交互。

---

### 7. UnitSeparation 多 Stance 单位约束裁剪策略

**问题**：当周围有多个 Stance 单位时，逐个投影裁剪可能导致裁剪结果取决于遍历顺序，且多次裁剪可能将 SeparationVector 压到接近零或方向扭曲。

**方案**：**累加所有 InwardToStance 方向后做一次裁剪**。具体做法：
1. 先收集所有 Stance 邻居的 InwardToStance 方向向量
2. 对这些方向做归一化累加，得到一个综合 InwardDirection
3. 对最终 SeparationVector 做一次投影裁剪，移除朝综合 InwardDirection 的分量

这保证裁剪结果与遍历顺序无关，且只做一次投影操作，避免多次裁剪造成的过度衰减。

---

### 8. BT 节点编辑器分类与 API 宏

**BT 节点分类**：所有插件 BT 节点的构造函数中设置 `NodeName` 以 `"TSN "` 为前缀（已在 V2 文档中实现），使其在行为树编辑器菜单中成组出现，方便设计师查找。

**API 宏**：所有 Public/ 头文件中的 `UCLASS`、`USTRUCT`、`UENUM` 声明均使用 `TIREFLYSQUADNAVIGATION_API` 模块导出宏（由 UBT 在 `TireflySquadNavigationModule.h` 中自动生成）。这是 UE 模块系统的标准要求，确保宿主项目能正确链接插件符号。

---

### 9. OnDestroyed 委托统一清理

`TsnBTTask_MoveToEngagementSlot` 绑定目标的 `OnDestroyed` 委托以实现目标销毁时立即 Failed。为防止委托泄漏，所有退出路径（Succeeded/Failed/Aborted）统一在 `OnTaskFinished` 中解绑，而非在每个退出分支分别处理。

---

### 10. 槽位分配在连续空间而非离散格子

**问题**：固定角度的离散格子在攻击者数目少时会出现扭曲路径（明明可以直达，却分配到对面的格子）。

**方案**：以攻击者当前相对目标的**方位角**为理想角，在该角度附近线性搜索满足最小间距约束的空位。

```
IdealAngle = atan2(Requester.Y - Target.Y, Requester.X - Target.X)
             （攻击者相对目标的角度，即"就近原则"）

搜索：Offset = 0, ±SearchStep, ±2*SearchStep, ...
     取第一个与同环所有占用角度间距 > MinAngularGap 的候选值
```

**好处**：路径自然，不强迫绕远路；环被占满才退化回理想角度（不阻塞攻击）。

**默认参数值**（需在 Spec 中明确声明）：`MaxSlots = 12`、`SlotRadiusOffset = -20`、`MinSlotSpacing = 80`、`SameRingRadiusTolerance = 60`。

---

#### 10.1 槽位身份存本地极坐标，MoveTo 追世界快照

`UTsnEngagementSlotComponent` 保存的槽位身份应是挂在目标上的**本地极坐标**（`AngleDeg + Radius`），
而不是持久缓存的世界坐标。`CalculateWorldPosition()` 的职责只是把这组本地参数换算成
**目标当前位置下的世界空间快照**。

因此，`UTsnBTTask_MoveToEngagementSlot` 在目标移动时不应继续追逐旧世界点，而应周期性调用
`RequestSlotAndMove()` 刷新当前快照并重发 MoveTo。为控制高速目标下的漂移，建议默认值收紧为：

- `RePathCheckInterval = 0.1s`
- `RePathDistanceThreshold = 50cm`

这组默认值对应 `600cm/s` 目标时，单次快照的典型漂移约为 `60cm`，明显优于更松的 `0.3s / 100cm`。

---

#### 10.2 `ObstacleRadius` 与 `RepulsionRadius` 的不变量

双层防线成立的前提是：`RepulsionRadius > ObstacleRadius`。否则外层区间不存在，系统会退化为单层。

这里不强制固定比例，但要求两层约束同时存在：

- **硬不变量**：`RepulsionRadius > ObstacleRadius`
- **最小外层带**：非法配置时，运行时自动修正为 `RepulsionRadius >= ObstacleRadius + 30cm`

经验调参上，`RepulsionRadius` 推荐落在 `ObstacleRadius × 1.5~2.5`。实现层应由
`UTsnStanceObstacleComponent` 在注册到 `UTsnStanceRepulsionSubsystem` 前统一做 sanitize，
并通过 `ensure` / warning 暴露非法配置，而不是静默退化。

---

### 11. 三阶段行为模型与 BT Task 划分

将三阶段（远追 / 预占位接近 / 站桩攻击）映射到独立的 BT Task 是刻意的设计：

- **可替换性**：宿主项目可以替换其中任意一个 Task，或用 StateTree 重新实现，接口保持一致。
- **职责清晰**：Chase 不做槽位操作；MoveToSlot 只在预战斗距离内申请槽位；EnterStance 只切换状态。
- **容错性**：MoveToSlot 的三个退出条件（到达/中途进攻/超时）保证永远不会无限等待。

---

### 12. NavModifier 开关式 vs 创建/销毁式

单位频繁在 Moving/Stance 间切换时，创建/销毁 `UNavModifierComponent` 可能造成频繁 GC 和 NavMesh Tile 重建。

**选用开关式**：`BeginPlay` 预创建，`SetCanEverAffectNavigation(true/false)` 开关。退出时延迟 `NavModifierDeactivationDelay`（默认 0.3s）关闭，避免其他单位路径突然跳变。

---

### 13. 模块依赖增量策略

当前 Build.cs 只有运行时基础依赖。按以下规则增量添加：

**PublicDependencyModuleNames（新增）**：
- `NavigationSystem`：`UTsnNavArea_StanceUnit` 是公共头文件，继承自 `UNavArea`；宿主项目 `#include` 时依赖此模块的传递可见性，必须放 Public。

**PrivateDependencyModuleNames（新增）**：
- `AIModule`：`UCrowdFollowingComponent`、`AAIController`、BT Task/Decorator 基类
- `GameplayTasks`：`UBTTaskNode` 异步执行基础

---

## 文件布局（目标状态）

```
Plugins/TireflySquadNavigation/Source/TireflySquadNavigation/
├── TireflySquadNavigation.Build.cs          ← 修改（添加模块依赖）
├── Public/
│   ├── Interfaces/
│   │   └── ITsnTacticalUnit.h               ← 新建
│   ├── Components/
│   │   ├── TsnTacticalMovementComponent.h   ← 新建
│   │   ├── TsnEngagementSlotComponent.h     ← 新建
│   │   ├── TsnStanceObstacleComponent.h     ← 新建
│   │   └── TsnUnitSeparationComponent.h     ← 新建
│   ├── Subsystems/
│   │   ├── TsnStanceRepulsionSubsystem.h    ← 新建
│   │   └── TsnEngagementSlotSubsystem.h     ← 新建
│   ├── BehaviorTree/
│   │   ├── TsnBTTask_ChaseEngagementTarget.h     ← 新建
│   │   ├── TsnBTTask_MoveToEngagementSlot.h      ← 新建
│   │   ├── TsnBTTask_EnterStanceMode.h           ← 新建（仅头文件）
│   │   ├── TsnBTTask_ExitStanceMode.h            ← 新建（仅头文件）
│   │   ├── TsnBTTask_ReleaseEngagementSlot.h     ← 新建（仅头文件）
│   │   └── TsnBTDecorator_IsTargetStationary.h   ← 新建
│   └── NavAreas/
│       └── TsnNavArea_StanceUnit.h          ← 新建（仅头文件）
└── Private/
    ├── Components/
    │   ├── TsnTacticalMovementComponent.cpp  ← 新建
    │   ├── TsnEngagementSlotComponent.cpp    ← 新建
    │   ├── TsnStanceObstacleComponent.cpp    ← 新建
    │   └── TsnUnitSeparationComponent.cpp    ← 新建
    ├── Subsystems/
    │   ├── TsnStanceRepulsionSubsystem.cpp   ← 新建
    │   └── TsnEngagementSlotSubsystem.cpp    ← 新建
    └── BehaviorTree/
        ├── TsnBTTask_ChaseEngagementTarget.cpp  ← 新建
        ├── TsnBTTask_MoveToEngagementSlot.cpp   ← 新建
        └── TsnBTDecorator_IsTargetStationary.cpp ← 新建
```

**共计新增文件数**：19 个头文件 + 11 个实现文件 = 30 个文件（+1 个修改的 Build.cs）

> 包含 `Public/TsnLog.h` + `Private/TsnLog.cpp` 统一日志分类文件

---

### 14. BT 节点不使用 FallbackEngagementRange，接口非法值直接 Failed

**问题**：如果为 BT Task 提供 `FallbackEngagementRange` 兜底值，宿主项目可能永远不发现 `ITsnTacticalUnit` 接口配置错误，导致单位使用错误的攻击距离运行。

**方案**：当 Pawn 未实现 `ITsnTacticalUnit` 接口，或 `GetEngagementRange()` 返回 ≤ 0 时，BT Task（Chase / MoveToSlot）直接返回 `EBTNodeResult::Failed`，同时输出 `UE_LOG(LogTireflySquadNav, Warning, ...)` 提示开发者。BT 树结构的失败分支负责恢复逻辑。

**好处**：强制要求宿主项目正确实现接口，Fail-fast 比 Silent-fallback 更安全。

---

### 15. UpdateStanceUnitParams 运行时参数更新

**问题**：`FStanceObstacle` 结构体中的半径和强度参数在 `RegisterStanceUnit` 时以快照方式拷贝。若宿主项目在运行时修改 `ObstacleRadius` / `RepulsionRadius` / `RepulsionStrength`（如 buff/debuff 改变体型），注册时的值会过期。

**方案**：在 `UTsnStanceObstacleComponent` 上新增 `UpdateStanceUnitParams()` 公共方法，宿主项目修改属性后调用此方法，组件内部：
1. 对新的半径做 `GetSanitizedRadii` 合法化
2. 若处于 Stance 状态，更新 NavModifier 的 FailsafeExtent
3. 调用 `TsnStanceRepulsionSubsystem::UpdateStanceUnit(...)` 刷新缓存参数

在 `TsnStanceRepulsionSubsystem` 上新增 `UpdateStanceUnit(AActor*, float NewRepulsionRadius, float NewRepulsionStrength, float NewNavModifierRadius)` 方法，直接修改已注册条目的数值。

---

### 16. 对象池回收场景支持

**问题**：如果目标被对象池回收（`SetActorHiddenInGame(true)` + 禁用碰撞）而不是真正 `Destroy`，`OnDestroyed` 委托不会触发，`TsnBTTask_MoveToEngagementSlot` 可能继续追踪一个已失效的目标。

**方案**：`TsnBTTask_MoveToEngagementSlot` 除绑定 `OnDestroyed` 外，额外绑定 `OnEndPlay`。对象池回收时通常会触发 EndPlay（reason = `RemovedFromLevel` 或自定义），可以覆盖更多失效场景。`OnTaskFinished` 中统一解绑两个委托。

---

### 17. 调试可视化策略

> 归档说明（2026-05）：本节记录的是 V2 阶段的 raw debug 设计假设。当前插件已移除 `bDrawDebugRepulsion`、`bDrawDebugSeparation`、`tsn.debug.DrawRepulsion` 这类分散的运行时调试入口，统一收敛到 `UTsnDebugDrawSubsystem` 与 `tsn.debug.DrawUnitOverlay*` 控制面。

当前只有 `TsnEngagementSlotComponent` 包含 `#if ENABLE_DRAW_DEBUG` 绘制。以下系统需要补充调试绘制支持：

| 系统 | 绘制内容 |
|------|---------|
| `TsnStanceRepulsionSubsystem` | 内层/外层区域圆环 + 排斥力方向箭头 |
| `TsnTacticalMovementComponent` | 当前帧消费的排斥力向量（箭头） |
| `TsnUnitSeparationComponent` | 分离力方向 + Stance 约束裁剪后的最终方向 |

所有调试绘制均用 `#if ENABLE_DRAW_DEBUG` 包裹，通过 `bool bDrawDebug*` UPROPERTY 控制开关，仅在 Development/Debug 配置下可见。

**`TsnStanceRepulsionSubsystem` 额外提供控制台变量**：WorldSubsystem 没有可选中的 Actor 实例，编辑器 Details 面板中无法拨动 UPROPERTY。因此在 .cpp 中额外声明：
```
static bool GDrawDebugTsnRepulsion = false;
static FAutoConsoleVariableRef CVarDrawDebugTsnRepulsion(
    TEXT("tsn.debug.DrawRepulsion"), GDrawDebugTsnRepulsion, ...);
```
`Tick` 的调试条件为 `bDrawDebugRepulsion || GDrawDebugTsnRepulsion`——两者任意一个为 true 即启用绘制。运行时在控制台输入 `tsn.debug.DrawRepulsion 1` 即可免重启切换。组件类（`TsnTacticalMovementComponent`、`TsnUnitSeparationComponent`）挂在 Character 上，Details 面板可直接拨动，无需额外 CVar。

---

### 18. 编译验证检查点重构

原 T9-1（阶段 0~3 后编译）存在前向依赖问题：`TsnTacticalMovementComponent.cpp` 的 `BeginPlay/EndPlay` 需要 `#include "Subsystems/TsnStanceRepulsionSubsystem.h"`，但该头文件在阶段 6 才创建。

**新的检查点划分**：
- **CP-1**（阶段 0~2 后）：Build.cs + TsnLog + NavArea + Interface — 这些层完全独立，无交叉引用
- **CP-2**（阶段 3~6 后）：MovementComponent + EngagementSlot + **EngagementSlotSubsystem**（阶段 4B）+ StanceObstacle + RepulsionSubsystem — 组件与子系统相互引用，须一起编译验证；EngagementSlotSubsystem 本身无跨 TSN 依赖，可最早完成
- **CP-3**（阶段 7~8 后）：UnitSeparation + BT 节点 — 依赖前面所有层；**T7-2（UnitSeparationComponent.cpp）与 T3-2（TacticalMovementComponent.cpp）具有相同的依赖模式**：均需 `#include "Subsystems/TsnStanceRepulsionSubsystem.h"`，因此 T7-2 须等 CP-2 完成后才可实现（见 design.md §39）

---

### 19. ExitStanceMode 后排斥力间隙窗口

`ExitStanceMode` 立即注销 RepulsionSubsystem，但 NavModifier 延迟 `NavModifierDeactivationDelay`（默认 0.3s）关闭。在这个窗口内：
- NavMesh 仍显示障碍区域（路径规划仍绕开）
- 但排斥力不再生产（运行时兜底失效）

**这是设计意图**：该单位已回到 Moving 状态，从语义上不再是障碍物。NavModifier 的延迟关闭仅用于防止其他单位的路径突然跳变（平滑过渡），而非作为持续的物理屏障。在 0.3s 内若有移动单位恰好穿过该区域，DetourCrowd 的局部避障仍能提供一定程度的防穿模。

---

### 20. 槽位角度空间声明

`CalculateWorldPosition` 中的 `AngleDeg` 是**世界空间方位角**，不随目标旋转。这是刻意的设计选择：

- 理由：对于自走棋类场景，攻击者从哪个方向接近就站在哪个方向，不需要关心目标"正面/背面"
- 影响：如果目标原地转身 180°，原本在其"正面"的攻击者现在变成了"背面"，但世界空间位置不变
- 对于需要"绕后攻击"语义的宿主项目，需自行在 BT 层面调整目标选择或槽位请求逻辑

---

### 21. OverspeedToleranceRatio 的视觉影响

`OverspeedToleranceRatio = 1.1` 允许排斥力叠加后单位达到 110% 最大走速，可能造成**短暂的视觉加速感**（对于动画驱动的角色可能出现轻微滑步）。

**这是有意为之的 tradeoff**：保证排斥力在接近限速时仍有可见效果，避免被限幅完全吃掉。宿主项目可通过调低此值到 1.0 来优先动画匹配（代价是排斥力在高速时可能无感）。

---

### 22. 双通道时序分析（AddMovementInput vs SetRepulsionVelocity）

系统中存在两条独立的力注入通道，消费时机不同：

| 通道 | 使用者 | 注入方式 | 消费时机 |
|------|--------|---------|---------|
| `SetRepulsionVelocity` | `TsnStanceRepulsionSubsystem` | 写入 `DeferredRepulsionVelocity` 缓冲 | `Super::CalcVelocity()` **之后**叠加 |
| `AddMovementInput` | `TsnUnitSeparationComponent` | 累加到 `PendingInputVector` | `Super::CalcVelocity()` **内部**的 `ApplyControlInputVector` 阶段消费 |

> 注：CrowdFollowingComponent 的 `RequestDirectMove` 同样最终由 `Super::CalcVelocity()` 内部的 `ApplyRequestedMove` 消费，属于与 `AddMovementInput` 同一个 Super 内部阶段。

**时序差异的影响**：`AddMovementInput` 的输入会被 Super 内部的加速/制动逻辑处理，可能被放大或衰减；而 `SetRepulsionVelocity` 直接叠加到最终速度，不受 Super 内部逻辑影响。

**已知且可接受**：`UnitSeparation` 作为软协调层，其输入被 Super 的物理模型自然处理是合理行为——分离力本质上是一种"我想往这个方向多走一点"的建议，由物理系统决定实际效果。而 `RepulsionVelocity` 是硬约束，必须绕过物理系统的重新计算。

**残留风险**：即使 `UnitSeparation` 在注入前裁剪了朝 Stance 方向的分量，Super 内部的加速逻辑理论上可能产生微小的朝内残余分量。该风险由 §23 的 Stance Penetration Guard 兜底消除。

---

### 23. 站桩穿透防护（Stance Penetration Guard）

**第三层防线的必要性根源**：第二层排斥力的设计意图是"柔性引导绕行"而非"硬性顶回"——70% 切向混合 + 40% 速度上限，使得真正的径向推力上限仅为 `0.4 × 0.3 = 12% MaxWalkSpeed`。这组参数的柔性刻意不足，因此第三层是对这个参数选择的**必然补偿**，不是冗余防御。

**具体触发场景**：

| 场景 | 触发概率 | 是否属于上游 Bug |
|------|---------|----------------|
| A. NavMesh Tile 重建延迟（Stance 切换后 1~3 帧，路径仍经过 Stance 区域）| **高**，每次 Stance 切换都可能出现 | 否，Tile 延迟是 NavMesh 系统的固有特性 |
| B. 排斥力 1 帧延迟空窗期（帧 N Stance 注册，帧 N 的 CalcVelocity 排斥力还未注入）| 低，仅发生在切换瞬间+近距离重叠时 | 否，延迟契约的固有代价 |
| C. UnitSeparation 经 Super 加速逻辑产生的微小残余朝内分量（见 §22）| 极低，数值精度级别 | 否，CMC 物理模型限制 |
| D. 多 Stance 单位包夹，合成排斥力指向空隙而某个个体 Stance 单位仍有朝内分量 | 中，密集战场下常见 | 否，全局合力无法对每个个体单独保证 |

场景 A（高频）+ 场景 D（中频）已足以证明第三层不是过度防御。

**方案**：在 `TsnTacticalMovementComponent` 中增加第三层防线——**站桩穿透防护（Stance Penetration Guard）**。在 `CalcVelocity` 的最终速度确定后（`Super` + 排斥力叠加 + 限幅之后）执行：

1. 查询 `TsnStanceRepulsionSubsystem` 获取当前已注册的 Stance 单位列表
2. 对每个与本单位距离 ≤ `ObstacleRadius` 的 Stance 单位，检测最终速度是否存在朝向该单位的分量
3. 若 `Dot(Velocity2D, TowardStance2D) > 0`，裁剪该分量至零，**并输出 `UE_LOG(LogTireflySquadNav, Verbose, ...)` 记录本次裁剪**（不是 Warning，属于正常运行路径；但可用于调参时追踪前两层介入频率）

```
CalcVelocity 完整流程：
  Super::CalcVelocity()          ← 路径跟随 + AddMovementInput + CrowdVelocity
  += GetClampedRepulsion()       ← 排斥力叠加
  Overspeed 限幅
  ApplyStancePenetrationGuard()  ← 第三层：硬约束裁剪（附 Verbose 日志）
```

**这是一个硬约束**：无论路径跟随、分离力、排斥力如何合成，最终速度在 `ObstacleRadius` 边界处不可能有向内分量。开销为 O(Stance 单位数)，在小规模场景下可忽略。

**三层防线完整形态**：

| 层次 | 机制 | 作用时机 | 保证 |
|------|------|---------|------|
| 第一层 | NavModifier | 路径规划 | 路径不经过 Stance 区域 |
| 第二层 | RepulsionSubsystem | 运行时排斥 | 接近时柔性引导方向改变（12% MaxWalkSpeed 径向上限） |
| 第三层 | Penetration Guard | 速度后处理 | ObstacleRadius 内零向内速度（硬约束，正常触发） |

**实现依赖**：`TsnStanceRepulsionSubsystem` 需提供 `GetStanceUnits()` 公共访问器，返回已注册 Stance 单位列表（含 Actor 弱引用和 ObstacleRadius 等参数，**不含位置快照**——调用方每帧自行调用 `Obs.Unit->GetActorLocation()` 动态读取）。`TsnTacticalMovementComponent` 已持有 Subsystem 引用（用于注册），无需新增依赖。

---

### 24. 单目标槽位约束

同一攻击者同一时间只能在一个目标上持有槽位。通过 `UTsnEngagementSlotSubsystem`（继承 `UWorldSubsystem`）维护全局 `TMap<AActor* Requester, UTsnEngagementSlotComponent* SlotComp>` 映射：

- `RequestSlot` 入口查询子系统的 `FindOccupiedSlotComponent(Requester)`，若返回非 null 且不是当前组件，先自动释放旧槽位
- 分配成功后调用 `RegisterSlotOccupancy(Requester, this)` 登记
- `ReleaseSlot` 中调用 `UnregisterSlotOccupancy(Requester)` 注销

子系统本身不 Tick，仅作为无状态查询注册表。由于所有 BT 操作都在 GameThread，不需要加锁。

---

### 25. Stance 状态下不重新申请槽位

攻击者进入 Stance（站桩攻击）后，除非以下两种情况，不应重新触发 `RequestSlot`：
1. 目标死亡（由 BT 的 `OnTargetDestroyed` / 失败分支处理）
2. 自身受到强制切换目标的控制效果（如魅惑、嘲讽），由 BT 树结构的中断逻辑触发重新选敌

这是 **BT 树层面的约束**，不是组件层的约束——`TsnEngagementSlotComponent` 不拒绝 Stance 状态下的 `RequestSlot` 调用（保持 API 灵活性），而是由 BT 树保证不在 Stance 分支内重复请求。

---

### 26. ObstacleRadius 与 RepulsionRadius 编辑器约束

`TsnStanceObstacleComponent` 的半径属性需在编辑器中提供合理的默认值和范围约束：

| 属性 | 默认值 | 编辑器元数据 |
|------|-------|------------|
| `ObstacleRadius` | `60.f` | `ClampMin = "10.0"` |
| `RepulsionRadius` | `120.f` | `ClampMin = "20.0"` |
| `RepulsionStrength` | `800.f` | `ClampMin = "0.0"` |

`ObstacleRadius = 60` 约等于标准胶囊体半径，保证即使宿主未自定义也能产生合理的障碍效果。编辑器 ClampMin 防止配置为零或负值，但运行时的 `RepulsionRadius > ObstacleRadius` 不变量仍由 `GetSanitizedRadii` 保证（见 §10.2）。

---

### 27. Level Teardown 与 Subsystem 生命周期安全

**问题**：关卡拆除时，`UTickableWorldSubsystem` 可能先于 `UActorComponent::EndPlay` 被销毁。若 `TsnTacticalMovementComponent::EndPlay` 或 `TsnStanceObstacleComponent::EndPlay` 在 Subsystem 已销毁后尝试注销，会导致空指针崩溃。

**方案**：所有在 `EndPlay` 中访问 `TsnStanceRepulsionSubsystem` 的组件，必须先做 null-guard：

```cpp
if (UWorld* World = GetWorld())
{
    if (auto* Sub = World->GetSubsystem<UTsnStanceRepulsionSubsystem>())
    {
        Sub->UnregisterMovingUnit(this);  // 或 UnregisterStanceUnit
    }
}
```

同样适用于 `BeginPlay` 中的注册：若 Subsystem 因某些原因不存在（Editor Preview 等），跳过注册而非崩溃。

---

### 28. AIController Unpossess 与 CrowdFollowingComponent 引用有效性

**问题**：`TsnStanceObstacleComponent` 在 `CacheComponents` 中缓存了 `UCrowdFollowingComponent` 引用。若 AI 在战斗中被 Unpossess（控制权切换），AIController 被替换或移除，缓存的引用指向已失效的对象。

**方案**：`EnterStanceMode` / `ExitStanceMode` 在操作 Crowd 状态前，调用 `CacheComponents` 重新获取引用。`CacheComponents` 本身已设计为可重入（通过 AIController 查询 PathFollowingComponent），因此 Unpossess 后会自然获取 null，不会操作失效对象。

需确保 `CrowdFollowingComp` 为 null 时，`EnterStanceMode` / `ExitStanceMode` 跳过 Crowd 状态切换步骤（仅输出 Warning），其他步骤（NavModifier、RepulsionSubsystem）照常执行。

---

### 29. GameThread 线程安全假设

所有 `TsnEngagementSlotComponent` 的 `RequestSlot` / `ReleaseSlot` 调用假定在 GameThread 上执行。当前 UE 的 BT Task 执行均在 GameThread，因此无线程竞争风险。若未来引入异步 BT Tick 或并行任务执行，则需在 `RequestSlot` / `ReleaseSlot` 入口处加锁。`TsnEngagementSlotSubsystem` 同样依赖此假设——其 `TMap` 无锁保护。

---

### 30. 排斥力方向混合比例可配置（TangentBlendRatio）

排斥力方向混合原为硬编码 70% 切线 + 30% 径向。将此比例提升为 `TsnStanceRepulsionSubsystem` 的 `UPROPERTY`：

- `TangentBlendRatio`（默认 `0.7f`，`ClampMin = 0.0`，`ClampMax = 1.0`）
- 代码中使用 `TangentBlendRatio` 和 `(1.0f - TangentBlendRatio)` 替代硬编码 `0.7f` / `0.3f`

设计动机：不同场景对绕行行为的偏好不同——棋盘格自走棋倾向高切向（0.7~0.8），MOBA 可能倾向更直接的推开（0.4~0.5）。提供 UPROPERTY 而非硬编码，让宿主项目可以在编辑器中无代码调参。

---

### 31. 对象池回收支持（OnOwnerReleased）

对象池场景下，Actor 被回收时不一定触发 `EndPlay`（取决于对象池实现）。为此，所有持有运行时状态的组件提供 `OnOwnerReleased()` BlueprintCallable 方法：

| 组件 | OnOwnerReleased 行为 |
|------|---------------------|
| `TsnTacticalMovementComponent` | 清零排斥力缓冲、注销 Subsystem 注册、清空缓存指针 |
| `TsnStanceObstacleComponent` | 强制退出 Stance、清除 Timer、反激活 NavModifier |
| `TsnEngagementSlotComponent` | 遍历所有 Slots，通知子系统注销每个占用者的记录，清空 Slots 数组 |
| `TsnUnitSeparationComponent` | 停止 Tick |

宿主项目在对象池回收逻辑中依次调用各组件的 `OnOwnerReleased()`，等效于 EndPlay 的清理效果。

---

### 32. RequestSlot 攻击距离变化检测

攻击者可能因 buff/debuff 改变 `GetEngagementRange()` 返回值。`RequestSlot` 入口处增加半径变化检测：

```
if (已有槽位 Existing):
    NewRadius = AttackRange + SlotRadiusOffset
    if |Existing.Radius - NewRadius| > SameRingRadiusTolerance:
        ReleaseSlot(Requester)  ← 释放旧环槽位
        // 继续执行后续分配逻辑，在新环上分配
    else:
        复用已有槽位，仅刷新世界快照
```

这确保攻击距离发生显著变化时，攻击者被重新分配到正确的环上，而非继续占据旧环的不合适位置。

---

### 33. SetRepulsionVelocity WriteFrame 首次写入语义

**背景**：UE 5.7 中 `UTickableWorldSubsystem`（继承 `FTickableGameObject`）的 Tick 在所有 TickGroup（包括 CMC 所在的 `TG_PrePhysics`）执行完毕后才被调度（见 `UWorld::Tick` 中 `FTickableGameObject::TickObjects()` 的位置）。因此 `TsnStanceRepulsionSubsystem` **天然在 CMC 之后 Tick**，1 帧延迟由引擎调度顺序保证：

```
帧 N:
  1. CMC.CalcVelocity → 消费帧 N-1 的排斥力 (WriteFrame(N-1) < N ✓)
  2. Subsystem.Tick   → SetRepulsionVelocity → WriteFrame = N
帧 N+1:
  1. CMC.CalcVelocity → 消费帧 N 的排斥力 (WriteFrame(N) < N+1 ✓)
  2. Subsystem.Tick   → SetRepulsionVelocity → WriteFrame = N+1
```

**原先描述的"若 Subsystem 在同一帧的 CMC 之前 Tick"场景在当前引擎中不会发生**。

**防御性保留**：仍然采用"首次写入"保护作为额外防御措施，防止未来 Tick 机制变更（如改用自定义 FTickFunction 注册到更早的 TickGroup）时产生同帧覆盖：

```cpp
DeferredRepulsionVelocity += InRepulsionVelocity;
if (!bHasDeferredRepulsion)
{
    bHasDeferredRepulsion = true;
    DeferredRepulsionWriteFrame = GFrameCounter;
}
```

这样只有"第一笔"写入记录帧号。后续同帧累加只增量数值、不覆盖帧号。

---

### 34. BT 节点采用 `bCreateNodeInstance = true` 实例化策略

**问题**：UE 行为树系统中 `UBTTaskNode` 默认是**共享模板**——多个 BehaviorTreeComponent 共用同一个节点实例。如果将运行时状态（缓存数据、委托引用等）存储为类成员变量，不同 Agent 的执行会互相覆盖，导致不确定行为。

**可选方案对比**：

| 方案 | 优点 | 缺点 |
|------|------|------|
| NodeMemory（默认） | 内存占用最小，引擎标准模式 | 需实现 `GetInstanceMemorySize()`，所有状态需通过 `NodeMemory` 指针访问，代码可读性差；委托绑定无法自然存储 |
| `bCreateNodeInstance = true` | 类成员变量直接使用，代码直观；委托绑定天然安全（每个实例独立） | 内存占用略高（每个 BT 实例多一份节点对象） |

**选用 `bCreateNodeInstance = true`**：本插件面向小规模战斗（十几到几十 AI），额外内存开销可忽略。`MoveToEngagementSlot` 需要绑定目标的 `OnDestroyed` / `OnEndPlay` 委托，实例化模式使委托生命周期管理自然清晰。

**应用范围**：仅对有运行时状态的 BT Task 设置：
- `UTsnBTTask_ChaseEngagementTarget`：缓存 `CachedAttackRange`、`CachedPreEngagementRadius`
- `UTsnBTTask_MoveToEngagementSlot`：缓存槽位数据、超时计时、委托绑定等

**不需要设置的节点**：
- `UTsnBTTask_EnterStanceMode` / `ExitStanceMode` / `ReleaseEngagementSlot`：瞬时执行（`ExecuteTask` 立即返回），无运行时状态
- `UTsnBTDecorator_IsTargetStationary`：无运行时状态，只读查询

---

### 35. `FTsnStanceObstacle` 从私有内嵌 struct 提升为独立 USTRUCT

**问题**：原设计中 `FStanceObstacle` 声明在 `UTsnStanceRepulsionSubsystem` 的 `private:` 区段内。虽然 `GetStanceUnits()` 是公开方法，但调用方（`TsnTacticalMovementComponent`）在 `.cpp` 中无法使用该类型名称（私有嵌套类型在类外不可见），导致 `const FStanceObstacle& Obs` 等写法会编译报错。

**决策**：将 `FStanceObstacle` 改为文件作用域的独立 `USTRUCT`，命名为 `FTsnStanceObstacle`（添加插件前缀），并标注 `TIREFLYSQUADNAVIGATION_API` 导出宏。将其声明在 `UTsnStanceRepulsionSubsystem` 类定义之前，同文件的 `generated.h` 之后。

**命名变化**：`FStanceObstacle` → `FTsnStanceObstacle`（符合插件 `Tsn` 前缀规范）

**影响范围**：
- `TsnStanceRepulsionSubsystem.h`：新增 USTRUCT 定义，移除私有内嵌 struct，更新 `GetStanceUnits()` 返回类型及私有成员类型
- `TsnStanceRepulsionSubsystem.cpp`：更新 4 处 lambda 参数和循环变量类型
- `TsnTacticalMovementComponent.cpp`：更新 `ApplyStancePenetrationGuard` 中的 2 处引用

**不需要 `UPROPERTY`**：`FTsnStanceObstacle` 是注册参数的载体（Actor 弱引用 + 半径/强度数值），不参与序列化或蓝图反射，成员无需 `UPROPERTY` 标注。**注意：struct 不缓存位置**，调用方每帧通过 `Obs.Unit->GetActorLocation()` 动态读取，以避免站桩单位位移后快照过期的问题。`USTRUCT()` 仅用于满足 UHT 扫描要求并确保正确导出。

---

### 36. BT 节点 `InitializeFromAsset` 中的 Blackboard Key 双阶段验证

**背景**：`FBlackboardKeySelector` 拥有两个字段：
- `SelectedKeyName`（`FName`）：编辑器时填写的 key 名称字符串
- `SelectedKeyID`（`int32`，通过 `GetSelectedKeyID()` 转换为 `FBlackboard::FKey`）：运行时整数索引，初始为 `FBlackboard::InvalidKey`（UE 5.7 中为 `uint16(-1)` = 65535，不再是旧版的 255）

运行时 `GetBlackboardComponent()->GetValueAsObject(SelectedKeyName)` 进行字符串查找（O(n)）；用 `SelectedKeyID` 查找是 O(1) 数组访问。

**`ResolveSelectedKey` 的作用**：在 BT Asset 加载期间（`InitializeFromAsset`）调用，内部执行 `BBData.GetKeyID(SelectedKeyName)`：
- 找到匹配 key → 将整数 ID 写入 `SelectedKeyID`，后续运行时走快速路径
- 未找到（key 名拼写错误、BB Asset 未绑定、key 已被删除）→ `SelectedKeyID` 保持 `FBlackboard::InvalidKey`

**`ResolveSelectedKey` 作为有效性验证的原理**：调用之后检查 `SelectedKeyID == FBlackboard::InvalidKey`，即可判断 key 名称在目标 BB Asset 中是否真实存在。这是引擎提供的唯一可靠验证点——在 `InitializeFromAsset`（Asset 加载时）做检查，比在 `ExecuteTask` 每次执行时检查更早，可以在进入关卡前就暴露配置错误。

**双阶段验证**：
1. **第一阶段**：`GetBlackboardAsset() != nullptr`——BT 自身有绑定的 BB Asset（捕获"BT 根本没绑 BB"）
2. **第二阶段**：`ResolveSelectedKey` 后 `TargetKey.SelectedKeyID != FBlackboard::InvalidKey`——key 名称在该 BB 中确实存在（捕获"BB 绑了但 key 改名/删除"的常见配置错误）

两道验证缺一不可：仅第一道不能发现 key 名称错误；仅第二道在无 BB 时会崩溃（`ResolveSelectedKey` 需要有效的 `UBlackboardData&`）。

**应用节点**：`UTsnBTTask_ChaseEngagementTarget`、`UTsnBTTask_MoveToEngagementSlot`、`UTsnBTDecorator_IsTargetStationary`（均有 `TargetKey`）。

### 37. `bIsReleasingOwner` 使 `ExitStanceMode` 感知调用上下文

**问题**：`ExitStanceMode` 在正常游戏流程中对 NavModifier 使用**延迟关闭**（`NavModifierDeactivationDelay = 0.3s`），避免其他单位路径突然跳变。但对象池回收（`OnOwnerReleased`）要求**立即**清理，不能留下悬垂定时器——否则 Actor 被重新激活并进入 Stance 后，上一次回收的定时器一旦触发就会错误关闭 NavModifier。

**原有实现的缺陷**：`OnOwnerReleased` 先调用 `ExitStanceMode`（产生定时器），再手动 `ClearTimer` + 再 `DeactivateNavModifier`。这是"产生后立即撤销"的反模式，逻辑分散、易遗漏。

**方案**：引入 `bool bIsReleasingOwner` 私有标志，由 `OnOwnerReleased` 在调用 `ExitStanceMode` 前置为 `true`，结束后复位。`ExitStanceMode` 内部根据此标志选择路径：
- `bIsReleasingOwner == false`（正常退出）→ 延迟定时器路径（原行为不变）
- `bIsReleasingOwner == true`（池回收）→ 立即 `DeactivateNavModifier()`，不产生定时器

**`OnOwnerReleased` 简化后**：调用 `ExitStanceMode`（内部已处理立即关闭）→ 仅以 `ClearTimer` 作为兜底清理残留定时器 → 复位标志。不再需要手动补调 `DeactivateNavModifier`。

### 38. 槽满时 BT 返回 Failed 而非移动到 Fallback 位置

**问题**：原 `RequestSlot` 在槽位已满（`Slots.Num() >= MaxSlots`）时返回一个朝向请求者方向的 Fallback 世界坐标，但**不注册**请求者为槽位占用者。这导致：
1. BT Task 拿到一个"看起来有效"的位置并发起 `MoveTo`，单位实际移动但没有任何槽位注册
2. 到达 Fallback 后触发 `Succeeded`，进入 Stance 攻击，但后续逻辑依赖 `HasSlot` 判断会全部失效
3. 如果两个无槽攻击者同时拿到同一 Fallback 方向，会发生完全重叠

**方案**：在 `TsnEngagementSlotComponent` 增加 `IsSlotAvailable(Requester)` 查询方法：
- 若请求者已持有槽位 → `true`（后续 `RequestSlot` 会复用，不消耗新槽）
- 否则检查 `Slots.Num() < MaxSlots`

`RequestSlotAndMove` 在调用 `RequestSlot` 前先检查 `IsSlotAvailable`，槽满时通过 Log 记录后返回 `false`（→ BT 返回 `Failed`），让行为树的失败分支处理换目标逻辑。

**Fallback 路径保留原因**：`RequestSlot` 是 `BlueprintCallable`，Blueprint 调用者可以绕过 `IsSlotAvailable` 直接调用，Fallback 路径作为最后防线防止 Blueprint 侧崩溃。C++ 侧的 `RequestSlotAndMove` 通过 `IsSlotAvailable` 保证不会走到该路径。

### 39. `UTsnUnitSeparationComponent` 用注册列表替换物理 Overlap 查询

**原方案**：`TickComponent` 调用 `GetWorld()->OverlapMultiByObjectType` 以 `SeparationRadius` 球做物理宽相查询，返回 `TArray<FOverlapResult>`，再逐个检查 `UTsnStanceObstacleComponent` 获取 Stance 状态。

**问题**：
1. 物理宽相（Broad Phase）本身有 CPU 开销（BVH 树遍历、碰撞层过滤），即使单次耗时微小，每帧对所有移动单位（~30 个）各触发一次累积可观
2. 查询范围固定为 `SeparationRadius`（120cm），无法感知 Stance 单位的 `RepulsionRadius`（150cm）——Stance 约束裁剪半径应与 RepulsionSubsystem 保持一致，否则在 120~150cm 区间存在盲区

**新方案**：UnitSeparationComponent 在 `BeginPlay` 缓存 `UTsnStanceRepulsionSubsystem*`，在 `TickComponent` 中直接遍历两个已维护的列表：
- `GetMovingUnits()`（`TArray<TWeakObjectPtr<AActor>>`）——Moving↔Moving 分离，跳过当前 Stance 的项
- `GetStanceUnits()`（`TArray<FTsnStanceObstacle>`）——Stance 约束裁剪，使用 `SeparationRadius` 做距离门控

**优点**：
- 零物理系统调用；30 个单位的 O(n²) 直接距离查询在 Debug 配置下 < 0.01ms
- Stance 约束方向直接来自权威 StanceUnits 列表，与 RepulsionSubsystem 数据完全一致
- UnitSeparation 与 RepulsionSubsystem 共用同一注册表，不产生额外状态

**注意**：`GetMovingUnits()` 返回的是**所有战术移动单位**（按 BeginPlay/EndPlay 注册），包括当前处于 Stance 的单位，因此遍历时仍需检查对方的 `GetMobilityStance()` 来过滤 Stance 邻居——这是设计意图，避免在 RepulsionSubsystem 之外重复注册/注销逻辑。