using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigationTest;

/// <summary>
/// 测试用 AI 棋子角色。
/// 
/// 继承 ACharacter 并实现 ITsnTacticalUnit 接口，
/// 挂载全部插件组件，用于演示和测试四层架构。
/// 自带胶囊体可视化（阵营颜色）和 DrawDebug 阶段信息。
/// </summary>
[UClass, GeneratedType("TsnTestChessPiece", "UnrealSharp.TireflySquadNavigationTest.TsnTestChessPiece")]
public partial class ATsnTestChessPiece : UnrealSharp.Engine.ACharacter, UnrealSharp.TireflySquadNavigation.ITsnTacticalUnit
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(ATsnTestChessPiece).GetAssemblyName(), "UnrealSharp.TireflySquadNavigationTest", "TsnTestChessPiece");
    static ATsnTestChessPiece()
    {
        IntPtr TeamID_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TeamID");
        TeamID_Offset = CallGetPropertyOffset(TeamID_NativeProperty);
        IntPtr bDrawDebugSlotInfo_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bDrawDebugSlotInfo");
        bDrawDebugSlotInfo_Offset = CallGetPropertyOffset(bDrawDebugSlotInfo_NativeProperty);
        IntPtr TacticalMovementComp_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "TacticalMovementComp");
        TacticalMovementComp_Offset = CallGetPropertyOffset(TacticalMovementComp_NativeProperty);
        IntPtr StanceObstacleComp_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "StanceObstacleComp");
        StanceObstacleComp_Offset = CallGetPropertyOffset(StanceObstacleComp_NativeProperty);
        IntPtr EngagementSlotComp_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EngagementSlotComp");
        EngagementSlotComp_Offset = CallGetPropertyOffset(EngagementSlotComp_NativeProperty);
        IntPtr UnitSeparationComp_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "UnitSeparationComp");
        UnitSeparationComp_Offset = CallGetPropertyOffset(UnitSeparationComp_NativeProperty);
        IntPtr EngagementRange_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "EngagementRange");
        EngagementRange_Offset = CallGetPropertyOffset(EngagementRange_NativeProperty);
        GetEngagementRange_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "GetEngagementRange");
        GetEngagementRange_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(GetEngagementRange_NativeFunction);
        GetEngagementRange_ReturnValue_Offset = CallGetPropertyOffsetFromName(GetEngagementRange_NativeFunction, "ReturnValue");
        IsInStanceMode_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "IsInStanceMode");
        IsInStanceMode_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(IsInStanceMode_NativeFunction);
        IsInStanceMode_ReturnValue_Offset = CallGetPropertyOffsetFromName(IsInStanceMode_NativeFunction, "ReturnValue");
        GetEngagementRange_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "GetEngagementRange");
        GetEngagementRange_ParamsSize = UFunctionExporter.CallGetNativeFunctionParamsSize(GetEngagementRange_NativeFunction);
        GetEngagementRange_ReturnValue_Offset = CallGetPropertyOffsetFromName(GetEngagementRange_NativeFunction, "ReturnValue");
    }
    
    static int TeamID_Offset;
    
    /// <summary>
    /// 阵营 ID（0=红方, 1=蓝方）
    /// </summary>
    public int TeamID
    {
        get
        {
            return BlittableMarshaller<int>.FromNative(NativeObject + TeamID_Offset, 0);
        }
        set
        {
            BlittableMarshaller<int>.ToNative(NativeObject + TeamID_Offset, 0, value);
        }
    }
    
    
    static int bDrawDebugSlotInfo_Offset;
    
    /// <summary>
    /// 是否绘制阶段信息（头顶文字 + 连线）
    /// </summary>
    public bool DrawDebugSlotInfo
    {
        get
        {
            return BoolMarshaller.FromNative(NativeObject + bDrawDebugSlotInfo_Offset, 0);
        }
        set
        {
            BoolMarshaller.ToNative(NativeObject + bDrawDebugSlotInfo_Offset, 0, value);
        }
    }
    
    
    static int TacticalMovementComp_Offset;
    
    /// <summary>
    /// 战术移动组件（排斥力注入）
    /// </summary>
    public UnrealSharp.TireflySquadNavigation.UTsnTacticalMovementComponent TacticalMovementComp
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnTacticalMovementComponent>.FromNative(NativeObject + TacticalMovementComp_Offset, 0);
        }
        set
        {
            ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnTacticalMovementComponent>.ToNative(NativeObject + TacticalMovementComp_Offset, 0, value);
        }
    }
    
    
    static int StanceObstacleComp_Offset;
    
    /// <summary>
    /// 站桩障碍物组件
    /// </summary>
    public UnrealSharp.TireflySquadNavigation.UTsnStanceObstacleComponent StanceObstacleComp
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnStanceObstacleComponent>.FromNative(NativeObject + StanceObstacleComp_Offset, 0);
        }
        set
        {
            ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnStanceObstacleComponent>.ToNative(NativeObject + StanceObstacleComp_Offset, 0, value);
        }
    }
    
    
    static int EngagementSlotComp_Offset;
    
    /// <summary>
    /// 交战槽位组件（使棋子可作为被攻击目标）
    /// </summary>
    public UnrealSharp.TireflySquadNavigation.UTsnEngagementSlotComponent EngagementSlotComp
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnEngagementSlotComponent>.FromNative(NativeObject + EngagementSlotComp_Offset, 0);
        }
        set
        {
            ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnEngagementSlotComponent>.ToNative(NativeObject + EngagementSlotComp_Offset, 0, value);
        }
    }
    
    
    static int UnitSeparationComp_Offset;
    
    /// <summary>
    /// 单位分离力组件
    /// </summary>
    public UnrealSharp.TireflySquadNavigation.UTsnUnitSeparationComponent UnitSeparationComp
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnUnitSeparationComponent>.FromNative(NativeObject + UnitSeparationComp_Offset, 0);
        }
        set
        {
            ObjectMarshaller<UnrealSharp.TireflySquadNavigation.UTsnUnitSeparationComponent>.ToNative(NativeObject + UnitSeparationComp_Offset, 0, value);
        }
    }
    
    
    // GetEngagementRange
    static IntPtr GetEngagementRange_NativeFunction;
    IntPtr GetEngagementRange_InstanceFunction;
    static int GetEngagementRange_ParamsSize;
    static int GetEngagementRange_ReturnValue_Offset;
    static int EngagementRange_Offset;
    static IntPtr EngagementRange_NativeProperty;
    static int EngagementRange_Size;
    
    /// <summary>
    /// 交战距离（cm），不同兵种蓝图子类中覆写
    /// </summary>
    public float EngagementRange
    {
        get
        {
            unsafe
            {
                byte* paramsBufferAllocation = stackalloc byte[GetEngagementRange_ParamsSize];
                nint paramsBuffer = (nint) paramsBufferAllocation;
                
                UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, GetEngagementRange_NativeFunction, paramsBuffer, paramsBuffer + GetEngagementRange_ReturnValue_Offset);
                
                var returnValue = BlittableMarshaller<float>.FromNative(paramsBuffer + GetEngagementRange_ReturnValue_Offset, 0);
                
                return returnValue;
            }
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + EngagementRange_Offset, 0, value);
        }
    }
    
    // IsInStanceMode
    static IntPtr IsInStanceMode_NativeFunction;
    IntPtr IsInStanceMode_InstanceFunction;
    static int IsInStanceMode_ParamsSize;
    static int IsInStanceMode_ReturnValue_Offset;
    
    /// <summary>
    /// 返回该单位是否处于站姿模式
    /// </summary>
    [UFunction(FunctionFlags.BlueprintEvent), GeneratedType("IsInStanceMode", "UnrealSharp.TireflySquadNavigationTest.IsInStanceMode")]
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
    
    [UFunction, GeneratedType("IsInStanceMode", "UnrealSharp.TireflySquadNavigationTest.IsInStanceMode")]
    protected virtual bool IsInStanceMode_Implementation()
    {
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[IsInStanceMode_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            
            UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, IsInStanceMode_NativeFunction, paramsBuffer, paramsBuffer + IsInStanceMode_ReturnValue_Offset);
            
            var returnValue = BoolMarshaller.FromNative(paramsBuffer + IsInStanceMode_ReturnValue_Offset, 0);
            
            return returnValue;
        }
    }
    void Invoke_IsInStanceMode(IntPtr buffer, IntPtr returnBuffer)
    {
        unsafe
        {
            bool returnValue = IsInStanceMode_Implementation();
            BoolMarshaller.ToNative(returnBuffer + 0, 0, returnValue);
        }
    }
    
    
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
    
    [UFunction, GeneratedType("GetEngagementRange", "UnrealSharp.TireflySquadNavigation.GetEngagementRange")]
    protected virtual float GetEngagementRange_Implementation()
    {
        unsafe
        {
            byte* paramsBufferAllocation = stackalloc byte[GetEngagementRange_ParamsSize];
            nint paramsBuffer = (nint) paramsBufferAllocation;
            
            UObjectExporter.CallInvokeNativeFunctionOutParms(NativeObject, GetEngagementRange_NativeFunction, paramsBuffer, paramsBuffer + GetEngagementRange_ReturnValue_Offset);
            
            var returnValue = BlittableMarshaller<float>.FromNative(paramsBuffer + GetEngagementRange_ReturnValue_Offset, 0);
            
            return returnValue;
        }
    }
    void Invoke_GetEngagementRange(IntPtr buffer, IntPtr returnBuffer)
    {
        unsafe
        {
            float returnValue = GetEngagementRange_Implementation();
            BlittableMarshaller<float>.ToNative(returnBuffer + 0, 0, returnValue);
        }
    }
    
    
}