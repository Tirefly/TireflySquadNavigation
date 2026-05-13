using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 只读判断当前单位是否已经进入当前目标上的合法接战带。
/// 
/// 该节点不会认领槽位、不会刷新槽位，也不会发起移动请求。
/// 只有当单位已经在目标上持有槽位时，才会基于当前帧的槽位世界快照
/// 和真实接战半径预算判断是否处于合法接战带内。
/// </summary>
[UClass, GeneratedType("TsnBTDecorator_IsWithinEngagementBand", "UnrealSharp.TireflySquadNavigation.TsnBTDecorator_IsWithinEngagementBand")]
public partial class UTsnBTDecorator_IsWithinEngagementBand : UnrealSharp.AIModule.UBTDecorator
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnBTDecorator_IsWithinEngagementBand).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnBTDecorator_IsWithinEngagementBand");
    static UTsnBTDecorator_IsWithinEngagementBand()
    {
        IntPtr TargetKey_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetKey");
        TargetKey_Offset = CallGetPropertyOffset(TargetKey_NativeProperty);
        IntPtr AcceptanceRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AcceptanceRadius");
        AcceptanceRadius_Offset = CallGetPropertyOffset(AcceptanceRadius_NativeProperty);
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
    
    
    static int AcceptanceRadius_Offset;
    
    /// <summary>
    /// 接战带到位判定容差。
    /// 最终仍会受真实攻击距离预算约束，避免把已脱离攻击窗的位置误判为合法接战带。
    /// </summary>
    public float AcceptanceRadius
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + AcceptanceRadius_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + AcceptanceRadius_Offset, 0, value);
        }
    }
    
    
}