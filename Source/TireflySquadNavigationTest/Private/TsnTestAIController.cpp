// TsnTestAIController.cpp
// 测试用 AI 控制器 —— CrowdFollowing 集成

#include "TsnTestAIController.h"
#include "Navigation/CrowdFollowingComponent.h"

ATsnTestAIController::ATsnTestAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(
		TEXT("PathFollowingComponent")))
{
}
