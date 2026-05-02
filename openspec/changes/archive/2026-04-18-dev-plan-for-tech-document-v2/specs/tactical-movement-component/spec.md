# Spec Delta: tactical-movement-component

## ADDED Requirements

### Requirement: 系统 SHALL 新增 UTsnTacticalMovementComponent 合法排斥力注入

系统 SHALL 提供 `UTsnTacticalMovementComponent`，继承 `UCharacterMovementComponent`，位于 `Public/Components/TsnTacticalMovementComponent.h` + `Private/Components/TsnTacticalMovementComponent.cpp`。

核心能力：通过 `DeferredRepulsionVelocity` 缓冲区在 `CalcVelocity`（`Super` 之后）统一叠加外部排斥力，避免与路径跟随系统产生 Velocity 写入冲突。排斥力遵循**固定 1 帧延迟消费**契约：帧 N 写入，帧 N+1 的首次 `CalcVelocity` 才消费。插件不强制设置 `bUseAccelerationForPaths`，两种模式均兼容（机制详见 V2 文档 §5.4）。

`SetRepulsionVelocity` 采用**累加语义**（`+=`），允许多个系统在同一帧内安全写入。同时添加 `ensureMsgf` 调试检测，仅在**不同系统**同帧调用时触发警告（`TsnStanceRepulsionSubsystem` 内部已合并后只调用一次，不会触发）。累加语义为未来扩展预留安全余量。

`DeferredRepulsionWriteFrame` 采用**首次写入语义**：仅在 `bHasDeferredRepulsion == false` 时记录帧号。UE 5.7 中 `UTickableWorldSubsystem`（`FTickableGameObject`）在所有 TickGroup 执行完毕后才调度，RepulsionSubsystem 天然晚于 CMC（`TG_PrePhysics`），1 帧延迟由引擎调度顺序保证；首次写入保护是额外防御性措施，防止未来 Tick 机制变更时产生同帧覆盖。

构造函数中**SHALL** 自动设置 `bUseRVOAvoidance = false`。与 DetourCrowd 同时启用会引发数据污染、双重注册、Lock 干扰三类冲突（原因详见 V2 文档 §5.2），宿主项目无需额外配置。

`BeginPlay` 时缓存 `CachedRepulsionSubsystem`（`UTsnStanceRepulsionSubsystem*`）指针，并向 Subsystem 注册为战术移动单位；`EndPlay` 时注销并清空缓存。两者均需 null-guard（关卡拆除时 Subsystem 可能已先于组件销毁）。Subsystem 通过此注册列表遍历移动单位，避免遍历场景中所有 Character。`ApplyStancePenetrationGuard` 通过 `CachedRepulsionSubsystem->GetStanceUnits()` 查询 Stance 单位列表，避免每帧 `GetSubsystem` 查询开销。

`MaxRepulsionVelocityRatio` 是所有排斥力共享的**最终速度限幅**。像外层站桩排斥力的"10%"这类规则，属于 `stance-repulsion-subsystem` 的上游力度计算系数，不替代本组件的统一限幅职责。

**关联能力**：依赖本组件的 `SetRepulsionVelocity` 接口：`stance-repulsion-subsystem`、`engagement-slot-subsystem`（间接，通过 `stance-repulsion-subsystem` 的 `GetStanceUnits()`）

#### Scenario: CalcVelocity 路径下排斥力正确叠加

**Given** 上一帧 `SetRepulsionVelocity(FVector(100, 0, 0))` 已被调用  
**When** 当前帧 `PerformMovement` 触发 `CalcVelocity`  
**Then**  
  1. `Super::CalcVelocity` 先执行（路径跟随速度 + `AddMovementInput` + CrowdVelocity via `ApplyRequestedMove` 已写入 Velocity）  
  2. `Velocity += GetClampedRepulsion()` 叠加排斥力  
  3. 叠加后若速度超过 `MaxWalkSpeed * OverspeedToleranceRatio`，进行限幅  
  4. `ApplyStancePenetrationGuard()` 裁剪向内速度分量（通过 `CachedRepulsionSubsystem->GetStanceUnits()` 查询）  
  5. `DeferredRepulsionVelocity` 清零，`bHasDeferredRepulsion = false`

#### Scenario: 无排斥力时行为与基类完全一致

**Given** `bHasDeferredRepulsion == false` 或 `DeferredRepulsionVelocity.IsNearlyZero()`，或 `DeferredRepulsionWriteFrame == CurrentFrame`  
**When** `CalcVelocity` 执行  
**Then** 直接调用 Super，不做任何额外操作（最小化性能开销）

#### Scenario: 排斥力限幅防止完全压过寻路意图

**Given** MaxWalkSpeed = 600，MaxRepulsionVelocityRatio = 0.4  
  且外部设置排斥力为 FVector(500, 0, 0)  
**When** `GetClampedRepulsion()` 计算  
**Then** 返回值被限幅至 `600 * 0.4 = 240`（而非 500），路径跟随主导权保留

