# Tasks: make-dev-plan-for-tech-plan-document-v2

有序的工作项列表，按依赖关系排定执行顺序。每项完成后更新状态。

---

## 阶段 0：准备工作

- [x] **T0-1** 更新 `TireflySquadNavigation.Build.cs`，添加实现所需模块依赖
  - Public 依赖：`Core`（保持）+ 新增 `NavigationSystem`（NavArea 公共头文件被宿主项目引用，须 Public 传递）
  - Private 依赖：`CoreUObject`、`Engine`、`Slate`、`SlateCore`（保持）+ 新增 `AIModule`、`GameplayTasks`
  - 验证：UBT 刷新项目文件后无红色依赖错误

- [x] **T0-2** 刷新 UBT 项目文件
  - 每次新增文件批次后执行一次
  - 命令：`"E:\UnrealEngine\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="E:\Projects_Unreal\SquadNavDev\SquadNavDev.uproject" -game -rocket -progress`

- [x] **T0-3** 创建插件统一日志分类文件
  - 新建 `Public/TsnLog.h`：声明 `DECLARE_LOG_CATEGORY_EXTERN(LogTireflySquadNav, Log, All)`
  - 新建 `Private/TsnLog.cpp`：包含 `DEFINE_LOG_CATEGORY(LogTireflySquadNav)`
  - 插件内所有 .cpp 均通过 `#include "TsnLog.h"` 使用统一 log category，禁止使用 `LogTemp`
  - 验证：编译后可在 UE 输出日志中按 `LogTireflySquadNav` 过滤

---

## 阶段 1：导航区域（无依赖，最简单）

- [x] **T1-1** 创建 `Public/NavAreas/TsnNavArea_StanceUnit.h`
  - 继承 `UNavArea`，仅头文件（无 .cpp）
  - 专用于 `ETsnNavModifierMode::HighCost` 模式；Impassable 模式直接使用引擎内置 `NavArea_Null`（导航调试视图中两者颜色已自然不同：Null 呈红色，本类呈橙色）
  - `DefaultCost = 10.f`，`FixedAreaEnteringCost = 5.f`，`DrawColor = FColor(255, 128, 0, 255)`（橙色）
  - 验证：文件存在，类声明正确

---

## 阶段 2：接口层

- [x] **T2-1** 创建 `Public/Interfaces/ITsnTacticalUnit.h`
  - `UINTERFACE(MinimalAPI, Blueprintable)` + `UTsnTacticalUnit`
  - 接口函数：`GetEngagementRange()`（返回 float）、`IsInStanceMode()`（返回 bool）
  - 均为 `UFUNCTION(BlueprintNativeEvent, BlueprintCallable)`
  - 验证：头文件可被其他模块 `#include` 引用

---

## 阶段 3：移动组件

- [x] **T3-1** 创建 `Public/Components/TsnTacticalMovementComponent.h`
  - 继承 `UCharacterMovementComponent`
  - **构造函数**：`bUseRVOAvoidance = false`（从根源关闭 RVO，防止与 DetourCrowd 双重避障冲突）
  - Public API：`SetRepulsionVelocity(const FVector&)`（累加语义 `+=`，非覆盖）、`OnOwnerReleased()`（对象池回收清理）
  - 属性：`MaxRepulsionVelocityRatio`（默认 0.4）、`OverspeedToleranceRatio`（默认 1.1）
  - Protected 重写：`CalcVelocity()`、`BeginPlay()`、`EndPlay()`（不再重写 `RequestDirectMove`——UE 5.7 中它只是输入端，排斥力统一在 `CalcVelocity` Super 之后注入）
  - Private：`DeferredRepulsionVelocity`、`bHasDeferredRepulsion`、`DeferredRepulsionWriteFrame`、`GetClampedRepulsion()`、`CanConsumeDeferredRepulsion()`、`ConsumeDeferredRepulsion()`、`ApplyStancePenetrationGuard()`
  - Private 缓存：`CachedRepulsionSubsystem`（`UTsnStanceRepulsionSubsystem*`，BeginPlay 时缓存，避免每帧 GetSubsystem 查询）

