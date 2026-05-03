using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 站桩障碍物状态管理组件 —— 挂载在每个 AI 战斗单位身上。
/// 
/// 管理单位在"移动 Agent"和"站桩障碍物"之间的导航身份切换：
/// - Moving：正常 DetourCrowd Agent，参与群体寻路和避障
/// - Stance：Crowd 切换为 ObstacleOnly；NavModifier 开启（开关式，非创建/销毁）；
///           注册到 TsnStanceRepulsionSubsystem
/// </summary>
[UClass, GeneratedType("TsnStanceObstacleComponent", "UnrealSharp.TireflySquadNavigation.TsnStanceObstacleComponent")]
public partial class UTsnStanceObstacleComponent : UnrealSharp.Engine.UActorComponent
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnStanceObstacleComponent).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStanceObstacleComponent");
    static UTsnStanceObstacleComponent()
    {
        IntPtr ObstacleRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ObstacleRadius");
        ObstacleRadius_Offset = CallGetPropertyOffset(ObstacleRadius_NativeProperty);
        IntPtr bUseNavModifier_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bUseNavModifier");
        bUseNavModifier_Offset = CallGetPropertyOffset(bUseNavModifier_NativeProperty);
        IntPtr NavModifierMode_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "NavModifierMode");
        NavModifierMode_Offset = CallGetPropertyOffset(NavModifierMode_NativeProperty);
        IntPtr NavModifierExtraRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "NavModifierExtraRadius");
        NavModifierExtraRadius_Offset = CallGetPropertyOffset(NavModifierExtraRadius_NativeProperty);
        IntPtr RepulsionRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "RepulsionRadius");
        RepulsionRadius_Offset = CallGetPropertyOffset(RepulsionRadius_NativeProperty);
        IntPtr RepulsionStrength_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "RepulsionStrength");
        RepulsionStrength_Offset = CallGetPropertyOffset(RepulsionStrength_NativeProperty);
        IntPtr NavModifierDeactivationDelay_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "NavModifierDeactivationDelay");
        NavModifierDeactivationDelay_Offset = CallGetPropertyOffset(NavModifierDeactivationDelay_NativeProperty);
        IntPtr bDrawDebugObstacle_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bDrawDebugObstacle");
        bDrawDebugObstacle_Offset = CallGetPropertyOffset(bDrawDebugObstacle_NativeProperty);
        IntPtr NavModifierComp_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "NavModifierComp");
        NavModifierComp_Offset = CallGetPropertyOffset(NavModifierComp_NativeProperty);
        IntPtr CrowdFollowingComp_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "CrowdFollowingComp");
        CrowdFollowingComp_Offset = CallGetPropertyOffset(CrowdFollowingComp_NativeProperty);
        GetMobilityStance_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "GetMobilityStance");
        GetMobilityStance_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(GetMobilityStance_NativeFunction);
        GetMobilityStance_ReturnValue_Offset = CallGetPropertyOffsetFromName(GetMobilityStance_NativeFunction, "ReturnValue");
        EnterStanceMode_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "EnterStanceMode");
        ExitStanceMode_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "ExitStanceMode");
        UpdateStanceUnitParams_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "UpdateStanceUnitParams");
        OnOwnerReleased_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "OnOwnerReleased");
    }
    
    static int ObstacleRadius_Offset;
    
    /// <summary>
    /// NavModifier 障碍半径（同时作为排斥力双阶段分界线）。
    /// 建议设为角色碰撞胶囊半径 × 1.2~1.5。
    /// </summary>
    public float ObstacleRadius
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + ObstacleRadius_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + ObstacleRadius_Offset, 0, value);
        }
    }
    
    
    static int bUseNavModifier_Offset;
    
    /// <summary>
    /// 是否使用 NavModifier 影响路径规划
    /// </summary>
    public bool UseNavModifier
    {
        get
        {
            return BoolMarshaller.FromNative(NativeObject + bUseNavModifier_Offset, 0);
        }
        set
        {
            BoolMarshaller.ToNative(NativeObject + bUseNavModifier_Offset, 0, value);
        }
    }
    
    
    static int NavModifierMode_Offset;
    
    /// <summary>
    /// NavModifier 区域策略
    /// </summary>
    public UnrealSharp.TireflySquadNavigation.ETsnNavModifierMode NavModifierMode
    {
        get
        {
            return EnumMarshaller<UnrealSharp.TireflySquadNavigation.ETsnNavModifierMode>.FromNative(NativeObject + NavModifierMode_Offset, 0);
        }
        set
        {
            EnumMarshaller<UnrealSharp.TireflySquadNavigation.ETsnNavModifierMode>.ToNative(NativeObject + NavModifierMode_Offset, 0, value);
        }
    }
    
    
    static int NavModifierExtraRadius_Offset;
    
    /// <summary>
    /// 额外放大的导航影响半径。
    /// 仅作用于 NavModifier 的面积，用于让路径规划更早把站桩单位视为绕行障碍，
    /// 不改变排斥力内层边界本身。
    /// </summary>
    public float NavModifierExtraRadius
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + NavModifierExtraRadius_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + NavModifierExtraRadius_Offset, 0, value);
        }
    }
    
    
    static int RepulsionRadius_Offset;
    
    /// <summary>
    /// 排斥力作用半径（必须大于 ObstacleRadius）。
    /// 推荐落在 ObstacleRadius × 1.5~2.5。
    /// </summary>
    public float RepulsionRadius
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + RepulsionRadius_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + RepulsionRadius_Offset, 0, value);
        }
    }
    
    
    static int RepulsionStrength_Offset;
    
    /// <summary>
    /// 排斥力强度
    /// </summary>
    public float RepulsionStrength
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + RepulsionStrength_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + RepulsionStrength_Offset, 0, value);
        }
    }
    
    
    static int NavModifierDeactivationDelay_Offset;
    
    /// <summary>
    /// 退出站姿模式时，延迟多久关闭 NavModifier
    /// </summary>
    public float NavModifierDeactivationDelay
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + NavModifierDeactivationDelay_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + NavModifierDeactivationDelay_Offset, 0, value);
        }
    }
    
    
    static int bDrawDebugObstacle_Offset;
    
    /// <summary>
    /// 是否绘制当前单位的 ObstacleRadius、NavModifier 半径和 RepulsionRadius。
    /// 适合直接在 PIE 中观察哪些单位仍把窄缝当成可通行。
    /// </summary>
    public bool DrawDebugObstacle
    {
        get
        {
            return BoolMarshaller.FromNative(NativeObject + bDrawDebugObstacle_Offset, 0);
        }
        set
        {
            BoolMarshaller.ToNative(NativeObject + bDrawDebugObstacle_Offset, 0, value);
        }
    }
    
    
    static int NavModifierComp_Offset;
    
    private UnrealSharp.NavigationSystem.UNavModifierComponent NavModifierComp
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.NavigationSystem.UNavModifierComponent>.FromNative(NativeObject + NavModifierComp_Offset, 0);
        }
    }
    
    
    static int CrowdFollowingComp_Offset;
    
    private UnrealSharp.AIModule.UCrowdFollowingComponent CrowdFollowingComp
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.AIModule.UCrowdFollowingComponent>.FromNative(NativeObject + CrowdFollowingComp_Offset, 0);
        }
    }
    
    
    // GetMobilityStance
    static IntPtr GetMobilityStance_NativeFunction;
    static int GetMobilityStance_ParamsSize;
    static int GetMobilityStance_ReturnValue_Offset;
    
    public UnrealSharp.TireflySquadNavigation.ETsnMobilityStance MobilityStance
    {
        get
        {
            unsafe
            {
                byte* paramsBufferAllocation = stackalloc byte[GetMobilityStance_ParamsSize];
                nint paramsBuffer = (nint) paramsBufferAllocation;
                
                UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, GetMobilityStance_NativeFunction, paramsBuffer, paramsBuffer + GetMobilityStance_ReturnValue_Offset);
                
                var returnValue = EnumMarshaller<UnrealSharp.TireflySquadNavigation.ETsnMobilityStance>.FromNative(paramsBuffer + GetMobilityStance_ReturnValue_Offset, 0);
                
                return returnValue;
            }
        }
    }
    
    // EnterStanceMode
    static IntPtr EnterStanceMode_NativeFunction;
    
    /// <summary>
    /// 进入站姿模式（幂等）
    /// </summary>
    [UFunction, GeneratedType("EnterStanceMode", "UnrealSharp.TireflySquadNavigation.EnterStanceMode")]
    public void EnterStanceMode()
    {
        unsafe
        {
            UObjectExporter.CallInvokeNativeFunction(NativeObject, EnterStanceMode_NativeFunction, IntPtr.Zero, IntPtr.Zero);
        }
    }
    
    // ExitStanceMode
    static IntPtr ExitStanceMode_NativeFunction;
    
    /// <summary>
    /// 退出站姿模式，恢复移动 Agent 身份（幂等）
    /// </summary>
    [UFunction, GeneratedType("ExitStanceMode", "UnrealSharp.TireflySquadNavigation.ExitStanceMode")]
    public void ExitStanceMode()
    {
        unsafe
        {
            UObjectExporter.CallInvokeNativeFunction(NativeObject, ExitStanceMode_NativeFunction, IntPtr.Zero, IntPtr.Zero);
        }
    }
    
    // UpdateStanceUnitParams
    static IntPtr UpdateStanceUnitParams_NativeFunction;
    
    /// <summary>
    /// 运行时更新障碍物参数并同步到 NavModifier 和 RepulsionSubsystem。
    /// 宿主项目修改 ObstacleRadius / RepulsionRadius / RepulsionStrength 后调用此方法。
    /// </summary>
    [UFunction, GeneratedType("UpdateStanceUnitParams", "UnrealSharp.TireflySquadNavigation.UpdateStanceUnitParams")]
    public void UpdateStanceUnitParams()
    {
        unsafe
        {
            UObjectExporter.CallInvokeNativeFunction(NativeObject, UpdateStanceUnitParams_NativeFunction, IntPtr.Zero, IntPtr.Zero);
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
    
    
}