#### Scenario: SetRepulsionVelocity 采用累加语义

**Given** 帧 N 内先调用 `SetRepulsionVelocity(FVector(100, 0, 0))`  
**When** 同帧再次调用 `SetRepulsionVelocity(FVector(0, 50, 0))`（例如未来新增的技能击退系统）  
**Then**  
  1. `DeferredRepulsionVelocity` 累加为 `FVector(100, 50, 0)`  
  2. `ensureMsgf` 触发调试警告（提示有不同系统同帧写入）  
  3. 不覆盖先前写入的值

> 注：`TsnStanceRepulsionSubsystem` 内部已先合并所有 Stance 单位的排斥力再调用一次，因此单一来源不会触发此警告。

#### Scenario: 构造函数自动关闭 RVO Avoidance

**Given** 宿主项目创建了使用 `UTsnTacticalMovementComponent` 的 Character  
**When** 组件构造完成  
**Then** `bUseRVOAvoidance == false`，无需宿主项目额外配置

#### Scenario: BeginPlay 注册到 RepulsionSubsystem 的移动单位列表

**Given** 场景中存在 `TsnStanceRepulsionSubsystem`  
**When** `UTsnTacticalMovementComponent::BeginPlay` 执行  
**Then** 该组件所属 Character 被注册到 Subsystem 的 `MovingUnits` 列表，Tick 遍历时可发现该单位。若 Subsystem 不存在（Editor Preview 等场景），跳过注册而非崩溃。

#### Scenario: OverspeedToleranceRatio 允许轻微超速

**Given** MaxWalkSpeed = 600，OverspeedToleranceRatio = 1.1，排斥力叠加后速度 = 680  
**When** `CalcVelocity` 中执行限幅  
**Then** 速度被裁剪至 660（600 × 1.1）而非 600，保证排斥力在接近限速时仍有可见效果。宿主项目可调低至 1.0 优先动画匹配（代价是排斥力在高速时可能无感）

#### Scenario: 调试可视化（仅 Debug/Development）

**Given** `bDrawDebugRepulsion == true` 且构建配置包含 `ENABLE_DRAW_DEBUG`  
**When** `CalcVelocity` 消费排斥力  
**Then** 在单位位置绘制排斥力方向箭头，颜色区分内层（红）和外层（黄）

#### Scenario: Stance Penetration Guard 裁剪向内速度分量

**Given** 移动单位 A 与 Stance 单位 C 的距离 = 45 cm（< C 的 ObstacleRadius = 50 cm）  
  最终速度（路径跟随 + 排斥力合成后）方向部分朝向 C  
**When** `ApplyStancePenetrationGuard()` 在 CalcVelocity 末尾执行  
**Then**  
  1. 计算 `TowardStance2D = (C.Location - A.Location).GetSafeNormal2D()`  
  2. `InwardComponent = Dot(Velocity2D, TowardStance2D)` > 0  
  3. `Velocity -= TowardStance2D * InwardComponent`（裁剪向内分量）  
  4. 裁剪后 `Dot(Velocity2D, TowardStance2D) <= 0`——硬保证无向内穿透  
  5. 输出 `UE_LOG(LogTireflySquadNav, Verbose, ...)` 记录本次裁剪（属于正常运行路径，Verbose 级别）

#### Scenario: Penetration Guard 不影响远离 Stance 的速度

**Given** 移动单位 A 距 Stance 单位 C = 40 cm（< ObstacleRadius）  
  但最终速度方向远离 C（`Dot(Velocity2D, TowardStance2D) < 0`）  
**When** `ApplyStancePenetrationGuard()` 执行  
**Then** 速度不被修改（只裁剪向内分量，不干扰向外移动）

#### Scenario: 超出 ObstacleRadius 时 Penetration Guard 不激活

**Given** 移动单位 A 距所有 Stance 单位 > ObstacleRadius  
**When** `ApplyStancePenetrationGuard()` 执行  
**Then** 速度不被修改（Guard 仅在 ObstacleRadius 范围内激活，外层由排斥力处理）

#### Scenario: OnOwnerReleased 对象池回收清理

**Given** 宿主项目使用对象池管理单位，回收时调用 `OnOwnerReleased()`  
**When** `OnOwnerReleased()` 执行  
**Then**  
  1. `DeferredRepulsionVelocity` 清零，`bHasDeferredRepulsion = false`  
  2. 通过 `CachedRepulsionSubsystem` 注销 Subsystem 注册（**需 null-guard**）  
  3. `CachedRepulsionSubsystem` 清空为 `nullptr`  
  4. 下次重新激活时通过 `BeginPlay` 重新缓存和注册

---

## 文件影响

| 文件 | 操作 |
|------|------|
| `Public/Components/TsnTacticalMovementComponent.h` | ADDED |
| `Private/Components/TsnTacticalMovementComponent.cpp` | ADDED |
