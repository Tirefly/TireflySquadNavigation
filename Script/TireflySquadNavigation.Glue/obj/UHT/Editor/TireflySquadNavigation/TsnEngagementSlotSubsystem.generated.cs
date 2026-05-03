using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 全局交战槽位注册表子系统 —— WorldSubsystem（自动创建）。
/// 
/// 维护 Requester → SlotComponent 的全局映射，实现"单目标槽位约束"：
/// 同一攻击者同一时间只能在一个目标上持有槽位。
/// TsnEngagementSlotComponent::RequestSlot 内部通过本子系统检查并自动释放旧槽位。
/// 
/// 不需要 Tick，仅做注册/查询。
/// </summary>
[UClass, GeneratedType("TsnEngagementSlotSubsystem", "UnrealSharp.TireflySquadNavigation.TsnEngagementSlotSubsystem")]
public partial class UTsnEngagementSlotSubsystem : UnrealSharp.Engine.UWorldSubsystem
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnEngagementSlotSubsystem).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnEngagementSlotSubsystem");
    
}