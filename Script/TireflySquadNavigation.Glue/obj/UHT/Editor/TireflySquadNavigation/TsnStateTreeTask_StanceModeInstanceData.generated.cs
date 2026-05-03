using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

[UStruct, GeneratedType("TsnStateTreeTask_StanceModeInstanceData", "UnrealSharp.TireflySquadNavigation.TsnStateTreeTask_StanceModeInstanceData")]
public partial struct FTsnStateTreeTask_StanceModeInstanceData : MarshalledStruct<FTsnStateTreeTask_StanceModeInstanceData>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStateTreeTask_StanceModeInstanceData).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStateTreeTask_StanceModeInstanceData");
    // AIController
    
    static int AIController_Offset;
    /// <summary>
    /// Context-绑定 AIController，由 AI Schema 自动注入
    /// </summary>
    public UnrealSharp.AIModule.AAIController AIController;
    
    public FTsnStateTreeTask_StanceModeInstanceData(UnrealSharp.AIModule.AAIController AIController)
    {
        this.AIController = AIController;
    }
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStateTreeTask_StanceModeInstanceData()
    {
        IntPtr AIController_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "AIController");
        AIController_Offset = CallGetPropertyOffset(AIController_NativeProperty);
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStateTreeTask_StanceModeInstanceData(IntPtr InNativeStruct)
    {
        unsafe
        {
            AIController = ObjectMarshaller<UnrealSharp.AIModule.AAIController>.FromNative(InNativeStruct + AIController_Offset, 0);
        }
    }
    
    public static FTsnStateTreeTask_StanceModeInstanceData FromNative(IntPtr buffer) => new FTsnStateTreeTask_StanceModeInstanceData(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
            ObjectMarshaller<UnrealSharp.AIModule.AAIController>.ToNative(buffer + AIController_Offset, 0, AIController);
        }
    }
}

public static class FTsnStateTreeTask_StanceModeInstanceDataMarshaller
{
    public static FTsnStateTreeTask_StanceModeInstanceData FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStateTreeTask_StanceModeInstanceData(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStateTreeTask_StanceModeInstanceData obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStateTreeTask_StanceModeInstanceData.NativeDataSize;
    }
}