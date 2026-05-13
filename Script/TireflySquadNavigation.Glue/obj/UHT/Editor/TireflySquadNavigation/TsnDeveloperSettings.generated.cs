using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// TSN 插件级默认几何设置。
/// 
/// 这层设置只负责提供插件自己的默认 obstacle / nav modifier / repulsion 预算，
/// 不负责推导宿主项目的玩法尺度。
/// </summary>
[UClass, GeneratedType("TsnDeveloperSettings", "UnrealSharp.TireflySquadNavigation.TsnDeveloperSettings")]
public partial class UTsnDeveloperSettings : UnrealSharp.DeveloperSettings.UDeveloperSettings
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(UTsnDeveloperSettings).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnDeveloperSettings");
    static UTsnDeveloperSettings()
    {
        IntPtr bDefaultUseNavModifier_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bDefaultUseNavModifier");
        bDefaultUseNavModifier_Offset = CallGetPropertyOffset(bDefaultUseNavModifier_NativeProperty);
        IntPtr DefaultNavModifierMode_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "DefaultNavModifierMode");
        DefaultNavModifierMode_Offset = CallGetPropertyOffset(DefaultNavModifierMode_NativeProperty);
        IntPtr DefaultObstacleRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "DefaultObstacleRadius");
        DefaultObstacleRadius_Offset = CallGetPropertyOffset(DefaultObstacleRadius_NativeProperty);
        IntPtr DefaultNavModifierExtraRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "DefaultNavModifierExtraRadius");
        DefaultNavModifierExtraRadius_Offset = CallGetPropertyOffset(DefaultNavModifierExtraRadius_NativeProperty);
        IntPtr DefaultRepulsionRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "DefaultRepulsionRadius");
        DefaultRepulsionRadius_Offset = CallGetPropertyOffset(DefaultRepulsionRadius_NativeProperty);
        IntPtr DefaultRepulsionStrength_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "DefaultRepulsionStrength");
        DefaultRepulsionStrength_Offset = CallGetPropertyOffset(DefaultRepulsionStrength_NativeProperty);
        IntPtr DefaultNavModifierDeactivationDelay_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "DefaultNavModifierDeactivationDelay");
        DefaultNavModifierDeactivationDelay_Offset = CallGetPropertyOffset(DefaultNavModifierDeactivationDelay_NativeProperty);
    }
    
    static int bDefaultUseNavModifier_Offset;
    
    /// <summary>
    /// 默认是否启用 NavModifier。
    /// </summary>
    protected bool DefaultUseNavModifier
    {
        get
        {
            return BoolMarshaller.FromNative(NativeObject + bDefaultUseNavModifier_Offset, 0);
        }
        set
        {
            BoolMarshaller.ToNative(NativeObject + bDefaultUseNavModifier_Offset, 0, value);
        }
    }
    
    
    static int DefaultNavModifierMode_Offset;
    
    /// <summary>
    /// 默认 NavModifier 区域策略。
    /// </summary>
    protected UnrealSharp.TireflySquadNavigation.ETsnNavModifierMode DefaultNavModifierMode
    {
        get
        {
            return EnumMarshaller<UnrealSharp.TireflySquadNavigation.ETsnNavModifierMode>.FromNative(NativeObject + DefaultNavModifierMode_Offset, 0);
        }
        set
        {
            EnumMarshaller<UnrealSharp.TireflySquadNavigation.ETsnNavModifierMode>.ToNative(NativeObject + DefaultNavModifierMode_Offset, 0, value);
        }
    }
    
    
    static int DefaultObstacleRadius_Offset;
    
    /// <summary>
    /// 默认站姿障碍半径。
    /// </summary>
    protected float DefaultObstacleRadius
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + DefaultObstacleRadius_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + DefaultObstacleRadius_Offset, 0, value);
        }
    }
    
    
    static int DefaultNavModifierExtraRadius_Offset;
    
    /// <summary>
    /// 默认导航扩张半径。
    /// </summary>
    protected float DefaultNavModifierExtraRadius
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + DefaultNavModifierExtraRadius_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + DefaultNavModifierExtraRadius_Offset, 0, value);
        }
    }
    
    
    static int DefaultRepulsionRadius_Offset;
    
    /// <summary>
    /// 默认排斥感知半径。
    /// </summary>
    protected float DefaultRepulsionRadius
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + DefaultRepulsionRadius_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + DefaultRepulsionRadius_Offset, 0, value);
        }
    }
    
    
    static int DefaultRepulsionStrength_Offset;
    
    /// <summary>
    /// 默认排斥力强度。
    /// </summary>
    protected float DefaultRepulsionStrength
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + DefaultRepulsionStrength_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + DefaultRepulsionStrength_Offset, 0, value);
        }
    }
    
    
    static int DefaultNavModifierDeactivationDelay_Offset;
    
    /// <summary>
    /// 默认 NavModifier 延迟关闭时间。
    /// </summary>
    protected float DefaultNavModifierDeactivationDelay
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + DefaultNavModifierDeactivationDelay_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + DefaultNavModifierDeactivationDelay_Offset, 0, value);
        }
    }
    
    
}