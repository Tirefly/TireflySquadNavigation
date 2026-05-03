using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// IsInEngagementRange
/// </summary>
[UStruct, GeneratedType("TsnStateTreeCondition_IsInEngagementRangeInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeCondition_IsInEngagementRangeInstanceData")]
public partial struct FTsnStateTreeCondition_IsInEngagementRangeInstanceData : MarshalledStruct<FTsnStateTreeCondition_IsInEngagementRangeInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeCondition_IsInEngagementRangeInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeCondition_IsInEngagementRangeInstanceData");
    // DistanceToTarget2D
    
    static int DistanceToTarget2D_Offset;
    public float DistanceToTarget2D;
    
    // EngagementRange
    
    static int EngagementRange_Offset;
    public float EngagementRange;
    
    // Invert
    
    static int bInvert_Offset;
    public bool Invert;
    
    public FTsnStateTreeCondition_IsInEngagementRangeInstanceData(float DistanceToTarget2D, float EngagementRange, bool Invert)
    {
        this.DistanceToTarget2D = DistanceToTarget2D;
        this.EngagementRange = EngagementRange;
        this.Invert = Invert;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeCondition_IsInEngagementRangeInstanceData()
    {
        IntPtr DistanceToTarget2D_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "DistanceToTarget2D");
        DistanceToTarget2D_Offset = CallGetPropertyOffset(DistanceToTarget2D_NativeProperty);
        IntPtr EngagementRange_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EngagementRange");
        EngagementRange_Offset = CallGetPropertyOffset(EngagementRange_NativeProperty);
        IntPtr bInvert_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bInvert");
        bInvert_Offset = CallGetPropertyOffset(bInvert_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeCondition_IsInEngagementRangeInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            DistanceToTarget2D = BlittableMarshaller<float>.FromNative(InNativeStruct + DistanceToTarget2D_Offset, 0);
            EngagementRange = BlittableMarshaller<float>.FromNative(InNativeStruct + EngagementRange_Offset, 0);
            Invert = BoolMarshaller.FromNative(InNativeStruct + bInvert_Offset, 0);
        }
    }
    
    public static FTsnStateTreeCondition_IsInEngagementRangeInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeCondition_IsInEngagementRangeInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            BlittableMarshaller<float>.ToNative(buffer + DistanceToTarget2D_Offset, 0, DistanceToTarget2D);
            BlittableMarshaller<float>.ToNative(buffer + EngagementRange_Offset, 0, EngagementRange);
            BoolMarshaller.ToNative(buffer + bInvert_Offset, 0, Invert);
        }
    }
}

public static class FTsnStateTreeCondition_IsInEngagementRangeInstanceDataMarshaller
{
    public static FTsnStateTreeCondition_IsInEngagementRangeInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeCondition_IsInEngagementRangeInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeCondition_IsInEngagementRangeInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeCondition_IsInEngagementRangeInstanceData.NativeDataSize;
    }
}