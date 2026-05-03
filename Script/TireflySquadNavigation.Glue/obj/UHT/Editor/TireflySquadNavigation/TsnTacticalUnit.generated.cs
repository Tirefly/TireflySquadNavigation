using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

[UInterface, GeneratedType("TsnTacticalUnit", "UnrealSharp.TireflySquadNavigation.TsnTacticalUnit")]
public partial interface ITsnTacticalUnit
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(ITsnTacticalUnit).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnTacticalUnit");
    
    static ITsnTacticalUnit Wrap(UnrealSharp.CoreUObject.UObject obj)
    {
        return new ITsnTacticalUnitWrapper(obj);
    }
    /// <summary>
    /// 返回该单位的交战距离（攻击/施法/射程距离）。
    /// 插件用此值决定 TsnEngagementSlotComponent 的环半径，
    /// 以及 BT 节点中"进入交战距离"的判断阈值。
    /// 
    /// 
    /// </summary>
    [UFunction(FunctionFlags.BlueprintEvent), GeneratedType("GetEngagementRange", "UnrealSharp.TireflySquadNavigation.GetEngagementRange")]
    public float GetEngagementRange();
    /// <summary>
    /// 返回该单位当前是否处于站姿模式（Stance Mode）。
    /// 插件子系统在遍历场景中的战斗单位时，
    /// 通过此接口判断是否需要对其应用排斥力。
    /// 
    /// 
    /// </summary>
    [UFunction(FunctionFlags.BlueprintEvent), GeneratedType("IsInStanceMode", "UnrealSharp.TireflySquadNavigation.IsInStanceMode")]
    public bool IsInStanceMode();
}

internal sealed class ITsnTacticalUnitWrapper : ITsnTacticalUnit, UnrealSharp.CoreUObject.IScriptInterface
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(ITsnTacticalUnit).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnTacticalUnit");
    public UnrealSharp.CoreUObject.UObject Object { get; }
    private IntPtr NativeObject => Object.NativeObject;
    
    internal ITsnTacticalUnitWrapper(UnrealSharp.CoreUObject.UObject obj)
    {
        Object = obj;
    }
    static ITsnTacticalUnitWrapper()
    {
        GetEngagementRange_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "GetEngagementRange");
        GetEngagementRange_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(GetEngagementRange_NativeFunction);
        GetEngagementRange_ReturnValue_Offset = CallGetPropertyOffsetFromName(GetEngagementRange_NativeFunction, "ReturnValue");
        IsInStanceMode_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "IsInStanceMode");
        IsInStanceMode_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(IsInStanceMode_NativeFunction);
        IsInStanceMode_ReturnValue_Offset = CallGetPropertyOffsetFromName(IsInStanceMode_NativeFunction, "ReturnValue");
    }
    // GetEngagementRange
    static IntPtr GetEngagementRange_NativeFunction;
    IntPtr GetEngagementRange_InstanceFunction;
    static int GetEngagementRange_ParamsSize;
    static int GetEngagementRange_ReturnValue_Offset;
    
    /// <summary>
    /// 返回该单位的交战距离（攻击/施法/射程距离）。
    /// 插件用此值决定 TsnEngagementSlotComponent 的环半径，
    /// 以及 BT 节点中"进入交战距离"的判断阈值。
    /// 
    /// 
    /// </summary>
    [UFunction(FunctionFlags.BlueprintEvent), GeneratedType("GetEngagementRange", "UnrealSharp.TireflySquadNavigation.GetEngagementRange")]
    public float GetEngagementRange()
    {
        if (GetEngagementRange_InstanceFunction == IntPtr.Zero)
        {
            GetEngagementRange_InstanceFunction = UClassExporter.CallGetNativeFunctionFromInstanceAndName(NativeObject, "GetEngagementRange");
        }
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[GetEngagementRange_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            
            UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, GetEngagementRange_InstanceFunction, paramsBuffer, paramsBuffer + GetEngagementRange_ReturnValue_Offset);
            
            var returnValue = BlittableMarshaller<float>.FromNative(paramsBuffer + GetEngagementRange_ReturnValue_Offset, 0);
            
            return returnValue;
        }
    }
    
    // IsInStanceMode
    static IntPtr IsInStanceMode_NativeFunction;
    IntPtr IsInStanceMode_InstanceFunction;
    static int IsInStanceMode_ParamsSize;
    static int IsInStanceMode_ReturnValue_Offset;
    
    /// <summary>
    /// 返回该单位当前是否处于站姿模式（Stance Mode）。
    /// 插件子系统在遍历场景中的战斗单位时，
    /// 通过此接口判断是否需要对其应用排斥力。
    /// 
    /// 
    /// </summary>
    [UFunction(FunctionFlags.BlueprintEvent), GeneratedType("IsInStanceMode", "UnrealSharp.TireflySquadNavigation.IsInStanceMode")]
    public bool IsInStanceMode()
    {
        if (IsInStanceMode_InstanceFunction == IntPtr.Zero)
        {
            IsInStanceMode_InstanceFunction = UClassExporter.CallGetNativeFunctionFromInstanceAndName(NativeObject, "IsInStanceMode");
        }
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[IsInStanceMode_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            
            UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, IsInStanceMode_InstanceFunction, paramsBuffer, paramsBuffer + IsInStanceMode_ReturnValue_Offset);
            
            var returnValue = BoolMarshaller.FromNative(paramsBuffer + IsInStanceMode_ReturnValue_Offset, 0);
            
            return returnValue;
        }
    }
    
}

public static class ITsnTacticalUnitMarshaller
{
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, ITsnTacticalUnit obj)
    {
        UnrealSharp.CoreUObject.ScriptInterfaceMarshaller<ITsnTacticalUnit>.ToNative(nativeBuffer, arrayIndex, obj);
    }
    
    public static ITsnTacticalUnit FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return UnrealSharp.CoreUObject.ScriptInterfaceMarshaller<ITsnTacticalUnit>.FromNative(nativeBuffer, arrayIndex);
    }
}