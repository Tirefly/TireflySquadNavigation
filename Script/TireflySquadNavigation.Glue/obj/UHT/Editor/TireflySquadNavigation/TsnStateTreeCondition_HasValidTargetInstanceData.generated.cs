using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// HasValidTarget
/// </summary>
[UStruct, GeneratedType("TsnStateTreeCondition_HasValidTargetInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeCondition_HasValidTargetInstanceData")]
public partial struct FTsnStateTreeCondition_HasValidTargetInstanceData : MarshalledStruct<FTsnStateTreeCondition_HasValidTargetInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeCondition_HasValidTargetInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeCondition_HasValidTargetInstanceData");
    // HasValidTarget
    
    static int bHasValidTarget_Offset;
    /// <summary>
    /// 通常绑定自 CombatContext 评估器输出
    /// </summary>
    public bool HasValidTarget;
    
    // Invert
    
    static int bInvert_Offset;
    public bool Invert;
    
    public FTsnStateTreeCondition_HasValidTargetInstanceData(bool HasValidTarget, bool Invert)
    {
        this.HasValidTarget = HasValidTarget;
        this.Invert = Invert;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeCondition_HasValidTargetInstanceData()
    {
        IntPtr bHasValidTarget_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bHasValidTarget");
        bHasValidTarget_Offset = CallGetPropertyOffset(bHasValidTarget_NativeProperty);
        IntPtr bInvert_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bInvert");
        bInvert_Offset = CallGetPropertyOffset(bInvert_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeCondition_HasValidTargetInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            HasValidTarget = BoolMarshaller.FromNative(InNativeStruct + bHasValidTarget_Offset, 0);
            Invert = BoolMarshaller.FromNative(InNativeStruct + bInvert_Offset, 0);
        }
    }
    
    public static FTsnStateTreeCondition_HasValidTargetInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeCondition_HasValidTargetInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            BoolMarshaller.ToNative(buffer + bHasValidTarget_Offset, 0, HasValidTarget);
            BoolMarshaller.ToNative(buffer + bInvert_Offset, 0, Invert);
        }
    }
}

public static class FTsnStateTreeCondition_HasValidTargetInstanceDataMarshaller
{
    public static FTsnStateTreeCondition_HasValidTargetInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeCondition_HasValidTargetInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeCondition_HasValidTargetInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeCondition_HasValidTargetInstanceData.NativeDataSize;
    }
}