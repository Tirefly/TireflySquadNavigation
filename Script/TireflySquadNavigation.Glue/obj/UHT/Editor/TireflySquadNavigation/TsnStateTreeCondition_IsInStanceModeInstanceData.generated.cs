using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// IsInStanceMode
/// </summary>
[UStruct, GeneratedType("TsnStateTreeCondition_IsInStanceModeInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeCondition_IsInStanceModeInstanceData")]
public partial struct FTsnStateTreeCondition_IsInStanceModeInstanceData : MarshalledStruct<FTsnStateTreeCondition_IsInStanceModeInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeCondition_IsInStanceModeInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeCondition_IsInStanceModeInstanceData");
    // AIController
    
    static int AIController_Offset;
    /// <summary>
    /// Context-绑定 AIController；运行时直接查询 Pawn 的 TsnStanceObstacleComponent
    /// </summary>
    public UnrealSharp.AIModule.AAIController AIController;
    
    // Invert
    
    static int bInvert_Offset;
    public bool Invert;
    
    public FTsnStateTreeCondition_IsInStanceModeInstanceData(UnrealSharp.AIModule.AAIController AIController, bool Invert)
    {
        this.AIController = AIController;
        this.Invert = Invert;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeCondition_IsInStanceModeInstanceData()
    {
        IntPtr AIController_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AIController");
        AIController_Offset = CallGetPropertyOffset(AIController_NativeProperty);
        IntPtr bInvert_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bInvert");
        bInvert_Offset = CallGetPropertyOffset(bInvert_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeCondition_IsInStanceModeInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            AIController = ObjectMarshaller<UnrealSharp.AIModule.AAIController>.FromNative(InNativeStruct + AIController_Offset, 0);
            Invert = BoolMarshaller.FromNative(InNativeStruct + bInvert_Offset, 0);
        }
    }
    
    public static FTsnStateTreeCondition_IsInStanceModeInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeCondition_IsInStanceModeInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            ObjectMarshaller<UnrealSharp.AIModule.AAIController>.ToNative(buffer + AIController_Offset, 0, AIController);
            BoolMarshaller.ToNative(buffer + bInvert_Offset, 0, Invert);
        }
    }
}

public static class FTsnStateTreeCondition_IsInStanceModeInstanceDataMarshaller
{
    public static FTsnStateTreeCondition_IsInStanceModeInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeCondition_IsInStanceModeInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeCondition_IsInStanceModeInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeCondition_IsInStanceModeInstanceData.NativeDataSize;
    }
}