- [x] **T3-2** 创建 `Private/Components/TsnTacticalMovementComponent.cpp`
  - 实现 `SetRepulsionVelocity`（累加语义 `DeferredRepulsionVelocity += InRepulsionVelocity`；**WriteFrame 首次写入语义**：仅在 `bHasDeferredRepulsion == false` 时设置 `DeferredRepulsionWriteFrame = GFrameCounter`。UE 5.7 中 RepulsionSubsystem（`FTickableGameObject`）天然晚于 CMC（`TG_PrePhysics`）Tick，1 帧延迟由引擎调度顺序保证；首次写入保护是额外防御，防止未来 Tick 机制变更时产生同帧覆盖）
  - 实现 `GetClampedRepulsion`、`CanConsumeDeferredRepulsion`、`ConsumeDeferredRepulsion`
  - 至少 1 帧延迟：帧 N 写入，帧 N+1 的 `CalcVelocity` 才允许消费
  - 实现 `CalcVelocity`：Super → 叠加上一帧排斥力 → 限幅 → `ApplyStancePenetrationGuard`
  - ~~实现 `RequestDirectMove`~~（已移除：UE 5.7 中 `RequestDirectMove` 只是写入 `RequestedVelocity`，由下一帧 `CalcVelocity` → `ApplyRequestedMove` 消费，排斥力统一在 `CalcVelocity` 注入即可）
  - 实现 `ApplyStancePenetrationGuard`：通过 `CachedRepulsionSubsystem->GetStanceUnits()` 查询 Stance 单位列表，对 ObstacleRadius 内的单位裁剪向内速度分量；**每次实际发生裁剪时输出 `UE_LOG(LogTireflySquadNav, Verbose, ...)` 记录（裁剪属于正常运行路径，用 Verbose 而非 Warning，方便调参时追踪前两层介入频率）**
  - 实现 `BeginPlay`：缓存 `CachedRepulsionSubsystem` 指针 + 向 Subsystem 注册为战术移动单位（**需 null-guard**）
  - 实现 `EndPlay`：使用 `CachedRepulsionSubsystem` 注销（**需 null-guard**），清空缓存指针
  - 实现 `OnOwnerReleased`：清零排斥力缓冲 + 注销 Subsystem 注册 + 清空 `CachedRepulsionSubsystem`
  - `#if ENABLE_DRAW_DEBUG`：可视化当前帧消费的排斥力向量（箭头）
  - **注意**：`BeginPlay/EndPlay` 需要 `#include "Subsystems/TsnStanceRepulsionSubsystem.h"`，该头文件在阶段 6 创建，因此本文件编译验证须延至 CP-2（阶段 3~6 全部完成后）

---

## 阶段 4：交战槽位组件

- [x] **T4-1** 创建 `Public/Components/TsnEngagementSlotComponent.h`
  - `FTsnEngagementSlotInfo` 结构体（Occupant、AngleDeg、Radius），其中 AngleDeg/Radius 表示目标本地极坐标
  - Public API：`RequestSlot`、`ReleaseSlot`、`HasSlot`、`IsSlotAvailable(AActor* Requester) const`、`GetOccupiedSlotCount`、`OnOwnerReleased()`（对象池回收清理）
  - 属性：`MaxSlots`、`SlotRadiusOffset`、`MinSlotSpacing`、`SameRingRadiusTolerance`、`bDrawDebugSlots`
  - `#if ENABLE_DRAW_DEBUG` 调试绘制接口（`DrawDebugSlots()`）
  - **注意**：头文件本身不 include `TsnEngagementSlotSubsystem.h`（仅在 T4-2 的 .cpp 中引用），因此 **T4-1 无 T4B-x 依赖，可在批次 A 完成**

