// TsnTestSpawner.cpp
// 测试用批量生成器 —— 阵型计算 + 行为树配置

#include "TsnTestSpawner.h"
#include "TsnTestChessPiece.h"
#include "TsnTestAIController.h"
#include "TsnTestLog.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ATsnTestSpawner::ATsnTestSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATsnTestSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoSpawnOnBeginPlay)
	{
		SpawnAll();
	}
}

void ATsnTestSpawner::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if ENABLE_DRAW_DEBUG
	const FVector HeadLoc = GetActorLocation() + FVector(0.f, 0.f, 150.f);
	const int32 Alive = GetAliveCount();
	const TCHAR* TeamName = (SpawnTeamID == 0) ? TEXT("Red") : TEXT("Blue");

	DrawDebugString(GetWorld(), HeadLoc,
		FString::Printf(TEXT("Spawner [%s] | Alive: %d/%d"),
			TeamName, Alive, SpawnCount),
		nullptr, FColor::White, 0.f, true);
#endif
}

void ATsnTestSpawner::SpawnAll()
{
	UWorld* World = GetWorld();
	if (!World || !ChessPieceClass)
	{
		UE_LOG(LogTsnTest, Warning,
			TEXT("Spawner [%s] cannot spawn: World=%p, Class=%s"),
			*GetName(), World, ChessPieceClass ? *ChessPieceClass->GetName() : TEXT("null"));
		return;
	}

	for (int32 i = 0; i < SpawnCount; ++i)
	{
		const FVector Offset = CalculateFormationOffset(i);
		const FVector SpawnLoc = GetActorLocation() + Offset;
		const FRotator SpawnRot = GetActorRotation();

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ATsnTestChessPiece* Piece = World->SpawnActor<ATsnTestChessPiece>(
			ChessPieceClass, SpawnLoc, SpawnRot, Params);
		if (!Piece)
		{
			UE_LOG(LogTsnTest, Warning,
				TEXT("Spawner [%s] failed to spawn unit #%d"), *GetName(), i);
			continue;
		}

		// 设置 TeamID
		Piece->TeamID = SpawnTeamID;

		// 更新胶囊体颜色（BeginPlay 已设置，但 TeamID 在 BeginPlay 后被覆写，需要重新应用）
		if (UCapsuleComponent* Capsule = Piece->GetCapsuleComponent())
		{
			Capsule->ShapeColor = (SpawnTeamID == 1) ? FColor::Blue : FColor::Red;
		}

		// 创建 AI 控制器
		Piece->SpawnDefaultController();

		// 配置行为树
		if (AAIController* AICon = Cast<AAIController>(Piece->GetController()))
		{
			if (BehaviorTreeAsset)
			{
				AICon->RunBehaviorTree(BehaviorTreeAsset);

				// 设置初始目标
				if (InitialTarget && AICon->GetBlackboardComponent())
				{
					AICon->GetBlackboardComponent()->SetValueAsObject(
						TEXT("TargetActor"), InitialTarget);
					AICon->GetBlackboardComponent()->SetValueAsBool(
						TEXT("HasTarget"), true);
				}
			}
		}

		SpawnedUnits.Add(Piece);
		UE_LOG(LogTsnTest, Log, TEXT("Spawner [%s] spawned unit #%d [%s] at %s, TeamID=%d"),
			*GetName(), i, *Piece->GetName(), *SpawnLoc.ToString(), SpawnTeamID);
	}
}

FVector ATsnTestSpawner::CalculateFormationOffset(int32 Index) const
{
	const float N = static_cast<float>(SpawnCount);

	switch (Formation)
	{
	case ETsnTestFormation::Line:
		{
			// 横排：沿 Y 轴排列
			const float YOffset = (Index - (N - 1.f) / 2.f) * FormationSpacing;
			return FVector(0.f, YOffset, 0.f);
		}

	case ETsnTestFormation::Column:
		{
			// 纵列：沿 X 轴排列
			const float XOffset = (Index - (N - 1.f) / 2.f) * FormationSpacing;
			return FVector(XOffset, 0.f, 0.f);
		}

	case ETsnTestFormation::Grid:
		{
			const int32 Cols = FMath::CeilToInt32(FMath::Sqrt(N));
			const int32 Rows = FMath::CeilToInt32(N / static_cast<float>(Cols));
			const int32 Row = Index / Cols;
			const int32 Col = Index % Cols;
			const float XOffset = (Row - (Rows - 1.f) / 2.f) * FormationSpacing;
			const float YOffset = (Col - (Cols - 1.f) / 2.f) * FormationSpacing;
			return FVector(XOffset, YOffset, 0.f);
		}

	case ETsnTestFormation::Circle:
		{
			const float AngleRad = 2.f * PI * Index / N;
			return FVector(
				FMath::Cos(AngleRad) * FormationSpacing,
				FMath::Sin(AngleRad) * FormationSpacing,
				0.f);
		}

	default:
		return FVector::ZeroVector;
	}
}

int32 ATsnTestSpawner::GetAliveCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ATsnTestChessPiece>& Unit : SpawnedUnits)
	{
		if (Unit.IsValid())
		{
			++Count;
		}
	}
	return Count;
}
