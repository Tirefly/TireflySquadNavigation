## 1. StateTree Runtime Nodes

- [x] 1.1 新增 `Public/StateTree/` 与 `Private/StateTree/` 目录，并实现 5 个 TSN StateTree 任务：追击目标、接近交战槽位、进入站姿、退出站姿、释放槽位
- [x] 1.2 为 5 个任务定义显式 `InstanceData` 契约，明确编辑器可绑定参数、默认值来源、以及必要的运行时内部状态
- [x] 1.3 新增 2 个 TSN StateTree 评估器：`CombatContext`、`TargetMotion`，统一暴露目标有效性、距离区间、目标速度与静止状态
- [x] 1.4 为 2 个评估器定义显式输入 / 输出字段契约，明确哪些字段供 transition 直接消费
- [x] 1.5 新增 5 个 TSN StateTree 条件：`HasValidTarget`、`IsInPreEngagementRange`、`IsInEngagementRange`、`IsTargetStationary`、`IsInStanceMode`
- [x] 1.6 为 5 个条件定义输入字段与反转语义，优先消费评估器输出，只有 `IsInStanceMode` 直接查询组件状态
- [x] 1.7 为异步移动任务补齐运行时上下文校验与状态语义：AIController / Pawn 解析、`ITsnTacticalUnit` 校验、目标失效处理、MoveTo 失败处理、超时处理
- [x] 1.8 为 `MoveToEngagementSlot` 明确“成功时保留槽位、失败/中断时释放槽位”的生命周期规则，并在任务退出路径上实现一致清理
- [x] 1.9 为评估器与条件明确数据边界：评估器仅输出 TSN 通用观测量，不承接宿主项目的选目标或攻击策略

## 2. Build And Docs

- [x] 2.1 更新 `TireflySquadNavigation.Build.cs`，补齐自定义 StateTree 任务所需的私有模块依赖
- [x] 2.2 更新技术方案文档与纯蓝图测试指南，增加 StateTree 最小推荐状态链、任务职责、推荐评估器 / 条件组合，以及“BT 自动化测试保持不变”的边界说明

## 3. Validation

- [x] 3.1 新增文件后刷新 UBT 项目文件
- [x] 3.2 编译 `SquadNavDevEditor Win64 Development`，确认新增 StateTree 节点和模块依赖均可通过编译
- [ ] 3.3 在编辑器中确认新任务、评估器、条件都能出现在 StateTree 资产可选列表中，并能通过字段绑定形成 `HasValidTarget + IsInPreEngagementRange/IsInEngagementRange` 驱动的最小状态链（需手动在编辑器内执行）

## 4. Test Module Demo StateTree Nodes

- [x] 4.1 更新 `TireflySquadNavigationTest.Build.cs`，新增 `StateTreeModule` 私有依赖
- [x] 4.2 在 `TireflySquadNavigationTest/Public/StateTree/` 与 `TireflySquadNavigationTest/Private/StateTree/` 下新增 `FTsnTestStateTreeTask_SelectTarget`，行为与 `UTsnTestBTTask_SelectTarget` 一致；输出 `TargetActor`、`bHasTarget` 至 InstanceData Output 字段
- [x] 4.3 在同一目录下新增 `FTsnTestStateTreeTask_Attack`，行为与 `UTsnTestBTTask_Attack` 一致；`AttackDuration` 为输入，运行时累计 `ElapsedTime` 到时返回 Succeeded
- [x] 4.4 编译 `SquadNavDevEditor Win64 Development`，确认测试模块连同新 demo Task 可以通过编译
- [x] 4.5 更新 `BehaviorTree 测试执行指南.md` 与 `StateTree 测试执行指南.md`，拆分 BT / StateTree 两条最小执行路径