- [x] **T4-2** 创建 `Private/Components/TsnEngagementSlotComponent.cpp`
  - 实现全部 public 方法和 private 辅助方法
  - **`RequestSlot` 子系统集成**：入口处查询 `UTsnEngagementSlotSubsystem::FindOccupiedSlotComponent(Requester)`，若返回非 null 且不是当前组件，自动释放旧槽位；分配成功后调用 `RegisterSlotOccupancy(Requester, this)`
  - **攻击距离变化检测**：已有槽位时，若新 Radius 与旧 Radius 之差超过 `SameRingRadiusTolerance`，释放旧槽位并在新环上重新分配
  - **`ReleaseSlot` 子系统集成**：释放槽位后调用 `UnregisterSlotOccupancy(Requester)`
  - **`OnOwnerReleased`**：遍历所有 Slots，通知子系统注销每个占用者的记录，清空 Slots 数组。**需双层 null-guard**：`GetWorld()` 可能为 null（组件脱离 World），`GetSubsystem<UTsnEngagementSlotSubsystem>()` 可能为 null（切关卡时 Subsystem 先于组件销毁）。切关卡时对象池对象被批量释放恰好与关卡拆除重叠，是最易触发此路径的场景。
  - `FindUnoccupiedAngle`：同环角度冲突检测，正负交替搜索
  - `CalculateWorldPosition`：目标本地角度 + 半径 → 基于目标当前位置的世界快照
  - `CleanupInvalidSlots`：清理失效弱引用
  - `DrawDebugSlots`（`#if ENABLE_DRAW_DEBUG`）
  - **注意**：需要 `#include "Subsystems/TsnEngagementSlotSubsystem.h"`，该文件在阶段 4B 创建

---

## 阶段 4B：交战槽位子系统（无 Tick，纯注册表）

- [x] **T4B-1** 创建 `Public/Subsystems/TsnEngagementSlotSubsystem.h`
  - 继承 `UWorldSubsystem`（**不是** `UTickableWorldSubsystem`，无需 Tick）
  - Public API：`RegisterSlotOccupancy(AActor* Requester, UTsnEngagementSlotComponent* SlotComp)`、`UnregisterSlotOccupancy(AActor* Requester)`、`FindOccupiedSlotComponent(AActor* Requester) const`
  - Private：`TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UTsnEngagementSlotComponent>> OccupancyMap`
  - 说明：维护全局 Requester → SlotComponent 映射，保证同一攻击者同一时间只在一个目标上持有槽位

- [x] **T4B-2** 创建 `Private/Subsystems/TsnEngagementSlotSubsystem.cpp`
  - `RegisterSlotOccupancy`：直接 `OccupancyMap.Add(Requester, SlotComp)`
  - `UnregisterSlotOccupancy`：`OccupancyMap.Remove(Requester)`
  - `FindOccupiedSlotComponent`：查找并返回 `UTsnEngagementSlotComponent*`，失效弱引用时自动清理并返回 null

---

## 阶段 5：NavArea + 站桩障碍物组件（依赖 T1-1）

- [x] **T5-1** 创建 `Public/Components/TsnStanceObstacleComponent.h`
  - 枚举：`ETsnNavModifierMode`（Impassable / HighCost）
  - 枚举：`ETsnMobilityStance`（Moving / Stance）
  - 组件类：`UTsnStanceObstacleComponent`
  - Public API：`EnterStanceMode`、`ExitStanceMode`（均须幂等，重复调用无副作用）、`GetMobilityStance`、`UpdateStanceUnitParams`、`OnOwnerReleased()`（对象池回收清理）
  - `UpdateStanceUnitParams()`：宿主项目修改 ObstacleRadius / RepulsionRadius / RepulsionStrength 后调用，同步更新 NavModifier 和 RepulsionSubsystem 中缓存的参数
  - 属性：`ObstacleRadius`（默认 60，`ClampMin = "10.0"`）、`bUseNavModifier`、`NavModifierMode`、`NavModifierExtraRadius`（默认 45，`ClampMin = "0.0"`）、`RepulsionRadius`（默认 150，`ClampMin = "10.0"`）、`RepulsionStrength`（默认 800，`ClampMin = "0.0"`）、`NavModifierDeactivationDelay`
  - 约束：`RepulsionRadius` 必须大于 `ObstacleRadius`（推荐 1.5x ~ 2.5x）；非法配置需运行时修正为至少 `ObstacleRadius + 30cm`
  - BeginPlay / EndPlay 重写

