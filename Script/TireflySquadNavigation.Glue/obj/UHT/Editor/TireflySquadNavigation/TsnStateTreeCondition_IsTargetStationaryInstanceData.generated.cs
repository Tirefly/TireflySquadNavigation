using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// IsTargetStationary
/// </summary>
[UStruct, GeneratedType("TsnStateTreeCondition_IsTargetStationaryInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeCondition_IsTargetStationaryInstanceData")]
public partial struct FTsnStateTreeCondition_IsTargetStationaryInstanceData : MarshalledStruct<FTsnStateTreeCondition_IsTargetStationaryInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeCondition_IsTargetStationaryInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeCondition_IsTargetStationaryInstanceData");
    // IsTargetStationary
    
    static int bIsTargetStationary_Offset;
    /// <summary>
    /// 通常绑定自 TargetMotion 评估器输出
    /// </summary>
    public bool IsTargetStationary;
    
    // Invert
    
    static int bInvert_Offset;
    public bool Invert;
    
    public FTsnStateTreeCondition_IsTargetStationaryInstanceData(bool IsTargetStationary, bool Invert)
    {
        this.IsTargetStationary = IsTargetStationary;
        this.Invert = Invert;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeCondition_IsTargetStationaryInstanceData()
    {
        IntPtr bIsTargetStationary_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bIsTargetStationary");
        bIsTargetStationary_Offset = CallGetPropertyOffset(bIsTargetStationary_NativeProperty);
        IntPtr bInvert_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bInvert");
        bInvert_Offset = CallGetPropertyOffset(bInvert_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeCondition_IsTargetStationaryInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            IsTargetStationary = BoolMarshaller.FromNative(InNativeStruct + bIsTargetStationary_Offset, 0);
            Invert = BoolMarshaller.FromNative(InNativeStruct + bInvert_Offset, 0);
        }
    }
    
    public static FTsnStateTreeCondition_IsTargetStationaryInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeCondition_IsTargetStationaryInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            BoolMarshaller.ToNative(buffer + bIsTargetStationary_Offset, 0, IsTargetStationary);
            BoolMarshaller.ToNative(buffer + bInvert_Offset, 0, Invert);
        }
    }
}

public static class FTsnStateTreeCondition_IsTargetStationaryInstanceDataMarshaller
{
    public static FTsnStateTreeCondition_IsTargetStationaryInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeCondition_IsTargetStationaryInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeCondition_IsTargetStationaryInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeCondition_IsTargetStationaryInstanceData.NativeDataSize;
    }
}