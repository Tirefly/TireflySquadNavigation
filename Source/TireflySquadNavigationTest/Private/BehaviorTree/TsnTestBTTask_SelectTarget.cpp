// TsnTestBTTask_SelectTarget.cpp
// 测试用 BT 任务 —— 搜索最近敌方目标

#include "BehaviorTree/TsnTestBTTask_SelectTarget.h"
#include "TsnTestChessPiece.h"
#include "TsnTestTargetDummy.h"
#include "TsnTestLog.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "EngineUtils.h"

UTsnTestBTTask_SelectTarget::UTsnTestBTTask_SelectTarget()
{
	NodeName = TEXT("TSN Test Select Target");

	TargetActorKey.AddObjectFilter(this,
		GET_MEMBER_NAME_CHECKED(UTsnTestBTTask_SelectTarget, TargetActorKey),
		AActor::StaticClass());
	HasTargetKey.AddBoolFilter(this,
		GET_MEMBER_NAME_CHECKED(UTsnTestBTTask_SelectTarget, HasTargetKey));
}

void UTsnTestBTTask_SelectTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (const UBlackboardData* BBData = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BBData);
		HasTargetKey.ResolveSelectedKey(*BBData);
	}
}

EBTNodeResult::Type UTsnTestBTTask_SelectTarget::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	if (!AICon || !AICon->GetPawn())
	{
		return EBTNodeResult::Failed;
	}

	APawn* OwnerPawn = AICon->GetPawn();
	const FVector OwnerLoc = OwnerPawn->GetActorLocation();

	// 获取自身 TeamID（仅 ATsnTestChessPiece 有）
	int32 MyTeamID = -1;
	if (const ATsnTestChessPiece* MyPiece = Cast<ATsnTestChessPiece>(OwnerPawn))
	{
		MyTeamID = MyPiece->TeamID;
	}

	AActor* BestTarget = nullptr;
	float BestDistSq = SearchRadius * SearchRadius;

	UWorld* World = OwnerPawn->GetWorld();

	// 搜索敌方棋子
	for (TActorIterator<ATsnTestChessPiece> It(World); It; ++It)
	{
		ATsnTestChessPiece* Candidate = *It;
		if (Candidate == OwnerPawn) continue;
		if (MyTeamID >= 0 && Candidate->TeamID == MyTeamID) continue;

		const float DistSq = FVector::DistSquared(OwnerLoc, Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	// 搜索靶标（始终视为有效目标）
	for (TActorIterator<ATsnTestTargetDummy> It(World); It; ++It)
	{
		ATsnTestTargetDummy* Candidate = *It;
		const float DistSq = FVector::DistSquared(OwnerLoc, Candidate->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	if (BestTarget)
	{
		BB->SetValue<UBlackboardKeyType_Object>(TargetActorKey.GetSelectedKeyID(), BestTarget);
		BB->SetValue<UBlackboardKeyType_Bool>(HasTargetKey.GetSelectedKeyID(), true);

		UE_LOG(LogTsnTest, Verbose, TEXT("[%s] SelectTarget → %s (dist=%.0f)"),
			*OwnerPawn->GetName(), *BestTarget->GetName(), FMath::Sqrt(BestDistSq));
		return EBTNodeResult::Succeeded;
	}

	BB->SetValue<UBlackboardKeyType_Object>(TargetActorKey.GetSelectedKeyID(), nullptr);
	BB->SetValue<UBlackboardKeyType_Bool>(HasTargetKey.GetSelectedKeyID(), false);

	UE_LOG(LogTsnTest, Verbose, TEXT("[%s] SelectTarget → no target found"), *OwnerPawn->GetName());
	return EBTNodeResult::Failed;
}
