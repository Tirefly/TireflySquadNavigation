using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 在预战斗距离内认领交战槽位并移动至槽位世界快照，目标静止或移动均适用。
/// 
/// 若进入时已在攻击距离内，立即 Succeeded（不分配槽位，后续直接站桩攻击）。
/// 否则认领最近空闲槽位并寻路至槽位；槽位身份由目标本地极坐标维持，
/// 目标移动时周期性重算当前世界快照并重寻路。
/// 到达槽位或超时后 Failed（超时为异常兜底），不阻塞攻击流程。
/// 
/// 本节点使用 bCreateNodeInstance = true，每个 BehaviorTreeComponent 拥有独立实例，
/// 因此运行时状态（包括委托绑定）可安全存储为类成员变量，无需使用 NodeMemory。
/// </summary>
[UClass, GeneratedType("TsnBTTask_MoveToEngagementSlot", "UnrealSharp.TireflySquadNavigation.TsnBTTask_MoveToEngagementSlot")]
public partial class UTsnBTTask_MoveToEngagementSlot : UnrealSharp.AIModule.UBTTaskNode
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnBTTask_MoveToEngagementSlot).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnBTTask_MoveToEngagementSlot");
    static UTsnBTTask_MoveToEngagementSlot()
    {
        IntPtr TargetKey_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TargetKey");
        TargetKey_Offset = CallGetPropertyOffset(TargetKey_NativeProperty);
        IntPtr AcceptanceRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AcceptanceRadius");
        AcceptanceRadius_Offset = CallGetPropertyOffset(AcceptanceRadius_NativeProperty);
        IntPtr RePathCheckInterval_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "RePathCheckInterval");
        RePathCheckInterval_Offset = CallGetPropertyOffset(RePathCheckInterval_NativeProperty);
        IntPtr RePathDistanceThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "RePathDistanceThreshold");
        RePathDistanceThreshold_Offset = CallGetPropertyOffset(RePathDistanceThreshold_NativeProperty);
        IntPtr MaxApproachTime_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "MaxApproachTime");
        MaxApproachTime_Offset = CallGetPropertyOffset(MaxApproachTime_NativeProperty);
        IntPtr ZeroIntentRecoveryDelay_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ZeroIntentRecoveryDelay");
        ZeroIntentRecoveryDelay_Offset = CallGetPropertyOffset(ZeroIntentRecoveryDelay_NativeProperty);
        IntPtr ZeroIntentRequestedSpeedThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ZeroIntentRequestedSpeedThreshold");
        ZeroIntentRequestedSpeedThreshold_Offset = CallGetPropertyOffset(ZeroIntentRequestedSpeedThreshold_NativeProperty);
        IntPtr ZeroIntentActualSpeedThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ZeroIntentActualSpeedThreshold");
        ZeroIntentActualSpeedThreshold_Offset = CallGetPropertyOffset(ZeroIntentActualSpeedThreshold_NativeProperty);
        OnTargetDestroyed_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "OnTargetDestroyed");
        OnTargetDestroyed_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(OnTargetDestroyed_NativeFunction);
        OnTargetDestroyed_DestroyedActor_Offset = CallGetPropertyOffsetFromName(OnTargetDestroyed_NativeFunction, "DestroyedActor");
        OnTargetEndPlay_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "OnTargetEndPlay");
        OnTargetEndPlay_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(OnTargetEndPlay_NativeFunction);
        OnTargetEndPlay_Actor_Offset = CallGetPropertyOffsetFromName(OnTargetEndPlay_NativeFunction, "Actor");
        OnTargetEndPlay_EndPlayReason_Offset = CallGetPropertyOffsetFromName(OnTargetEndPlay_NativeFunction, "EndPlayReason");
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
    /// 槽位到达判定容差，距离槽位世界快照 ≤ 此值时视为到达（不要设为 0，以免抖动）
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
    
    
    static int RePathCheckInterval_Offset;
    
    /// <summary>
    /// 检查是否需要刷新槽位世界快照并重新寻路的间隔（秒）
    /// </summary>
    public float RePathCheckInterval
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + RePathCheckInterval_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + RePathCheckInterval_Offset, 0, value);
        }
    }
    
    
    static int RePathDistanceThreshold_Offset;
    
    /// <summary>
    /// 目标移动超过此距离才触发重新寻路。
    /// 建议不大于 2 × AcceptanceRadius，避免长时间追逐旧快照点。
    /// </summary>
    public float RePathDistanceThreshold
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + RePathDistanceThreshold_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + RePathDistanceThreshold_Offset, 0, value);
        }
    }
    
    
    static int MaxApproachTime_Offset;
    
    /// <summary>
    /// 预占位接近阶段的最大允许时间（秒）。
    /// 超时后以 Failed 结束任务（路径异常兜底，即 §4.4 退出条件 c），由 BT 失败分支处理恢复逻辑。
    /// 0 = 不限时（不推荐，路径异常时可能导致单位无限等待）。
    /// </summary>
    public float MaxApproachTime
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + MaxApproachTime_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + MaxApproachTime_Offset, 0, value);
        }
    }
    
    
    static int ZeroIntentRecoveryDelay_Offset;
    
    /// <summary>
    /// 当预占位任务仍在 InProgress，但 RequestedVelocity 长时间为 0 时，
    /// 认为当前槽位 MoveTo 意图已静默失效，超过该时长后主动重发一次请求。
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
    
    // OnTargetDestroyed
    static IntPtr OnTargetDestroyed_NativeFunction;
    static int OnTargetDestroyed_ParamsSize;
    static int OnTargetDestroyed_DestroyedActor_Offset;
    
    /// <summary>
    /// 目标 OnDestroyed 回调——释放槽位并以 Failed 结束任务
    /// </summary>
    [UFunction, GeneratedType("OnTargetDestroyed", "UnrealSharp.TireflySquadNavigation.OnTargetDestroyed")]
    private void OnTargetDestroyed(UnrealSharp.Engine.AActor destroyedActor)
    {
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[OnTargetDestroyed_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(paramsBuffer + OnTargetDestroyed_DestroyedActor_Offset, 0, destroyedActor);
            
            UObjectExporter.CallInvokeNativeFunction(NativeObject, OnTargetDestroyed_NativeFunction, paramsBuffer, IntPtr.Zero);
            
        }
    }
    
    // OnTargetEndPlay
    static IntPtr OnTargetEndPlay_NativeFunction;
    static int OnTargetEndPlay_ParamsSize;
    static int OnTargetEndPlay_Actor_Offset;
    static int OnTargetEndPlay_EndPlayReason_Offset;
    
    /// <summary>
    /// 目标 OnEndPlay 回调——支持对象池回收场景，释放槽位并以 Failed 结束任务
    /// </summary>
    [UFunction, GeneratedType("OnTargetEndPlay", "UnrealSharp.TireflySquadNavigation.OnTargetEndPlay")]
    private void OnTargetEndPlay(UnrealSharp.Engine.AActor actor, UnrealSharp.Engine.EEndPlayReason endPlayReason)
    {
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[OnTargetEndPlay_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(paramsBuffer + OnTargetEndPlay_Actor_Offset, 0, actor);
            EnumMarshaller<UnrealSharp.Engine.EEndPlayReason>.ToNative(paramsBuffer + OnTargetEndPlay_EndPlayReason_Offset, 0, endPlayReason);
            
            UObjectExporter.CallInvokeNativeFunction(NativeObject, OnTargetEndPlay_NativeFunction, paramsBuffer, IntPtr.Zero);
            
        }
    }
    
    
}