using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 自定义角色移动组件。
/// 
/// 提供合法的排斥力注入接口，在 CalcVelocity（Super 完成后）统一叠加。
/// UE 5.7 中无论使用普通 PathFollowing 还是 DetourCrowd，CalcVelocity 均在
/// PhysWalking 中每帧调用，因此单一注入点即可覆盖所有路径。
/// 
/// 站桩排斥力采用"至少 1 帧延迟消费"契约：
/// Subsystem 在帧 N 写入，MovementComponent 在帧 N+1 的首次 CalcVelocity 中消费，
/// 从而彻底消除 Tick 先后不确定性。
/// 
/// 第三层防线——站桩 keep-out 防护（Stance Penetration Guard）：
/// CalcVelocity 最终速度确定后，在 RepulsionRadius 内渐进裁剪朝向 Stance 单位的向内分量；
/// 进入 ObstacleRadius 后则完全裁剪，避免继续硬挤静态站桩单位或试图穿过过窄缝隙。
/// </summary>
[UClass, GeneratedType("TsnTacticalMovementComponent", "UnrealSharp.TireflySquadNavigation.TsnTacticalMovementComponent")]
public partial class UTsnTacticalMovementComponent : UnrealSharp.Engine.UCharacterMovementComponent
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnTacticalMovementComponent).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnTacticalMovementComponent");
    static UTsnTacticalMovementComponent()
    {
        IntPtr MaxRepulsionVelocityRatio_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "MaxRepulsionVelocityRatio");
        MaxRepulsionVelocityRatio_Offset = CallGetPropertyOffset(MaxRepulsionVelocityRatio_NativeProperty);
        IntPtr OverspeedToleranceRatio_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "OverspeedToleranceRatio");
        OverspeedToleranceRatio_Offset = CallGetPropertyOffset(OverspeedToleranceRatio_NativeProperty);
        IntPtr OuterBandInwardClipRatio_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "OuterBandInwardClipRatio");
        OuterBandInwardClipRatio_Offset = CallGetPropertyOffset(OuterBandInwardClipRatio_NativeProperty);
        IntPtr EscapeModeDuration_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeDuration");
        EscapeModeDuration_Offset = CallGetPropertyOffset(EscapeModeDuration_NativeProperty);
        IntPtr EscapeModeSpeedRatio_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeSpeedRatio");
        EscapeModeSpeedRatio_Offset = CallGetPropertyOffset(EscapeModeSpeedRatio_NativeProperty);
        IntPtr EscapeModeStuckSpeedThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeStuckSpeedThreshold");
        EscapeModeStuckSpeedThreshold_Offset = CallGetPropertyOffset(EscapeModeStuckSpeedThreshold_NativeProperty);
        IntPtr EscapeModeRequestedSpeedThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeRequestedSpeedThreshold");
        EscapeModeRequestedSpeedThreshold_Offset = CallGetPropertyOffset(EscapeModeRequestedSpeedThreshold_NativeProperty);
        IntPtr EscapeModeMinRepulsionRatio_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeMinRepulsionRatio");
        EscapeModeMinRepulsionRatio_Offset = CallGetPropertyOffset(EscapeModeMinRepulsionRatio_NativeProperty);
        IntPtr EscapeModeOppositionDotThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeOppositionDotThreshold");
        EscapeModeOppositionDotThreshold_Offset = CallGetPropertyOffset(EscapeModeOppositionDotThreshold_NativeProperty);
        IntPtr EscapeModeEnterMargin_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeEnterMargin");
        EscapeModeEnterMargin_Offset = CallGetPropertyOffset(EscapeModeEnterMargin_NativeProperty);
        IntPtr EscapeModeReleaseMargin_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeReleaseMargin");
        EscapeModeReleaseMargin_Offset = CallGetPropertyOffset(EscapeModeReleaseMargin_NativeProperty);
        IntPtr EscapeModeTriggerFrames_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeTriggerFrames");
        EscapeModeTriggerFrames_Offset = CallGetPropertyOffset(EscapeModeTriggerFrames_NativeProperty);
        IntPtr EscapeModeClearFrames_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeClearFrames");
        EscapeModeClearFrames_Offset = CallGetPropertyOffset(EscapeModeClearFrames_NativeProperty);
        IntPtr EscapeModeTangentCarryRatio_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EscapeModeTangentCarryRatio");
        EscapeModeTangentCarryRatio_Offset = CallGetPropertyOffset(EscapeModeTangentCarryRatio_NativeProperty);
        IntPtr StationaryLogThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "StationaryLogThreshold");
        StationaryLogThreshold_Offset = CallGetPropertyOffset(StationaryLogThreshold_NativeProperty);
        IntPtr StationaryLogSpeedThreshold_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "StationaryLogSpeedThreshold");
        StationaryLogSpeedThreshold_Offset = CallGetPropertyOffset(StationaryLogSpeedThreshold_NativeProperty);
        IntPtr StationaryLogDistanceTolerance_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "StationaryLogDistanceTolerance");
        StationaryLogDistanceTolerance_Offset = CallGetPropertyOffset(StationaryLogDistanceTolerance_NativeProperty);
        IntPtr CachedRepulsionSubsystem_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "CachedRepulsionSubsystem");
        CachedRepulsionSubsystem_Offset = CallGetPropertyOffset(CachedRepulsionSubsystem_NativeProperty);
        SetRepulsionVelocity_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "SetRepulsionVelocity");
        SetRepulsionVelocity_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(SetRepulsionVelocity_NativeFunction);
        SetRepulsionVelocity_InRepulsionVelocity_Offset = CallGetPropertyOffsetFromName(SetRepulsionVelocity_NativeFunction, "InRepulsionVelocity");
        GetEscapeModeTimeRemaining_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "GetEscapeModeTimeRemaining");
        GetEscapeModeTimeRemaining_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(GetEscapeModeTimeRemaining_NativeFunction);
        GetEscapeModeTimeRemaining_ReturnValue_Offset = CallGetPropertyOffsetFromName(GetEscapeModeTimeRemaining_NativeFunction, "ReturnValue");
        GetLastConsumedRepulsionVelocity_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "GetLastConsumedRepulsionVelocity");
        GetLastConsumedRepulsionVelocity_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(GetLastConsumedRepulsionVelocity_NativeFunction);
        GetLastConsumedRepulsionVelocity_ReturnValue_Offset = CallGetPropertyOffsetFromName(GetLastConsumedRepulsionVelocity_NativeFunction, "ReturnValue");
        GetCachedEscapeVelocity_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "GetCachedEscapeVelocity");
        GetCachedEscapeVelocity_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(GetCachedEscapeVelocity_NativeFunction);
        GetCachedEscapeVelocity_ReturnValue_Offset = CallGetPropertyOffsetFromName(GetCachedEscapeVelocity_NativeFunction, "ReturnValue");
        OnOwnerReleased_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "OnOwnerReleased");
        IsEscapeModeActive_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "IsEscapeModeActive");
        IsEscapeModeActive_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(IsEscapeModeActive_NativeFunction);
        IsEscapeModeActive_ReturnValue_Offset = CallGetPropertyOffsetFromName(IsEscapeModeActive_NativeFunction, "ReturnValue");
    }
    
    static int MaxRepulsionVelocityRatio_Offset;
    
    /// <summary>
    /// 排斥力占最大速度的上限比例，防止排斥力完全压过寻路意图
    /// </summary>
    public float MaxRepulsionVelocityRatio
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + MaxRepulsionVelocityRatio_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + MaxRepulsionVelocityRatio_Offset, 0, value);
        }
    }
    
    
    static int OverspeedToleranceRatio_Offset;
    
    /// <summary>
    /// 排斥力叠加后允许的超速比例。
    /// 1.0 = 不允许超速，1.1 = 允许 10% 超速（推荐，确保紧凑空间能被有效推开）。
    /// </summary>
    public float OverspeedToleranceRatio
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + OverspeedToleranceRatio_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + OverspeedToleranceRatio_Offset, 0, value);
        }
    }
    
    
    static int OuterBandInwardClipRatio_Offset;
    
    /// <summary>
    /// 在外层感知带（ObstacleRadius ~ RepulsionRadius）内，
    /// 对朝向站桩单位的速度分量进行的最大裁剪比例。
    /// 0 = 不做外层软裁剪；1 = 靠近 ObstacleRadius 时几乎完全裁掉向内速度。
    /// </summary>
    public float OuterBandInwardClipRatio
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + OuterBandInwardClipRatio_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + OuterBandInwardClipRatio_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeDuration_Offset;
    
    /// <summary>
    /// EscapeMode 的持续时间（秒）。
    /// 当路径意图仍然顶向站桩障碍、而最终速度已接近 0 时，
    /// 短时间内改为使用排斥主导的逃逸速度，避免单位原地僵住。
    /// </summary>
    public float EscapeModeDuration
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + EscapeModeDuration_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + EscapeModeDuration_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeSpeedRatio_Offset;
    
    /// <summary>
    /// 触发 EscapeMode 时的速度上限比例。
    /// 逃逸阶段直接采用排斥主导的速度，但仍需要限幅，避免瞬间过冲。
    /// </summary>
    public float EscapeModeSpeedRatio
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + EscapeModeSpeedRatio_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + EscapeModeSpeedRatio_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeStuckSpeedThreshold_Offset;
    
    /// <summary>
    /// 触发 EscapeMode 的“几乎停住”阈值（cm/s）。
    /// 最终速度低于此值，同时路径请求与排斥对抗时，视为局部卡死。
    /// </summary>
    public float EscapeModeStuckSpeedThreshold
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + EscapeModeStuckSpeedThreshold_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + EscapeModeStuckSpeedThreshold_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeRequestedSpeedThreshold_Offset;
    
    /// <summary>
    /// 触发 EscapeMode 所需的最小路径请求速度（cm/s）。
    /// 避免单位本来就接近停下时误进入逃逸模式。
    /// </summary>
    public float EscapeModeRequestedSpeedThreshold
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + EscapeModeRequestedSpeedThreshold_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + EscapeModeRequestedSpeedThreshold_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeMinRepulsionRatio_Offset;
    
    /// <summary>
    /// 触发 EscapeMode 所需的最小排斥强度占比。
    /// 以 MaxWalkSpeed 为基准，过滤掉微弱排斥造成的正常减速。
    /// </summary>
    public float EscapeModeMinRepulsionRatio
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + EscapeModeMinRepulsionRatio_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + EscapeModeMinRepulsionRatio_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeOppositionDotThreshold_Offset;
    
    /// <summary>
    /// 触发 EscapeMode 的方向对抗阈值。
    /// 请求方向与排斥方向的点积低于此值时，视为“明显对抗”。
    /// </summary>
    public float EscapeModeOppositionDotThreshold
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + EscapeModeOppositionDotThreshold_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + EscapeModeOppositionDotThreshold_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeEnterMargin_Offset;
    
    /// <summary>
    /// 进入 EscapeMode 时允许附加在 NavModifier 半径外的缓冲带（cm）。
    /// 单位只要仍压在这条缓冲带内，就说明局部路径仍有较高概率是错误的。
    /// </summary>
    public float EscapeModeEnterMargin
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + EscapeModeEnterMargin_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + EscapeModeEnterMargin_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeReleaseMargin_Offset;
    
    /// <summary>
    /// 退出 EscapeMode 时使用的释放缓冲带（cm）。
    /// 该值应大于 EnterMargin，形成滞后，避免刚推出一点就立刻恢复旧路径。
    /// </summary>
    public float EscapeModeReleaseMargin
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + EscapeModeReleaseMargin_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + EscapeModeReleaseMargin_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeTriggerFrames_Offset;
    
    /// <summary>
    /// 触发 EscapeMode 所需的连续判定帧数。
    /// 用于过滤单帧速度抖动或瞬时接触导致的误触发。
    /// </summary>
    public int EscapeModeTriggerFrames
    {
        get
        {
            return BlittableMarshaller<int>.FromNative(NativeObject + EscapeModeTriggerFrames_Offset, 0);
        }
        set
        {
            BlittableMarshaller<int>.ToNative(NativeObject + EscapeModeTriggerFrames_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeClearFrames_Offset;
    
    /// <summary>
    /// 退出 EscapeMode 所需的连续清除帧数。
    /// 只有持续离开释放带后，才恢复正常路径驱动。
    /// </summary>
    public int EscapeModeClearFrames
    {
        get
        {
            return BlittableMarshaller<int>.FromNative(NativeObject + EscapeModeClearFrames_Offset, 0);
        }
        set
        {
            BlittableMarshaller<int>.ToNative(NativeObject + EscapeModeClearFrames_Offset, 0, value);
        }
    }
    
    
    static int EscapeModeTangentCarryRatio_Offset;
    
    /// <summary>
    /// EscapeMode 中保留路径切向分量的比例。
    /// 0 = 纯排斥逃逸；1 = 尽可能保留原路径的绕行切向趋势。
    /// </summary>
    public float EscapeModeTangentCarryRatio
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + EscapeModeTangentCarryRatio_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + EscapeModeTangentCarryRatio_Offset, 0, value);
        }
    }
    
    
    static int StationaryLogThreshold_Offset;
    
    /// <summary>
    /// 单位持续静止多久后输出一次诊断日志（秒）。
    /// 用于定位“任务仍在跑，但单位原地不动”的根因。
    /// </summary>
    public float StationaryLogThreshold
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + StationaryLogThreshold_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + StationaryLogThreshold_Offset, 0, value);
        }
    }
    
    
    static int StationaryLogSpeedThreshold_Offset;
    
    /// <summary>
    /// 判定“静止”时使用的二维速度阈值（cm/s）。
    /// 低于此速度且位置几乎不变时，计入静止时长。
    /// </summary>
    public float StationaryLogSpeedThreshold
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + StationaryLogSpeedThreshold_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + StationaryLogSpeedThreshold_Offset, 0, value);
        }
    }
    
    
    static int StationaryLogDistanceTolerance_Offset;
    
    /// <summary>
    /// 判定“静止”时允许的二维位移容差（cm）。
    /// 防止角色轻微抖动导致静止监测被频繁重置。
    /// </summary>
    public float StationaryLogDistanceTolerance
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + StationaryLogDistanceTolerance_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + StationaryLogDistanceTolerance_Offset, 0, value);
        }
    }
    
    
    static int CachedRepulsionSubsystem_Offset;
    
    /// <summary>
    /// 缓存 RepulsionSubsystem 指针，避免每帧 GetSubsystem 查询
    /// </summary>
    private UnrealSharp.TireflySquadNavigation.UTsnStanceRepulsionSubsystem CachedRepulsionSubsystem
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnStanceRepulsionSubsystem>.FromNative(NativeObject + CachedRepulsionSubsystem_Offset, 0);
        }
    }
    
    
    // SetRepulsionVelocity
    static IntPtr SetRepulsionVelocity_NativeFunction;
    static int SetRepulsionVelocity_ParamsSize;
    static int SetRepulsionVelocity_InRepulsionVelocity_Offset;
    
    public UnrealSharp.CoreUObject.FVector RepulsionVelocity
    {
        set
        {
            unsafe
            {
                byte* paramsBufferAllocation = stackalloc byte[SetRepulsionVelocity_ParamsSize];
                nint paramsBuffer = (nint) paramsBufferAllocation;
                BlittableMarshaller<UnrealSharp.CoreUObject.FVector>.ToNative(paramsBuffer + SetRepulsionVelocity_InRepulsionVelocity_Offset, 0, value);
                
                UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, SetRepulsionVelocity_NativeFunction, paramsBuffer, IntPtr.Zero);
                
                
            }
        }
    }
    
    
    // GetEscapeModeTimeRemaining
    static IntPtr GetEscapeModeTimeRemaining_NativeFunction;
    static int GetEscapeModeTimeRemaining_ParamsSize;
    static int GetEscapeModeTimeRemaining_ReturnValue_Offset;
    
    public float EscapeModeTimeRemaining
    {
        get
        {
            unsafe
            {
                byte* paramsBufferAllocation = stackalloc byte[GetEscapeModeTimeRemaining_ParamsSize];
                nint paramsBuffer = (nint) paramsBufferAllocation;
                
                UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, GetEscapeModeTimeRemaining_NativeFunction, paramsBuffer, paramsBuffer + GetEscapeModeTimeRemaining_ReturnValue_Offset);
                
                var returnValue = BlittableMarshaller<float>.FromNative(paramsBuffer + GetEscapeModeTimeRemaining_ReturnValue_Offset, 0);
                
                return returnValue;
            }
        }
    }
    
    
    // GetLastConsumedRepulsionVelocity
    static IntPtr GetLastConsumedRepulsionVelocity_NativeFunction;
    static int GetLastConsumedRepulsionVelocity_ParamsSize;
    static int GetLastConsumedRepulsionVelocity_ReturnValue_Offset;
    
    public UnrealSharp.CoreUObject.FVector LastConsumedRepulsionVelocity
    {
        get
        {
            unsafe
            {
                byte* paramsBufferAllocation = stackalloc byte[GetLastConsumedRepulsionVelocity_ParamsSize];
                nint paramsBuffer = (nint) paramsBufferAllocation;
                
                UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, GetLastConsumedRepulsionVelocity_NativeFunction, paramsBuffer, paramsBuffer + GetLastConsumedRepulsionVelocity_ReturnValue_Offset);
                
                var returnValue = BlittableMarshaller<UnrealSharp.CoreUObject.FVector>.FromNative(paramsBuffer + GetLastConsumedRepulsionVelocity_ReturnValue_Offset, 0);
                
                return returnValue;
            }
        }
    }
    
    
    // GetCachedEscapeVelocity
    static IntPtr GetCachedEscapeVelocity_NativeFunction;
    static int GetCachedEscapeVelocity_ParamsSize;
    static int GetCachedEscapeVelocity_ReturnValue_Offset;
    
    public UnrealSharp.CoreUObject.FVector CachedEscapeVelocity
    {
        get
        {
            unsafe
            {
                byte* paramsBufferAllocation = stackalloc byte[GetCachedEscapeVelocity_ParamsSize];
                nint paramsBuffer = (nint) paramsBufferAllocation;
                
                UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, GetCachedEscapeVelocity_NativeFunction, paramsBuffer, paramsBuffer + GetCachedEscapeVelocity_ReturnValue_Offset);
                
                var returnValue = BlittableMarshaller<UnrealSharp.CoreUObject.FVector>.FromNative(paramsBuffer + GetCachedEscapeVelocity_ReturnValue_Offset, 0);
                
                return returnValue;
            }
        }
    }
    
    // OnOwnerReleased
    static IntPtr OnOwnerReleased_NativeFunction;
    
    /// <summary>
    /// 对象池释放回调。宿主项目在将 Owner 返回对象池时调用此方法。
    /// 等价于 EndPlay 中的清理逻辑，但不依赖 Actor 实际销毁。
    /// </summary>
    [UFunction, GeneratedType("OnOwnerReleased", "UnrealSharp.TireflySquadNavigation.OnOwnerReleased")]
    public void OnOwnerReleased()
    {
        unsafe
        {
            UObjectExporter.CallInvokeNativeFunction(NativeObject, OnOwnerReleased_NativeFunction, IntPtr.Zero, IntPtr.Zero);
        }
    }
    
    // IsEscapeModeActive
    static IntPtr IsEscapeModeActive_NativeFunction;
    static int IsEscapeModeActive_ParamsSize;
    static int IsEscapeModeActive_ReturnValue_Offset;
    
    /// <summary>
    /// 当前是否处于 EscapeMode。
    /// 主要供 BT 任务与调试日志读取。
    /// </summary>
    [UFunction, GeneratedType("IsEscapeModeActive", "UnrealSharp.TireflySquadNavigation.IsEscapeModeActive")]
    public bool IsEscapeModeActive()
    {
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[IsEscapeModeActive_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            
            UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, IsEscapeModeActive_NativeFunction, paramsBuffer, paramsBuffer + IsEscapeModeActive_ReturnValue_Offset);
            
            var returnValue = BoolMarshaller.FromNative(paramsBuffer + IsEscapeModeActive_ReturnValue_Offset, 0);
            
            return returnValue;
        }
    }
    
    
}