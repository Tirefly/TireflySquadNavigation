// TsnTestChessPiece.h
// 测试用 AI 棋子 —— 实现 ITsnTacticalUnit 接口的 ACharacter 子类

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/ITsnTacticalUnit.h"
#include "TsnTestChessPiece.generated.h"

class UTsnTacticalMovementComponent;
class UTsnStanceObstacleComponent;
class UTsnEngagementSlotComponent;
class UTsnUnitSeparationComponent;

/**
 * 测试用 AI 棋子角色。
 *
 * 继承 ACharacter 并实现 ITsnTacticalUnit 接口，
 * 挂载全部插件组件，用于演示和测试四层架构。
 * 自带胶囊体可视化（阵营颜色）和 DrawDebug 阶段信息。
 */
UCLASS()
class ATsnTestChessPiece : public ACharacter, public ITsnTacticalUnit
{
	GENERATED_BODY()

public:
	ATsnTestChessPiece(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// --- ITsnTacticalUnit ---

	/** 返回该单位的交战距离 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TireflySquadNavigation")
	float GetEngagementRange() const;
	virtual float GetEngagementRange_Implementation() const override;

	/** 返回该单位是否处于站姿模式 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TireflySquadNavigation")
	bool IsInStanceMode() const;
	virtual bool IsInStanceMode_Implementation() const override;

	// --- 属性 ---

	/** 交战距离（cm），不同兵种蓝图子类中覆写 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TsnTest")
	float EngagementRange = 150.f;

	/** 阵营 ID（0=红方, 1=蓝方） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TsnTest")
	int32 TeamID = 0;

	/** 是否绘制阶段信息（头顶文字 + 连线） */
	UPROPERTY(EditAnywhere, Category = "TsnTest|Debug")
	bool bDrawDebugSlotInfo = true;

	// --- 插件组件 ---

	/** 战术移动组件（排斥力注入） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TsnTest|Components")
	TObjectPtr<UTsnTacticalMovementComponent> TacticalMovementComp;

	/** 站桩障碍物组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TsnTest|Components")
	TObjectPtr<UTsnStanceObstacleComponent> StanceObstacleComp;

	/** 交战槽位组件（使棋子可作为被攻击目标） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TsnTest|Components")
	TObjectPtr<UTsnEngagementSlotComponent> EngagementSlotComp;

	/** 单位分离力组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TsnTest|Components")
	TObjectPtr<UTsnUnitSeparationComponent> UnitSeparationComp;

private:
	/** 推断当前战斗阶段的文字标签 */
	FString GetPhaseDebugString() const;

	/** 获取阶段对应的调试颜色 */
	FColor GetPhaseDebugColor() const;
};
