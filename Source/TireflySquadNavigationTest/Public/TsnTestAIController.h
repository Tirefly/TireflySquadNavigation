// TsnTestAIController.h
// 测试用 AI 控制器 —— 构造函数中集成 CrowdFollowing

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TsnTestAIController.generated.h"

/**
 * 测试用 AI 控制器。
 *
 * 在构造函数中将默认 PathFollowingComponent 替换为 UCrowdFollowingComponent，
 * 以启用 DetourCrowd 群体避障。在 OnPossess 时关闭 CMC 内置 RVO Avoidance，
 * 防止双重避障冲突。
 */
UCLASS()
class ATsnTestAIController : public AAIController
{
	GENERATED_BODY()

public:
	ATsnTestAIController(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnPossess(APawn* InPawn) override;
};
