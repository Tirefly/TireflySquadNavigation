using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// IsInPreEngagementRange
/// </summary>
[UStruct, GeneratedType("TsnStateTreeCondition_IsInPreEngagementRangeInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeCondition_IsInPreEngagementRangeInstanceData")]
public partial struct FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData : MarshalledStruct<FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeCondition_IsInPreEngagementRangeInstanceData");
    // DistanceToTarget2D
    
    static int DistanceToTarget2D_Offset;
    public float DistanceToTarget2D;
    
    // PreEngagementRadius
    
    static int PreEngagementRadius_Offset;
    public float PreEngagementRadius;
    
    // Invert
    
    static int bInvert_Offset;
    public bool Invert;
    
    public FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData(float DistanceToTarget2D, float PreEngagementRadius, bool Invert)
    {
        this.DistanceToTarget2D = DistanceToTarget2D;
        this.PreEngagementRadius = PreEngagementRadius;
        this.Invert = Invert;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData()
    {
        IntPtr DistanceToTarget2D_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "DistanceToTarget2D");
        DistanceToTarget2D_Offset = CallGetPropertyOffset(DistanceToTarget2D_NativeProperty);
        IntPtr PreEngagementRadius_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "PreEngagementRadius");
        PreEngagementRadius_Offset = CallGetPropertyOffset(PreEngagementRadius_NativeProperty);
        IntPtr bInvert_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bInvert");
        bInvert_Offset = CallGetPropertyOffset(bInvert_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            DistanceToTarget2D = BlittableMarshaller<float>.FromNative(InNativeStruct + DistanceToTarget2D_Offset, 0);
            PreEngagementRadius = BlittableMarshaller<float>.FromNative(InNativeStruct + PreEngagementRadius_Offset, 0);
            Invert = BoolMarshaller.FromNative(InNativeStruct + bInvert_Offset, 0);
        }
    }
    
    public static FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            BlittableMarshaller<float>.ToNative(buffer + DistanceToTarget2D_Offset, 0, DistanceToTarget2D);
            BlittableMarshaller<float>.ToNative(buffer + PreEngagementRadius_Offset, 0, PreEngagementRadius);
            BoolMarshaller.ToNative(buffer + bInvert_Offset, 0, Invert);
        }
    }
}

public static class FTsnStateTreeCondition_IsInPreEngagementRangeInstanceDataMarshaller
{
    public static FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeCondition_IsInPreEngagementRangeInstanceData.NativeDataSize;
    }
}