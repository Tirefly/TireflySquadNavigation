using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigationTest;

/// <summary>
/// 测试用靶标角色。
/// 
/// 被攻击目标，挂载 TsnEngagementSlotComponent 供攻击者认领槽位。
/// 支持可选的 Spline 巡逻功能，用于验证移动目标时槽位追踪。
/// 含 DrawDebug 槽位环可视化。
/// </summary>
[UClass, GeneratedType("TsnTestTargetDummy", "UnrealSharp.TireflySquadNavigationTest.TsnTestTargetDummy")]
public partial class ATsnTestTargetDummy : UnrealSharp.Engine.ACharacter
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(ATsnTestTargetDummy).GetAssemblyName(), "UnrealSharp.TireflySquadNavigationTest", "TsnTestTargetDummy");
    static ATsnTestTargetDummy()
    {
        IntPtr EngagementSlotComp_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EngagementSlotComp");
        EngagementSlotComp_Offset = CallGetPropertyOffset(EngagementSlotComp_NativeProperty);
        IntPtr bDrawDebugSlotRings_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bDrawDebugSlotRings");
        bDrawDebugSlotRings_Offset = CallGetPropertyOffset(bDrawDebugSlotRings_NativeProperty);
        IntPtr bPatrolAlongSpline_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bPatrolAlongSpline");
        bPatrolAlongSpline_Offset = CallGetPropertyOffset(bPatrolAlongSpline_NativeProperty);
        IntPtr SplineActor_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SplineActor");
        SplineActor_Offset = CallGetPropertyOffset(SplineActor_NativeProperty);
        IntPtr PatrolSpeed_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "PatrolSpeed");
        PatrolSpeed_Offset = CallGetPropertyOffset(PatrolSpeed_NativeProperty);
    }
    
    static int EngagementSlotComp_Offset;
    
    /// <summary>
    /// 交战槽位组件
    /// </summary>
    public UnrealSharp.TireflySquadNavigation.UTsnEngagementSlotComponent EngagementSlotComp
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnEngagementSlotComponent>.FromNative(NativeObject + EngagementSlotComp_Offset, 0);
        }
        set
        {
            ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnEngagementSlotComponent>.ToNative(NativeObject + EngagementSlotComp_Offset, 0, value);
        }
    }
    
    
    static int bDrawDebugSlotRings_Offset;
    
    /// <summary>
    /// 是否绘制槽位环可视化
    /// </summary>
    public bool DrawDebugSlotRings
    {
        get
        {
            return BoolMarshaller.FromNative(NativeObject + bDrawDebugSlotRings_Offset, 0);
        }
        set
        {
            BoolMarshaller.ToNative(NativeObject + bDrawDebugSlotRings_Offset, 0, value);
        }
    }
    
    
    static int bPatrolAlongSpline_Offset;
    
    /// <summary>
    /// 是否沿 Spline 巡逻
    /// </summary>
    public bool PatrolAlongSpline
    {
        get
        {
            return BoolMarshaller.FromNative(NativeObject + bPatrolAlongSpline_Offset, 0);
        }
        set
        {
            BoolMarshaller.ToNative(NativeObject + bPatrolAlongSpline_Offset, 0, value);
        }
    }
    
    
    static int SplineActor_Offset;
    
    /// <summary>
    /// 持有 SplineComponent 的 Actor
    /// </summary>
    public UnrealSharp.Engine.AActor SplineActor
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.Engine.AActor>.FromNative(NativeObject + SplineActor_Offset, 0);
        }
        set
        {
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(NativeObject + SplineActor_Offset, 0, value);
        }
    }
    
    
    static int PatrolSpeed_Offset;
    
    /// <summary>
    /// 巡逻速度 (cm/s)
    /// </summary>
    public float PatrolSpeed
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + PatrolSpeed_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + PatrolSpeed_Offset, 0, value);
        }
    }
    
    
}