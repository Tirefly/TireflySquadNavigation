using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 槽位信息：挂在目标上的本地极坐标（角度 + 半径），不是持久缓存的世界坐标。
/// </summary>
[UStruct, GeneratedType("TsnEngagementSlotInfo", "UnrealSharp.TireflySquadNavigation.TsnEngagementSlotInfo")]
public partial struct FTsnEngagementSlotInfo : MarshalledStruct<FTsnEngagementSlotInfo>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnEngagementSlotInfo).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnEngagementSlotInfo");
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnEngagementSlotInfo()
    {
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnEngagementSlotInfo(IntPtr InNativeStruct)
    {
        unsafe
        {
        }
    }
    
    public static FTsnEngagementSlotInfo FromNative(IntPtr buffer) => new FTsnEngagementSlotInfo(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
        }
    }
}

public static class FTsnEngagementSlotInfoMarshaller
{
    public static FTsnEngagementSlotInfo FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnEngagementSlotInfo(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnEngagementSlotInfo obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnEngagementSlotInfo.NativeDataSize;
    }
}