using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 判断目标是否静止（速度 < SpeedThreshold）。
/// 辅助工具节点，供行为树根据目标运动状态微调策略（非主流程必须）。
/// </summary>
[UClass, GeneratedType("TsnBTDecorator_IsTargetStationary", "UnrealSharp.TireflySquadNavigation.TsnBTDecorator_IsTargetStationary")]
public partial class UTsnBTDecorator_IsTargetStationary : UnrealSharp.AIModule.UBTDecorator
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnBTDecorator_IsTargetStationary).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnBTDecorator_IsTargetStationary");
    static UTsnBTDecorator_IsTargetStationary()
    {
        IntPtr TargetKey_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetKey");
        TargetKey_Offset = CallGetPropertyOffset(TargetKey_NativeProperty);
        IntPtr SpeedThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SpeedThreshold");
        SpeedThreshold_Offset = CallGetPropertyOffset(SpeedThreshold_NativeProperty);
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
    
    
    static int SpeedThreshold_Offset;
    
    /// <summary>
    /// 速度低于此阈值视为静止（cm/s）
    /// </summary>
    public float SpeedThreshold
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + SpeedThreshold_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + SpeedThreshold_Offset, 0, value);
        }
    }
    
    
}