- [x] **T5-2** 创建 `Private/Components/TsnStanceObstacleComponent.cpp`
  - 实现初始化链：`BeginPlay` → `CacheComponents` → `InitNavModifier`
    - **参数合法化**：通过 `GetSanitizedRadii` 在注册和更新路径统一处理 `RepulsionRadius <= ObstacleRadius` 的情况，并以 effective 半径继续运行
  - 提供半径 sanitize 辅助逻辑，在注册前统一合法化 `ObstacleRadius / RepulsionRadius`
  - `EnterStanceMode`：**幂等检查**（`if (MobilityStance == Stance) return`）→ **重新缓存 CrowdFollowingComp**（防止 Unpossess 后引用失效）→ Crowd → ObstacleOnly → `ActivateNavModifier`（内含 ClearTimer 防止 Timer 竞态）→ 注册到 RepulsionSubsystem（**需 null-guard**）
  - `ExitStanceMode`：**幂等检查**（`if (MobilityStance == Moving) return`）→ **重新缓存 CrowdFollowingComp** → Crowd → Enabled → 延迟关闭 NavModifier → 注销 RepulsionSubsystem（**需 null-guard**）
  - **Timer 竞态防护**：`ActivateNavModifier` 首行必须 `ClearTimer(NavModifierDeactivationTimer)`，确保 Exit→Enter 快速切换时不会误关 NavModifier
  - `ApplyNavAreaClass`：按 Mode 选择 `NavArea_Null`（Impassable）或 `UTsnNavArea_StanceUnit`（HighCost）
  - `UpdateStanceUnitParams`：对新的半径做 `GetSanitizedRadii` 合法化；若启用 NavModifier，使用 `GetEffectiveNavModifierRadius` 更新其 FailsafeExtent 和导航边界；若处于 Stance 状态，再调用 `RepulsionSubsystem::UpdateStanceUnit(...)` 刷新缓存参数
  - `OnOwnerReleased`：强制退出 Stance（调用 `ExitStanceMode` 逻辑）、清除 NavModifier 延迟 Timer
  - 非法配置时输出 ensure/warning，并使用合法化后的 effective 半径继续运行

---

## 阶段 6：排斥力子系统（依赖 T3-1, T5-1）

- [x] **T6-1** 创建 `Public/Subsystems/TsnStanceRepulsionSubsystem.h`
  - 继承 `UTickableWorldSubsystem`
  - Public API：`RegisterStanceUnit`、`UnregisterStanceUnit`、`UpdateStanceUnit`、`GetStanceUnitCount`、`GetStanceUnits()`（返回 Stance 单位列表，用于 Penetration Guard 查询）
  - **新增**：`RegisterMovingUnit`、`UnregisterMovingUnit`（由 `TsnTacticalMovementComponent` 的 BeginPlay/EndPlay 调用）
  - 属性：`OuterRepulsionRatio`（默认 `0.1f`，`UPROPERTY(EditAnywhere)`，外层排斥力阶段系数）、`TangentBlendRatio`（默认 `0.7f`，`ClampMin = 0.0`，`ClampMax = 1.0`，切向混合比例）
  - `Tick` + `GetStatId` 重写

