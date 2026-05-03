using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 追击目标直至进入预战斗距离（AttackRange × PreEngagementRadiusMultiplier），不分配交战槽位。
/// 
/// 追击阶段只负责拉近距离，不分配槽位，避免因目标大幅移动导致槽位坐标漂移失效。
/// 进入预战斗距离后 Succeeded，由后续 TsnBTTask_MoveToEngagementSlot 负责槽位认领。
/// 若启动时已在预战斗距离（含已在攻击距离内）则立即 Succeeded。
/// 
/// 本节点使用 bCreateNodeInstance = true，每个 BehaviorTreeComponent 拥有独立实例，
/// 因此运行时状态可安全存储为类成员变量，无需使用 NodeMemory。
/// </summary>
[UClass, GeneratedType("TsnBTTask_ChaseEngagementTarget", "UnrealSharp.TireflySquadNavigation.TsnBTTask_ChaseEngagementTarget")]
public partial class UTsnBTTask_ChaseEngagementTarget : UnrealSharp.AIModule.UBTTaskNode
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnBTTask_ChaseEngagementTarget).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnBTTask_ChaseEngagementTarget");
    static UTsnBTTask_ChaseEngagementTarget()
    {
        IntPtr TargetKey_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetKey");
        TargetKey_Offset = CallGetPropertyOffset(TargetKey_NativeProperty);
        IntPtr PreEngagementRadiusMultiplier_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "PreEngagementRadiusMultiplier");
        PreEngagementRadiusMultiplier_Offset = CallGetPropertyOffset(PreEngagementRadiusMultiplier_NativeProperty);
        IntPtr PathRecoveryInterval_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "PathRecoveryInterval");
        PathRecoveryInterval_Offset = CallGetPropertyOffset(PathRecoveryInterval_NativeProperty);
        IntPtr ZeroIntentRecoveryDelay_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ZeroIntentRecoveryDelay");
        ZeroIntentRecoveryDelay_Offset = CallGetPropertyOffset(ZeroIntentRecoveryDelay_NativeProperty);
        IntPtr ZeroIntentRequestedSpeedThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ZeroIntentRequestedSpeedThreshold");
        ZeroIntentRequestedSpeedThreshold_Offset = CallGetPropertyOffset(ZeroIntentRequestedSpeedThreshold_NativeProperty);
        IntPtr ZeroIntentActualSpeedThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ZeroIntentActualSpeedThreshold");
        ZeroIntentActualSpeedThreshold_Offset = CallGetPropertyOffset(ZeroIntentActualSpeedThreshold_NativeProperty);
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
    
    
    static int PreEngagementRadiusMultiplier_Offset;
    
    /// <summary>
    /// 预战斗距离倍数，相对于 AttackRange 的倍率。
    /// 追击阶段到达距离 ≤ AttackRange × PreEngagementRadiusMultiplier 时停止。
    /// 推荐 1.3~2.0：值越大，单位越早开始占位，战场布局越提前展开。
    /// </summary>
    public float PreEngagementRadiusMultiplier
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + PreEngagementRadiusMultiplier_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + PreEngagementRadiusMultiplier_Offset, 0, value);
        }
    }
    
    
    static int PathRecoveryInterval_Offset;
    
    /// <summary>
    /// 检查当前 MoveTo 是否因动态 NavMesh / 动态障碍而停住的间隔（秒）。
    /// 若路径跟随已不在 Moving，但仍未进入预战斗距离，则重发一次 MoveTo。
    /// </summary>
    public float PathRecoveryInterval
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + PathRecoveryInterval_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + PathRecoveryInterval_Offset, 0, value);
        }
    }
    
    
    static int ZeroIntentRecoveryDelay_Offset;
    
    /// <summary>
    /// 当任务仍在 InProgress，但 RequestedVelocity 长时间为 0 时，
    /// 认为当前追击路径意图已静默失效，超过该时长后主动重发 MoveTo。
    /// </summary>
    public float ZeroIntentRecoveryDelay
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + ZeroIntentRecoveryDelay_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + ZeroIntentRecoveryDelay_Offset, 0, value);
        }
    }
    
    
    static int ZeroIntentRequestedSpeedThreshold_Offset;
    
    /// <summary>
    /// 判定“路径请求速度已经掉空”时使用的阈值（cm/s）。
    /// 低于该值时，视为当前帧几乎没有可用的路径推进意图。
    /// </summary>
    public float ZeroIntentRequestedSpeedThreshold
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + ZeroIntentRequestedSpeedThreshold_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + ZeroIntentRequestedSpeedThreshold_Offset, 0, value);
        }
    }
    
    
    static int ZeroIntentActualSpeedThreshold_Offset;
    
    /// <summary>
    /// 判定“单位实际几乎停住”时使用的二维速度阈值（cm/s）。
    /// 仅当请求速度和实际速度都很低时，才进入零意图恢复逻辑。
    /// </summary>
    public float ZeroIntentActualSpeedThreshold
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + ZeroIntentActualSpeedThreshold_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + ZeroIntentActualSpeedThreshold_Offset, 0, value);
        }
    }
    
    
}