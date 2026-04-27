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
 * 以启用 DetourCrowd 群体避障。
 * CMC 内置 RVO Avoidance 由 UTsnTacticalMovementComponent 构造函数统一关闭，
 * AIController 不再重复承担该职责。
 */
UCLASS()
class ATsnTestAIController : public AAIController
{
	GENERATED_BODY()

public:
	ATsnTestAIController(const FObjectInitializer& ObjectInitializer);
};