- [x] **T6-2** 创建 `Private/Subsystems/TsnStanceRepulsionSubsystem.cpp`
  - `Tick`：遍历已注册的 `MovingUnits` 列表（而非 `TActorIterator<ACharacter>`），对每个移动单位检查与所有站桩单位的排斥力
    - **注释标注**：当前为 O(站桩数 × 移动数) 遍历；十几到几十单位的场景规模可接受
  - **内部先合并再写入**：对每个 Moving 单位，先遍历所有 Stance 单位累加 `TotalRepulsion`，然后**只调用一次** `SetRepulsionVelocity(TotalRepulsion * DeltaTime)`
  - 双阶段力度计算：内层（穿入 NavModifier 区域）全力排斥；外层（NavModifier ~ RepulsionRadius）按 `RepulsionStrength * OuterRepulsionRatio * OuterFactor` 计算轻推强度
  - 注入 `TsnTacticalMovementComponent` 后，内层/外层统一复用 `MaxRepulsionVelocityRatio` 做最终速度限幅
  - 方向混合：`TangentBlendRatio` 切线引导 + `(1 - TangentBlendRatio)` 径向排斥（默认 70/30，可配置）
  - 作为 `Moving↔Stance` keep-out 的唯一权威来源
  - 当前帧只写入延迟缓冲，排斥力统一在下一帧首次移动更新时消费
  - `UpdateStanceUnit`：直接修改已注册条目的数值（RepulsionRadius、RepulsionStrength、NavModifierRadius）
  - **`RegisterStanceUnit` 幂等**：注册前遍历检查是否已存在，若已注册则直接返回，不产生重复条目。防护 Blueprint 调用者连续两次调用 `EnterStanceMode`（无 Exit）的情况。
  - **`UnregisterStanceUnit` 幂等**：`RemoveAll` 对未注册的 Actor 无副作用，同时顺带清理失效的弱引用条目。
  - `#if ENABLE_DRAW_DEBUG`：可视化内层/外层区域圆环 + 排斥力方向箭头

---

## 阶段 7：分离力组件（依赖 T5-1, T6-1）

- [x] **T7-1** 创建 `Public/Components/TsnUnitSeparationComponent.h`
  - 属性：`SeparationRadius`（默认 120）、`SeparationStrength`（默认 0.5）
  - Public API：`OnOwnerReleased()`（对象池回收清理——停止 Tick，清空 `CachedRepulsionSubsystem`）
  - TickComponent 重写（30Hz）
  - BeginPlay / EndPlay 重写
  - Private：`CachedRepulsionSubsystem`（`UTsnStanceRepulsionSubsystem*`，BeginPlay 时缓存）
  - 头文件需前向声明 `UTsnStanceRepulsionSubsystem`（无需 include）

- [x] **T7-2** 创建 `Private/Components/TsnUnitSeparationComponent.cpp`
  - 仅对 Moving 状态单位生效
  - 实现 `BeginPlay`：缓存 `CachedRepulsionSubsystem` + 向 Subsystem 注册为 Moving 单位（**需 null-guard**）
  - 实现 `EndPlay`：注销 Moving 单位 + 清空 `CachedRepulsionSubsystem = nullptr`（**需 null-guard**）
  - `TickComponent`：直接遍历 `CachedRepulsionSubsystem->GetMovingUnits()`（跳过 Stance 单位）和 `GetStanceUnits()`（Stance 约束裁剪），**不再使用物理 Overlap 查询**（见 design.md §39）
  - **多 Stance 单位裁剪策略**：累加所有 InwardToStance 方向后归一化，对最终 SeparationVector 做一次投影裁剪（而非逐个裁剪），避免遍历顺序依赖和过度衰减
  - 实现 `OnOwnerReleased`：`SetComponentTickEnabled(false)` + 注销 Moving 单位 + `CachedRepulsionSubsystem = nullptr`
  - 通过 `AddMovementInput(dir, SeparationStrength)` 注入（与路径跟随自然融合）；**不乘 DeltaTime**——`AddMovementInput` 仅写入 `PendingMovementInputVector`，速度积分由 CMC 自身的 `DeltaSeconds` 控制，30Hz tick 稀疏性不会导致低帧率下分离力脉冲放大
  - `#if ENABLE_DRAW_DEBUG`：可视化分离力方向 + Stance 约束裁剪后的最终方向
  - **注意**：需要 `#include "Subsystems/TsnStanceRepulsionSubsystem.h"`，该头文件在阶段 6 创建，与 T3-2 同理，T7-2 须在阶段 6（CP-2）完成后才可实现（在 CP-3 统一验证）

---

