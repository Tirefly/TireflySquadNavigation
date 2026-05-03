using UnrealSharp.Attributes;
using UnrealSharp.Core;
using UnrealSharp.Core.Attributes;
using UnrealSharp.Interop;
using UnrealSharp.Core.Marshallers;
using static UnrealSharp.Interop.FPropertyExporter;

namespace UnrealSharp.TireflySquadNavigationTest;

/// <summary>
/// 测试用批量生成器。
/// 
/// 按指定阵型（Line / Column / Grid / Circle）批量生成 ATsnTestChessPiece，
/// 配置行为树和黑板，可选设置初始攻击目标。
/// 所有属性均在编辑器 Details 面板暴露。
/// </summary>
[UClass, GeneratedType("TsnTestSpawner", "UnrealSharp.TireflySquadNavigationTest.TsnTestSpawner")]
public partial class ATsnTestSpawner : UnrealSharp.Engine.AActor
{
    static readonly IntPtr NativeClassPtr = UCoreUObjectExporter.CallGetType(typeof(ATsnTestSpawner).GetAssemblyName(), "UnrealSharp.TireflySquadNavigationTest", "TsnTestSpawner");
    static ATsnTestSpawner()
    {
        IntPtr ChessPieceClass_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "ChessPieceClass");
        ChessPieceClass_Offset = CallGetPropertyOffset(ChessPieceClass_NativeProperty);
        IntPtr SpawnCount_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SpawnCount");
        SpawnCount_Offset = CallGetPropertyOffset(SpawnCount_NativeProperty);
        IntPtr Formation_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "Formation");
        Formation_Offset = CallGetPropertyOffset(Formation_NativeProperty);
        IntPtr FormationSpacing_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "FormationSpacing");
        FormationSpacing_Offset = CallGetPropertyOffset(FormationSpacing_NativeProperty);
        IntPtr bAutoSpawnOnBeginPlay_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "bAutoSpawnOnBeginPlay");
        bAutoSpawnOnBeginPlay_Offset = CallGetPropertyOffset(bAutoSpawnOnBeginPlay_NativeProperty);
        IntPtr BehaviorTreeAsset_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "BehaviorTreeAsset");
        BehaviorTreeAsset_Offset = CallGetPropertyOffset(BehaviorTreeAsset_NativeProperty);
        IntPtr BlackboardAsset_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "BlackboardAsset");
        BlackboardAsset_Offset = CallGetPropertyOffset(BlackboardAsset_NativeProperty);
        IntPtr InitialTarget_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "InitialTarget");
        InitialTarget_Offset = CallGetPropertyOffset(InitialTarget_NativeProperty);
        IntPtr SpawnTeamID_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SpawnTeamID");
        SpawnTeamID_Offset = CallGetPropertyOffset(SpawnTeamID_NativeProperty);
        SpawnedUnits_NativeProperty = CallGetNativePropertyFromName(NativeClassPtr, "SpawnedUnits");
        SpawnedUnits_Offset = CallGetPropertyOffset(SpawnedUnits_NativeProperty);
        SpawnAll_NativeFunction = UClassExporter.CallGetNativeFunctionFromClassAndName(NativeClassPtr, "SpawnAll");
    }
    
    static int ChessPieceClass_Offset;
    
    /// <summary>
    /// 要生成的棋子类（或蓝图子类）
    /// </summary>
    public TSubclassOf<UnrealSharp.TireflySquadNavigationTest.ATsnTestChessPiece> ChessPieceClass
    {
        get
        {
            return SubclassOfMarshaller<UnrealSharp.TireflySquadNavigationTest.ATsnTestChessPiece>.FromNative(NativeObject + ChessPieceClass_Offset, 0);
        }
        set
        {
            SubclassOfMarshaller<UnrealSharp.TireflySquadNavigationTest.ATsnTestChessPiece>.ToNative(NativeObject + ChessPieceClass_Offset, 0, value);
        }
    }
    
    
    static int SpawnCount_Offset;
    
    /// <summary>
    /// 生成数量
    /// </summary>
    public int SpawnCount
    {
        get
        {
            return BlittableMarshaller<int>.FromNative(NativeObject + SpawnCount_Offset, 0);
        }
        set
        {
            BlittableMarshaller<int>.ToNative(NativeObject + SpawnCount_Offset, 0, value);
        }
    }
    
    
    static int Formation_Offset;
    
    /// <summary>
    /// 阵型类型
    /// </summary>
    public UnrealSharp.TireflySquadNavigationTest.ETsnTestFormation Formation
    {
        get
        {
            return EnumMarshaller<UnrealSharp.TireflySquadNavigationTest.ETsnTestFormation>.FromNative(NativeObject + Formation_Offset, 0);
        }
        set
        {
            EnumMarshaller<UnrealSharp.TireflySquadNavigationTest.ETsnTestFormation>.ToNative(NativeObject + Formation_Offset, 0, value);
        }
    }
    
    
    static int FormationSpacing_Offset;
    
    /// <summary>
    /// 阵型间距 (cm)
    /// </summary>
    public float FormationSpacing
    {
        get
        {
            return BlittableMarshaller<float>.FromNative(NativeObject + FormationSpacing_Offset, 0);
        }
        set
        {
            BlittableMarshaller<float>.ToNative(NativeObject + FormationSpacing_Offset, 0, value);
        }
    }
    
    
    static int bAutoSpawnOnBeginPlay_Offset;
    
    /// <summary>
    /// 是否 BeginPlay 自动生成
    /// </summary>
    public bool AutoSpawnOnBeginPlay
    {
        get
        {
            return BoolMarshaller.FromNative(NativeObject + bAutoSpawnOnBeginPlay_Offset, 0);
        }
        set
        {
            BoolMarshaller.ToNative(NativeObject + bAutoSpawnOnBeginPlay_Offset, 0, value);
        }
    }
    
    
    static int BehaviorTreeAsset_Offset;
    
    /// <summary>
    /// 分配给 AI 的行为树
    /// </summary>
    public UnrealSharp.AIModule.UBehaviorTree BehaviorTreeAsset
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.AIModule.UBehaviorTree>.FromNative(NativeObject + BehaviorTreeAsset_Offset, 0);
        }
        set
        {
            ObjectMarshaller<UnrealSharp.AIModule.UBehaviorTree>.ToNative(NativeObject + BehaviorTreeAsset_Offset, 0, value);
        }
    }
    
    
    static int BlackboardAsset_Offset;
    
    /// <summary>
    /// 分配给 AI 的黑板
    /// </summary>
    public UnrealSharp.AIModule.UBlackboardData BlackboardAsset
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.AIModule.UBlackboardData>.FromNative(NativeObject + BlackboardAsset_Offset, 0);
        }
        set
        {
            ObjectMarshaller<UnrealSharp.AIModule.UBlackboardData>.ToNative(NativeObject + BlackboardAsset_Offset, 0, value);
        }
    }
    
    
    static int InitialTarget_Offset;
    
    /// <summary>
    /// 初始攻击目标（可选）
    /// </summary>
    public UnrealSharp.Engine.AActor InitialTarget
    {
        get
        {
            return ObjectMarshaller<UnrealSharp.Engine.AActor>.FromNative(NativeObject + InitialTarget_Offset, 0);
        }
        set
        {
            ObjectMarshaller<UnrealSharp.Engine.AActor>.ToNative(NativeObject + InitialTarget_Offset, 0, value);
        }
    }
    
    
    static int SpawnTeamID_Offset;
    
    /// <summary>
    /// 生成的单位会被设为此 TeamID
    /// </summary>
    public int SpawnTeamID
    {
        get
        {
            return BlittableMarshaller<int>.FromNative(NativeObject + SpawnTeamID_Offset, 0);
        }
        set
        {
            BlittableMarshaller<int>.ToNative(NativeObject + SpawnTeamID_Offset, 0, value);
        }
    }
    
    
    static int SpawnedUnits_Offset;
    static IntPtr SpawnedUnits_NativeProperty;
    ArrayMarshaller<TWeakObjectPtr<UnrealSharp.TireflySquadNavigationTest.ATsnTestChessPiece>> SpawnedUnits_Marshaller = null;
    
    /// <summary>
    /// 已生成的棋子列表
    /// </summary>
    private System.Collections.Generic.IList<TWeakObjectPtr<UnrealSharp.TireflySquadNavigationTest.ATsnTestChessPiece>> SpawnedUnits
    {
        get
        {
            SpawnedUnits_Marshaller ??= new ArrayMarshaller<TWeakObjectPtr<UnrealSharp.TireflySquadNavigationTest.ATsnTestChessPiece>>(SpawnedUnits_NativeProperty, BlittableMarshaller<TWeakObjectPtr<UnrealSharp.TireflySquadNavigationTest.ATsnTestChessPiece>>.ToNative, BlittableMarshaller<TWeakObjectPtr<UnrealSharp.TireflySquadNavigationTest.ATsnTestChessPiece>>.FromNative);
            return SpawnedUnits_Marshaller.FromNative(NativeObject + SpawnedUnits_Offset, 0);
        }
    }
    
    // SpawnAll
    static IntPtr SpawnAll_NativeFunction;
    
    /// <summary>
    /// 手动触发生成
    /// </summary>
    [UFunction, GeneratedType("SpawnAll", "UnrealSharp.TireflySquadNavigationTest.SpawnAll")]
    public void SpawnAll()
    {
        unsafe
        {
            UObjectExporter.CallInvokeNativeFunction(NativeObject, SpawnAll_NativeFunction, IntPtr.Zero, IntPtr.Zero);
        }
    }
    
    
}