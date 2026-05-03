using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 单位分离力组件 —— 仅处理移动中单位之间的微量分离，防止 Moving↔Moving 重叠。
/// 
/// 作为 DetourCrowd 群体避障的补充层，通过 AddMovementInput 注入，
/// 与路径跟随的输入自然融合，在 CalcVelocity 中统一处理。
/// 
/// Moving↔Stance 的 keep-out 由 TsnStanceRepulsionSubsystem 独占负责；
/// 若附近存在站桩单位，本组件必须裁剪任何"朝站桩单位内侧"的分离分量，
/// 只保留切向或远离站桩单位的软协调输入。
/// 这一裁剪规则与 Penetration Guard 自洽：排斥力被设计为柔性（Guard 必然存在），
/// 因此软通道必须主动过滤朝内分量，避免产生注定被 Guard 裁掉的无效输入。
/// 
/// 本组件为可选组件：若宿主项目的 DetourCrowd 群体避障已能提供足够的单位间距，
/// 可不挂载此组件，其余三层防线（NavModifier、RepulsionSubsystem、PenetrationGuard）零影响。
/// </summary>
[UClass, GeneratedType("TsnUnitSeparationComponent", "UnrealSharp.TireflySquadNavigation.TsnUnitSeparationComponent")]
public partial class UTsnUnitSeparationComponent : UnrealSharp.Engine.UActorComponent
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnUnitSeparationComponent).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnUnitSeparationComponent");
    static UTsnUnitSeparationComponent()
    {
        IntPtr SeparationRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SeparationRadius");
        SeparationRadius_Offset = CallGetPropertyOffset(SeparationRadius_NativeProperty);
        IntPtr SeparationStrength_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SeparationStrength");
        SeparationStrength_Offset = CallGetPropertyOffset(SeparationStrength_NativeProperty);
        IntPtr bDrawDebugSeparation_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bDrawDebugSeparation");
        bDrawDebugSeparation_Offset = CallGetPropertyOffset(bDrawDebugSeparation_NativeProperty);
        IntPtr CachedRepulsionSubsystem_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "CachedRepulsionSubsystem");
        CachedRepulsionSubsystem_Offset = CallGetPropertyOffset(CachedRepulsionSubsystem_NativeProperty);
        OnOwnerReleased_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "OnOwnerReleased");
    }
    
    static int SeparationRadius_Offset;
    
    /// <summary>
    /// 分离力感知半径（约碰撞半径 2~3 倍）
    /// </summary>
    public float SeparationRadius
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + SeparationRadius_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + SeparationRadius_Offset, 0, value);
        }
    }
    
    
    static int SeparationStrength_Offset;
    
    /// <summary>
    /// 分离力强度（作为 AddMovementInput 的 ScaleValue）。
    /// 0.3 → 柔和；0.5 → 标准；0.8 → 明显推开。
    /// </summary>
    public float SeparationStrength
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + SeparationStrength_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + SeparationStrength_Offset, 0, value);
        }
    }
    
    
    static int bDrawDebugSeparation_Offset;
    
    /// <summary>
    /// 是否绘制分离力调试信息：分离力方向 + Stance 约束裁剪后的最终方向（箭头）。
    /// 在 Shipping 配置中此属性仍存在，但实际绘制代码不会编译进去。
    /// </summary>
    public bool DrawDebugSeparation
    {
        get
        {
            return BoolMarshaller.FromNative(NativeObject + bDrawDebugSeparation_Offset, 0);
        }
        set
        {
            BoolMarshaller.ToNative(NativeObject + bDrawDebugSeparation_Offset, 0, value);
        }
    }
    
    
    static int CachedRepulsionSubsystem_Offset;
    
    /// <summary>
    /// 缓存 TsnStanceRepulsionSubsystem 指针，供 TickComponent 遍历注册列表。
    /// BeginPlay 时赋值，EndPlay / OnOwnerReleased 时清空。
    /// </summary>
    private UnrealSharp.TireflySquadNavigation.UTsnStanceRepulsionSubsystem CachedRepulsionSubsystem
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnStanceRepulsionSubsystem>.FromNative(NativeObject + CachedRepulsionSubsystem_Offset, 0);
        }
    }
    
    // OnOwnerReleased
    static IntPtr OnOwnerReleased_NativeFunction;
    
    /// <summary>
    /// 对象池回收时由宿主调用——停止 Tick 并清空运行时状态，
    /// 配合 Deactivate() 使用，避免对象池中休眠组件仍参与计算。
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