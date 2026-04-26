// TsnTestSpawner.h
// 测试用批量生成器 —— 支持多种阵型生成棋子

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TsnTestTypes.h"
#include "TsnTestSpawner.generated.h"

class ATsnTestChessPiece;
class UBehaviorTree;
class UBlackboardData;

/**
 * 测试用批量生成器。
 *
 * 按指定阵型（Line / Column / Grid / Circle）批量生成 ATsnTestChessPiece，
 * 配置行为树和黑板，可选设置初始攻击目标。
 * 所有属性均在编辑器 Details 面板暴露。
 */
UCLASS()
class ATsnTestSpawner : public AActor
{
	GENERATED_BODY()

public:
	ATsnTestSpawner();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** 手动触发生成 */
	UFUNCTION(BlueprintCallable, Category = "TsnTest|Spawner")
	void SpawnAll();

	// --- 配置 ---

	/** 要生成的棋子类（或蓝图子类） */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
	TSubclassOf<ATsnTestChessPiece> ChessPieceClass;

	/** 生成数量 */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner", meta = (ClampMin = "1"))
	int32 SpawnCount = 4;

	/** 阵型类型 */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
	ETsnTestFormation Formation = ETsnTestFormation::Line;

	/** 阵型间距 (cm) */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner", meta = (ClampMin = "50.0"))
	float FormationSpacing = 200.f;

	/** 是否 BeginPlay 自动生成 */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
	bool bAutoSpawnOnBeginPlay = true;

	/** 分配给 AI 的行为树 */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;

	/** 分配给 AI 的黑板 */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
	TObjectPtr<UBlackboardData> BlackboardAsset;

	/** 初始攻击目标（可选） */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
	TObjectPtr<AActor> InitialTarget;

	/** 生成的单位会被设为此 TeamID */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Spawner")
	int32 SpawnTeamID = 0;

private:
	/** 已生成的棋子列表 */
	UPROPERTY()
	TArray<TWeakObjectPtr<ATsnTestChessPiece>> SpawnedUnits;

	/**
	 * 根据阵型和索引计算生成偏移量。
	 *
	 * @param Index 当前单位索引
	 * @return 相对于 Spawner 位置的偏移
	 */
	FVector CalculateFormationOffset(int32 Index) const;

	/** 统计存活单位数量 */
	int32 GetAliveCount() const;
};
