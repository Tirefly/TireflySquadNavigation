using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 站姿单位排斥力子系统 —— WorldSubsystem（自动创建）。
/// 
/// 职责：
/// 1. 维护所有处于 Stance 模式的单位列表
/// 2. 每帧遍历所有移动中的战斗单位
/// 3. 对接近站桩单位的移动单位施加双阶段排斥力
/// 4. 排斥力通过 TsnTacticalMovementComponent::SetRepulsionVelocity 合法注入
/// 5. Moving↔Stance 的 keep-out 语义仅由本子系统负责
/// </summary>
[UClass, GeneratedType("TsnStanceRepulsionSubsystem", "UnrealSharp.TireflySquadNavigation.TsnStanceRepulsionSubsystem")]
public partial class UTsnStanceRepulsionSubsystem : UnrealSharp.Engine.UTickableWorldSubsystem
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnStanceRepulsionSubsystem).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStanceRepulsionSubsystem");
    static UTsnStanceRepulsionSubsystem()
    {
        IntPtr OuterRepulsionRatio_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "OuterRepulsionRatio");
        OuterRepulsionRatio_Offset = CallGetPropertyOffset(OuterRepulsionRatio_NativeProperty);
        IntPtr TangentBlendRatio_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TangentBlendRatio");
        TangentBlendRatio_Offset = CallGetPropertyOffset(TangentBlendRatio_NativeProperty);
        IntPtr TangentSideDeadZone_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TangentSideDeadZone");
        TangentSideDeadZone_Offset = CallGetPropertyOffset(TangentSideDeadZone_NativeProperty);
        IntPtr bDrawDebugRepulsion_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bDrawDebugRepulsion");
        bDrawDebugRepulsion_Offset = CallGetPropertyOffset(bDrawDebugRepulsion_NativeProperty);
        GetStanceUnitCount_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "GetStanceUnitCount");
        GetStanceUnitCount_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(GetStanceUnitCount_NativeFunction);
        GetStanceUnitCount_ReturnValue_Offset = CallGetPropertyOffsetFromName(GetStanceUnitCount_NativeFunction, "ReturnValue");
    }
    
    static int OuterRepulsionRatio_Offset;
    
    /// <summary>
    /// 外层排斥力阶段系数（默认 0.1 = RepulsionStrength 的 10%）
    /// </summary>
    public float OuterRepulsionRatio
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + OuterRepulsionRatio_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + OuterRepulsionRatio_Offset, 0, value);
        }
    }
    
    
    static int TangentBlendRatio_Offset;
    
    /// <summary>
    /// 排斥力方向中切线引导占比
    /// </summary>
    public float TangentBlendRatio
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + TangentBlendRatio_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + TangentBlendRatio_Offset, 0, value);
        }
    }
    
    
    static int TangentSideDeadZone_Offset;
    
    /// <summary>
    /// 切线选边死区。
    /// 当站桩单位相对移动意图几乎正前方时，左右绕行没有稳定优先级；
    /// 若 Side 落入此死区，则退回纯径向排斥，避免切线方向在两侧来回翻转。
    /// </summary>
    public float TangentSideDeadZone
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + TangentSideDeadZone_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + TangentSideDeadZone_Offset, 0, value);
        }
    }
    
    
    static int bDrawDebugRepulsion_Offset;
    
    /// <summary>
    /// 是否绘制排斥力调试信息
    /// </summary>
    public bool DrawDebugRepulsion
    {
        get
        {
            return BoolMarshaller.FromNative(NativeObject + bDrawDebugRepulsion_Offset, 0);
        }
        set
        {
            BoolMarshaller.ToNative(NativeObject + bDrawDebugRepulsion_Offset, 0, value);
        }
    }
    
    
    // GetStanceUnitCount
    static IntPtr GetStanceUnitCount_NativeFunction;
    static int GetStanceUnitCount_ParamsSize;
    static int GetStanceUnitCount_ReturnValue_Offset;
    
    public int StanceUnitCount
    {
        get
        {
            unsafe
            {
                byte* paramsBufferAllocation = stackalloc byte[GetStanceUnitCount_ParamsSize];
                nint paramsBuffer = (nint) paramsBufferAllocation;
                
                UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, GetStanceUnitCount_NativeFunction, paramsBuffer, paramsBuffer + GetStanceUnitCount_ReturnValue_Offset);
                
                var returnValue = BlittableMarshaller<int>.FromNative(paramsBuffer + GetStanceUnitCount_ReturnValue_Offset, 0);
                
                return returnValue;
            }
        }
    }
    
    
}