## 阶段 8：行为树节点（依赖 T2-1, T4-1, T5-1）

- [x] **T8-1** 创建 `Public/BehaviorTree/TsnBTTask_ChaseEngagementTarget.h` + `Private/.cpp`
  - 追击目标至预战斗距离；已在范围内立即 Succeeded
  - 属性：`TargetKey`、`PreEngagementRadiusMultiplier`（默认 1.5）
  - **接口合法性检查**：Pawn 未实现 `ITsnTacticalUnit` 或 `GetEngagementRange()` 返回 ≤ 0 时，输出 `UE_LOG(Warning)` 并直接返回 `Failed`
  - `ExecuteTask` + `TickTask` + `AbortTask`
  - `AbortTask`：`StopMovement()` + 返回 `Aborted`（仅清理，不做决策）
  - `InitializeFromAsset`：验证 `TargetKey` 是否绑定有效 Blackboard Key；无效时编辑器日志提示
  - `GetNodeName()`：返回 `"Chase Engagement Target"`

- [x] **T8-2** 创建 `Public/BehaviorTree/TsnBTTask_MoveToEngagementSlot.h` + `Private/.cpp`
  - 已在攻击距离 → 立即 Succeeded；否则认领槽位并移动
  - 三退出条件：
    - (a) 到达槽位 → `EBTNodeResult::Succeeded`
    - (b) 中途进入攻击距离（目标向我方靠近）→ `EBTNodeResult::Succeeded`
    - (c) 超时（路径异常兜底）→ `EBTNodeResult::Failed`
  - 属性：`TargetKey`、`AcceptanceRadius`、`RePathCheckInterval`（默认 0.1s）、`RePathDistanceThreshold`（默认 50cm）、`MaxApproachTime`（默认 5s，可配置）
  - **接口合法性检查**：Pawn 未实现 `ITsnTacticalUnit` 或 `GetEngagementRange()` 返回 ≤ 0 时，输出 `UE_LOG(Warning)` 并直接返回 `Failed`
  - **目标失效处理**：
    - 监听 `AActor::OnDestroyed` 和 `AActor::OnEndPlay` 两个委托，确保目标被真正销毁或被对象池回收时都能正确释放槽位并返回 `Failed`
    - `OnTaskFinished` 中统一解绑两个委托，覆盖所有退出路径（Succeeded/Failed/Aborted）
  - `InitializeFromAsset`：验证 `TargetKey` 是否绑定有效 Blackboard Key；无效时编辑器日志提示
  - `GetNodeName()`：返回 `"Move To Engagement Slot"`
  - 追踪的是槽位的世界快照，不是固定世界点；目标移动时需基于已持有的本地槽位参数刷新快照并重发 MoveTo

- [x] **T8-3** 创建 `Public/BehaviorTree/TsnBTTask_EnterStanceMode.h`
  - 仅头文件（inline 实现），找 `TsnStanceObstacleComponent` 调用 `EnterStanceMode()`
  - `GetNodeName()`（inline）：返回 `"Enter Stance Mode"`

- [x] **T8-4** 创建 `Public/BehaviorTree/TsnBTTask_ExitStanceMode.h`
  - 仅头文件（inline 实现），调用 `ExitStanceMode()`
  - `GetNodeName()`（inline）：返回 `"Exit Stance Mode"`

- [x] **T8-5** 创建 `Public/BehaviorTree/TsnBTTask_ReleaseEngagementSlot.h`
  - 仅头文件（inline 实现），找 `TsnEngagementSlotComponent` 调用 `ReleaseSlot()`
  - 失败时也返回 `EBTNodeResult::Succeeded`（不阻塞流程）
  - `GetNodeName()`（inline）：返回 `"Release Engagement Slot"`

- [x] **T8-6** 创建 `Public/BehaviorTree/TsnBTDecorator_IsTargetStationary.h` + `Private/.cpp`
  - `CalculateRawConditionValue`：检查目标速度 < SpeedThreshold
  - 属性：`TargetKey`、`SpeedThreshold`（默认 10 cm/s）
  - `InitializeFromAsset`：验证 `TargetKey` 是否绑定有效 Blackboard Key；无效时编辑器日志提示
  - `GetNodeName()`：返回 `"Is Target Stationary"`

