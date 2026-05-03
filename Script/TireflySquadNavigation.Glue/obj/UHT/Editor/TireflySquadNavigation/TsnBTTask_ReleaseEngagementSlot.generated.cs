using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 释放当前持有的交战槽位，失败时也返回 Succeeded（不阻塞流程）
/// </summary>
[UClass, GeneratedType("TsnBTTask_ReleaseEngagementSlot", "UnrealSharp.TireflySquadNavigation.TsnBTTask_ReleaseEngagementSlot")]
public partial class UTsnBTTask_ReleaseEngagementSlot : UnrealSharp.AIModule.UBTTaskNode
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnBTTask_ReleaseEngagementSlot).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnBTTask_ReleaseEngagementSlot");
    static UTsnBTTask_ReleaseEngagementSlot()
    {
        IntPtr TargetKey_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetKey");
        TargetKey_Offset = CallGetPropertyOffset(TargetKey_NativeProperty);
    }
    
    static int TargetKey_Offset;
    
    /// <summary>
    /// Blackboard 中攻击目标的 Key
    /// </summary>
    public UnrealSharp.AIModule.FBlackboardKeySelector TargetKey
    {
        get
        {
            return StructMarshaller<UnrealSharp.AIModule.FBlackboardKeySelector>.FromNative(NativeObject + TargetKey_Offset, 0);
        }
        set
        {
            StructMarshaller<UnrealSharp.AIModule.FBlackboardKeySelector>.ToNative(NativeObject + TargetKey_Offset, 0, value);
        }
    }
    
    
}