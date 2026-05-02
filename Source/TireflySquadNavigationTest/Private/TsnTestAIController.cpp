// TsnTestAIController.cpp
// 测试用 AI 控制器 —— CrowdFollowing 集成

#include "TsnTestAIController.h"
#include "TsnTestLog.h"
#include "BrainComponent.h"
#include "GameFramework/Pawn.h"
#include "Navigation/CrowdFollowingComponent.h"

ATsnTestAIController::ATsnTestAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(
		TEXT("PathFollowingComponent")))
{
	bStartAILogicOnPossess = true;
}

void ATsnTestAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (TsnTestShouldLogControllerFlow())
	{
		UE_LOG(LogTsnTest, Log,
			TEXT("AIController [%s] possessed Pawn [%s], BrainComponent=%s, BrainRunning=%s, PathFollowing=%s"),
			*GetName(),
			*GetNameSafe(InPawn),
			*GetNameSafe(BrainComponent),
			BrainComponent && BrainComponent->IsRunning() ? TEXT("true") : TEXT("false"),
			*GetNameSafe(GetPathFollowingComponent()));
	}
}