---

## 阶段 9：编译验证

- [x] **T9-1** 编译检查点 CP-1（阶段 0~2 完成后）
  - 刷新 UBT 项目文件
  - 编译 SquadNavDevEditor Win64 Development
  - 验证基础层（Build.cs、TsnLog、NavArea、Interface）无编译错误
  - 这些层完全独立，无交叉引用
  - 发现问题立即修复

- [x] **T9-2** 编译检查点 CP-2（阶段 3~6 完成后）
  - 刷新 UBT 项目文件
  - 编译 SquadNavDevEditor Win64 Development
  - 验证组件层和子系统层（TacticalMovement、EngagementSlot、**EngagementSlotSubsystem**、StanceObstacle、RepulsionSubsystem）无编译错误
  - **说明**：TacticalMovementComponent.cpp 的 BeginPlay/EndPlay 依赖 TsnStanceRepulsionSubsystem.h，因此必须等阶段 6 完成后才能验证；EngagementSlotSubsystem（阶段 4B）无跨 TSN 头文件依赖，可最早编译
  - 发现问题立即修复

- [x] **T9-3** 编译检查点 CP-3（阶段 7~8 完成后）
  - 刷新 UBT 项目文件
  - 编译 SquadNavDevEditor Win64 Development
  - 验证 UnitSeparation + BT 节点无编译错误
  - **说明**：UnitSeparationComponent.cpp 的 BeginPlay/EndPlay 依赖 TsnStanceRepulsionSubsystem.h（阶段 6），因此 T7-2 须在 CP-2 完成后才可实现
  - 发现问题立即修复

- [x] **T9-4** 最终编译验证（所有文件添加完毕后）
  - 刷新 UBT 项目文件
  - 编译 SquadNavDevEditor Win64 Development
  - 命令：`"E:\UnrealEngine\UE_5.7\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" SquadNavDevEditor Win64 Development -Project="E:\Projects_Unreal\SquadNavDev\SquadNavDev.uproject" -rocket -progress`
  - 验证：0 errors（警告可接受）

- [x] **T9-5** 修复所有编译错误

- [x] **T9-6** 重新编译，确认 0 errors

---

## 依赖图

```
T0-1 (Build.cs) ──────────────────────────────────────────►
T0-3 (TsnLog)   ─────────── ★ 所有后续 .cpp 的前置 ──────►
T1-1 (NavArea)  ──────────────────────────────────────────►
T2-1 (Interface)──────────────────────────────────────────►
T3-1+T3-2 (TacticalMovement) ─────────────────────────────►
T4-1+T4-2 (EngagementSlot←T4B) ───────────────────────────►  T9-1 → T9-2 → T9-3 → T9-4
T4B-1+T4B-2 (EngagementSlotSubsystem) ────────────────────►
T5-1+T5-2 (StanceObstacle←T1-1) ─────────────────────────►
T6-1+T6-2 (RepulsionSubsystem←T3-1,T5-1) ─────────────────►
T7-1+T7-2 (UnitSeparation←T5-1,T6-1) ────────────────────►
T8-1~T8-6 (BT Nodes←T2-1,T4-1,T5-1) ────────────────────►
```

可并行执行的批次：
- **批次 A**（无依赖）：T0-1、T0-3、T1-1、T2-1、T4B-x、**T4-1**
- **批次 B**（依赖批次 A）：T3-x、**T4-2**（T4-2 的 .cpp 依赖 T4B-x 的头文件）
- **批次 C**（依赖 T1-1）：T5-x
- **批次 D**（依赖 T3-x + T5-x）：T6-x
- **批次 E**（依赖 T5-x + T6-x）：T7-x
- **批次 F**（依赖 T2-1 + T4-x + T5-x）：T8-x
- **批次 G**：T9-x（所有文件就绪后）
