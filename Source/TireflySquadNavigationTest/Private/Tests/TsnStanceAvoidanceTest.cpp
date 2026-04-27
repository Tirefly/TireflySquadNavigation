// TsnStanceAvoidanceTest.cpp
// 功能测试：验证移动单位绕开站桩单位（双层防线验证）

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TsnTestAutomationHelpers.h"
#include "TsnTestChessPiece.h"
#include "TsnTestAIController.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "Components/TsnTacticalMovementComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
	FTsnStanceAvoidanceTest,
	"TireflySquadNavigation.Test.Functional.StanceAvoidance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FTsnStanceAvoidanceTest::GetTests(
	TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(TEXT("StanceAvoidance_MoverBypassesWall"));
	OutTestCommands.Add(TsnTestAutomation::DemoAutomationMapPath);
}

bool FTsnStanceAvoidanceTest::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(Parameters));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.5f));

	/**
	 * 自定义 Latent Command：生成站桩墙 + Mover → MoveToLocation → 采样距离 → 验证
	 */
	class FStanceAvoidanceLatentCommand : public IAutomationLatentCommand
	{
	public:
		explicit FStanceAvoidanceLatentCommand(FAutomationTestBase* InTest)
			: Test(InTest) {}

		virtual bool Update() override
		{
			UWorld* World = GEngine ? GEngine->GetWorldContexts()[0].World() : nullptr;
			if (!World) { Test->AddError(TEXT("No World")); return true; }

			if (!bInitialized)
			{
				return Initialize(World);
			}

			return TickUpdate(World);
		}

	private:
		FAutomationTestBase* Test = nullptr;
		bool bInitialized = false;
		float Elapsed = 0.f;
		float MinDistToAnyStance = MAX_FLT;
		TArray<FVector> TrailPoints;
		TArray<TWeakObjectPtr<ATsnTestChessPiece>> StanceUnits;
		TWeakObjectPtr<ATsnTestChessPiece> Mover;
		FVector GoalLocation = FVector(1500.f, 0.f, 0.f);
		float ObstacleRadius = 60.f;
		const float Timeout = 20.f;

		bool Initialize(UWorld* World)
		{
			TsnTestAutomation::ResetShowcaseActors(World);

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// 生成 3 个站桩单位（墙）
			const FVector WallPositions[] = {
				FVector(0, -200, 0),
				FVector(0, 0, 0),
				FVector(0, 200, 0)
			};

			for (const FVector& Pos : WallPositions)
			{
				ATsnTestChessPiece* Unit = World->SpawnActor<ATsnTestChessPiece>(
					ATsnTestChessPiece::StaticClass(), Pos, FRotator::ZeroRotator, Params);
				if (Unit)
				{
					Unit->TeamID = 0;
					if (Unit->StanceObstacleComp)
					{
						ObstacleRadius = Unit->StanceObstacleComp->ObstacleRadius;
						Unit->StanceObstacleComp->EnterStanceMode();
					}
					StanceUnits.Add(Unit);
				}
			}

			// 生成 Mover
			ATsnTestChessPiece* MoverActor = World->SpawnActor<ATsnTestChessPiece>(
				ATsnTestChessPiece::StaticClass(),
				FVector(-1500, 0, 0), FRotator::ZeroRotator, Params);
			if (!MoverActor)
			{
				Test->AddError(TEXT("Failed to spawn Mover"));
				return true;
			}
			MoverActor->TeamID = 1;
			MoverActor->SpawnDefaultController();
			Mover = MoverActor;

			bInitialized = true;
			// 需等待 NavMesh 更新和控制器初始化
			return false;
		}

		bool TickUpdate(UWorld* World)
		{
			Elapsed += World->GetDeltaSeconds();

			ATsnTestChessPiece* MoverActor = Mover.Get();
			if (!MoverActor)
			{
				Test->AddError(TEXT("Mover destroyed unexpectedly"));
				Cleanup(World);
				return true;
			}

			// 首帧发起移动命令
			if (Elapsed < 0.1f)
			{
				AAIController* AICon = Cast<AAIController>(MoverActor->GetController());
				if (AICon)
				{
					AICon->MoveToLocation(GoalLocation);
				}
				return false;
			}

			const FVector MoverLoc = MoverActor->GetActorLocation();

			// 采样最小距离
			for (const TWeakObjectPtr<ATsnTestChessPiece>& StanceUnit : StanceUnits)
			{
				if (StanceUnit.IsValid())
				{
					const float Dist2D = FVector::Dist2D(MoverLoc, StanceUnit->GetActorLocation());
					MinDistToAnyStance = FMath::Min(MinDistToAnyStance, Dist2D);
				}
			}

			// 轨迹采样（每 10 帧）
			if (GFrameCounter % 10 == 0)
			{
				TrailPoints.Add(MoverLoc);
			}

			// 到达检测
			if (FVector::Dist2D(MoverLoc, GoalLocation) < 100.f)
			{
				VerifyAndVisualize(World, true);
				Cleanup(World);
				return true;
			}

			// 超时
			if (Elapsed > Timeout)
			{
				Test->AddWarning(FString::Printf(
					TEXT("Mover stuck after %.0fs, current pos=%s"),
					Elapsed, *MoverLoc.ToString()));
				VerifyAndVisualize(World, false);
				Cleanup(World);
				return true;
			}

			return false;
		}

		void VerifyAndVisualize(UWorld* World, bool bReached)
		{
			if (!bReached)
			{
				Test->AddError(TEXT("Mover did not reach goal (stuck)"));
			}

			Test->TestTrue(
				FString::Printf(TEXT("MinDist=%.1f >= ObstacleRadius=%.1f"),
					MinDistToAnyStance, ObstacleRadius),
				MinDistToAnyStance >= ObstacleRadius - 5.f); // 5cm tolerance

#if ENABLE_DRAW_DEBUG
			// 轨迹
			for (int32 i = 1; i < TrailPoints.Num(); ++i)
			{
				DrawDebugLine(World, TrailPoints[i - 1], TrailPoints[i],
					FColor::Green, false, 10.f, 0, 2.f);
			}

			// 站桩单位区域
			for (const TWeakObjectPtr<ATsnTestChessPiece>& Unit : StanceUnits)
			{
				if (Unit.IsValid())
				{
					const FVector Pos = Unit->GetActorLocation();
					DrawDebugCircle(World, Pos, ObstacleRadius, 24,
						FColor::Red, false, 10.f, 0, 2.f,
						FVector(1, 0, 0), FVector(0, 1, 0), false);
					DrawDebugCircle(World, Pos, 150.f, 24,
						FColor::Orange, false, 10.f, 0, 1.f,
						FVector(1, 0, 0), FVector(0, 1, 0), false);
				}
			}

			// 结果标注
			const FString ResultStr = (MinDistToAnyStance >= ObstacleRadius - 5.f)
				? FString::Printf(TEXT("PASS: MinDist=%.0fcm >= %.0fcm"),
					MinDistToAnyStance, ObstacleRadius)
				: FString::Printf(TEXT("FAIL: MinDist=%.0fcm < %.0fcm"),
					MinDistToAnyStance, ObstacleRadius);
			DrawDebugString(World, GoalLocation + FVector(0, 0, 50),
				ResultStr, nullptr,
				(MinDistToAnyStance >= ObstacleRadius - 5.f) ? FColor::Green : FColor::Red,
				10.f, true);
#endif
		}

		void Cleanup(UWorld* World)
		{
			if (Mover.IsValid()) Mover->Destroy();
			for (const TWeakObjectPtr<ATsnTestChessPiece>& Unit : StanceUnits)
			{
				if (Unit.IsValid()) Unit->Destroy();
			}
		}
	};

	ADD_LATENT_AUTOMATION_COMMAND(FStanceAvoidanceLatentCommand(this));

	return true;
}
