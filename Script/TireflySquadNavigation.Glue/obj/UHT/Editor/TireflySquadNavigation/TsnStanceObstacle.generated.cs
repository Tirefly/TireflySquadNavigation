using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigation;

/// <summary>
/// 站姿单位注册数据。
/// 结构体不缓存位置，只缓存 Actor 弱引用和注册时确定的参数值。
/// 调用方每帧通过 Obs.Unit->GetActorLocation() 动态读取当前位置。
/// </summary>
[UStruct, GeneratedType("TsnStanceObstacle", "UnrealSharp.TireflySquadNavigation.TsnStanceObstacle")]
public partial struct FTsnStanceObstacle : MarshalledStruct<FTsnStanceObstacle>
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(FTsnStanceObstacle).GetAssemblyName(), "UnrealSharp.TireflySquadNavigation", "TsnStanceObstacle");
    
    public static IntPtr GetNativeClassPtr() => NativeClassPtr;
    public static readonly int NativeDataSize;
    public static int GetNativeDataSize() => NativeDataSize;
    static FTsnStanceObstacle()
    {
        NativeDataSize = UScriptStructExporter.CallGetNativeStructSize(NativeClassPtr);
    }
    
    
    [System.Diagnostics.CodeAnalysis.SetsRequiredMembers]
    public FTsnStanceObstacle(IntPtr InNativeStruct)
    {
        unsafe
        {
        }
    }
    
    public static FTsnStanceObstacle FromNative(IntPtr buffer) => new FTsnStanceObstacle(buffer);
    
    public void ToNative(IntPtr buffer)
    {
        unsafe
        {
        }
    }
}

public static class FTsnStanceObstacleMarshaller
{
    public static FTsnStanceObstacle FromNative(IntPtr nativeBuffer, int arrayIndex)
    {
        return new FTsnStanceObstacle(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static void ToNative(IntPtr nativeBuffer, int arrayIndex, FTsnStanceObstacle obj)
    {
        obj.ToNative(nativeBuffer + (arrayIndex * GetNativeDataSize()));
    }
    
    public static int GetNativeDataSize()
    {
        return FTsnStanceObstacle.NativeDataSize;
    }
}