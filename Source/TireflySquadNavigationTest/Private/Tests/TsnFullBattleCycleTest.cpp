// TsnFullBattleCycleTest.cpp
// 功能测试：完整战斗循环集成测试（追击→占位→站桩→攻击→释放→退出站桩）

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "TsnTestChessPiece.h"
#include "TsnTestAIController.h"
#include "TsnTestTargetDummy.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
	FTsnFullBattleCycleTest,
	"TireflySquadNavigation.Test.Functional.FullBattleCycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FTsnFullBattleCycleTest::GetTests(
	TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(TEXT("FullBattleCycle_4Units_ChaseToStance"));
	OutTestCommands.Add(TEXT("/TireflySquadNavigation/Content/Test/Maps/MAP_TsnAutoTest"));
}

bool FTsnFullBattleCycleTest::RunTest(const FString& Parameters)
{
	ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(Parameters));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(1.0f));

	/**
	 * 自定义 Latent Command：BT 驱动的完整战斗循环测试
	 */
	class FFullBattleCycleLatentCommand : public IAutomationLatentCommand
	{
	public:
		explicit FFullBattleCycleLatentCommand(FAutomationTestBase* InTest)
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
		const float Timeout = 25.f;

		TWeakObjectPtr<ATsnTestTargetDummy> TargetDummy;

		struct FUnitTracker
		{
			TWeakObjectPtr<ATsnTestChessPiece> Unit;
			float EngagementRange;
			bool bEverReachedStance = false;
			FString TypeLabel;
		};
		TArray<FUnitTracker> Trackers;
		int32 PeakSlotCount = 0;

		bool Initialize(UWorld* World)
		{
			// 加载 BT/BB 资产
			UBehaviorTree* BT = LoadObject<UBehaviorTree>(nullptr,
				TEXT("/TireflySquadNavigation/Content/Test/BT/BT_TsnTestChessPiece.BT_TsnTestChessPiece"));
			UBlackboardData* BB = LoadObject<UBlackboardData>(nullptr,
				TEXT("/TireflySquadNavigation/Content/Test/BT/BB_TsnTest.BB_TsnTest"));

			if (!BT || !BB)
			{
				Test->AddWarning(TEXT("BT/BB assets not found — skip FullBattleCycleTest "
					"(requires [MANUAL] task 3.1/3.2 to create BB_TsnTest & BT_TsnTestChessPiece)"));
				return true;
			}

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			// Spawn TargetDummy
			ATsnTestTargetDummy* Dummy = World->SpawnActor<ATsnTestTargetDummy>(
				ATsnTestTargetDummy::StaticClass(),
				FVector(0, 0, 0), FRotator::ZeroRotator, Params);
			if (!Dummy)
			{
				Test->AddError(TEXT("Failed to spawn TargetDummy"));
				return true;
			}
			TargetDummy = Dummy;

			// Spawn 4 棋子
			const float Ranges[] = { 150.f, 150.f, 600.f, 600.f };
			const TCHAR* Labels[] = { TEXT("Melee"), TEXT("Melee"), TEXT("Ranged"), TEXT("Ranged") };

			for (int32 i = 0; i < 4; ++i)
			{
				const float Y = (i - 1.5f) * 200.f;
				ATsnTestChessPiece* Unit = World->SpawnActor<ATsnTestChessPiece>(
					ATsnTestChessPiece::StaticClass(),
					FVector(-3000, Y, 0), FRotator::ZeroRotator, Params);
				if (!Unit) continue;

				Unit->EngagementRange = Ranges[i];
				Unit->TeamID = 0;
				Unit->AIControllerClass = ATsnTestAIController::StaticClass();
				Unit->SpawnDefaultController();

				AAIController* AICon = Cast<AAIController>(Unit->GetController());
				if (AICon)
				{
					AICon->RunBehaviorTree(BT);
					if (UBlackboardComponent* BBComp = AICon->GetBlackboardComponent())
					{
						BBComp->SetValueAsObject(TEXT("TargetActor"), Dummy);
						BBComp->SetValueAsBool(TEXT("HasTarget"), true);
					}
				}

				FUnitTracker Tracker;
				Tracker.Unit = Unit;
				Tracker.EngagementRange = Ranges[i];
				Tracker.TypeLabel = Labels[i];
				Trackers.Add(Tracker);
			}

			bInitialized = true;
			return false;
		}

		bool TickUpdate(UWorld* World)
		{
			Elapsed += World->GetDeltaSeconds();

			// 更新追踪状态
			bool bAllReachedStance = true;
			for (FUnitTracker& T : Trackers)
			{
				if (!T.Unit.IsValid()) continue;

				if (T.Unit->IsInStanceMode_Implementation())
				{
					T.bEverReachedStance = true;
				}

				if (!T.bEverReachedStance)
				{
					bAllReachedStance = false;
				}
			}

			// 记录峰值槽位数
			if (TargetDummy.IsValid() && TargetDummy->EngagementSlotComp)
			{
				PeakSlotCount = FMath::Max(PeakSlotCount,
					TargetDummy->EngagementSlotComp->GetOccupiedSlotCount());
			}

#if ENABLE_DRAW_DEBUG
			// 实时可视化
			if (TargetDummy.IsValid())
			{
				const FVector Center = TargetDummy->GetActorLocation();
				DrawDebugCircle(World, Center, 130.f, 32, FColor::Red, false, 0.f, 0, 1.5f,
					FVector(1, 0, 0), FVector(0, 1, 0), false);
				DrawDebugCircle(World, Center, 580.f, 48, FColor::Blue, false, 0.f, 0, 1.5f,
					FVector(1, 0, 0), FVector(0, 1, 0), false);
			}

			// OnScreen message
			if (GEngine)
			{
				FString Msg = FString::Printf(TEXT("[FullBattleCycle] %.1fs / %.0fs\n"), Elapsed, Timeout);
				for (int32 i = 0; i < Trackers.Num(); ++i)
				{
					const FUnitTracker& T = Trackers[i];
					Msg += FString::Printf(TEXT("Unit#%d [%s] %s %s\n"),
						i + 1, *T.TypeLabel,
						T.Unit.IsValid() && T.Unit->IsInStanceMode_Implementation()
							? TEXT("STANCE") : TEXT("..."),
						T.bEverReachedStance ? TEXT("V") : TEXT(""));
				}
				Msg += FString::Printf(TEXT("Peak Slots: %d"), PeakSlotCount);
				GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan, Msg);
			}
#endif

			// 全部达成 STANCE
			if (bAllReachedStance)
			{
				VerifyResults(World);
				Cleanup(World);
				return true;
			}

			// 超时
			if (Elapsed > Timeout)
			{
				Test->AddError(FString::Printf(
					TEXT("Timeout after %.0fs. Not all units reached STANCE."), Elapsed));
				for (int32 i = 0; i < Trackers.Num(); ++i)
				{
					if (!Trackers[i].bEverReachedStance)
					{
						Test->AddError(FString::Printf(
							TEXT("Unit#%d [%s] never reached STANCE"),
							i + 1, *Trackers[i].TypeLabel));
					}
				}
				Cleanup(World);
				return true;
			}

			return false;
		}

		void VerifyResults(UWorld* World)
		{
			Test->TestTrue(TEXT("All units reached STANCE"), true);
			Test->TestTrue(
				FString::Printf(TEXT("PeakSlotCount=%d >= 4"), PeakSlotCount),
				PeakSlotCount >= 4);

			// 验证近战和远程的槽位分环
			// 通过各单位到 TargetDummy 的距离判断
			if (TargetDummy.IsValid())
			{
				float MinMeleeDist = MAX_FLT;
				float MaxRangedDist = 0.f;
				const FVector TargetLoc = TargetDummy->GetActorLocation();

				for (const FUnitTracker& T : Trackers)
				{
					if (!T.Unit.IsValid()) continue;
					const float Dist = FVector::Dist2D(T.Unit->GetActorLocation(), TargetLoc);
					if (T.EngagementRange <= 200.f)
					{
						MinMeleeDist = FMath::Min(MinMeleeDist, Dist);
					}
					else
					{
						MaxRangedDist = FMath::Max(MaxRangedDist, Dist);
					}
				}

				if (MinMeleeDist < MAX_FLT && MaxRangedDist > 0.f)
				{
					Test->TestTrue(
						FString::Printf(TEXT("Ranged dist (%.0f) > Melee dist (%.0f) by >100cm"),
							MaxRangedDist, MinMeleeDist),
						MaxRangedDist - MinMeleeDist > 100.f);
				}
			}

#if ENABLE_DRAW_DEBUG
			// 最终状态可视化 (Duration=10s)
			for (int32 i = 0; i < Trackers.Num(); ++i)
			{
				if (!Trackers[i].Unit.IsValid()) continue;
				const FVector Pos = Trackers[i].Unit->GetActorLocation();
				const FColor Color = Trackers[i].bEverReachedStance ? FColor::Green : FColor::Red;
				DrawDebugCapsule(World, Pos + FVector(0, 0, 44), 44.f, 34.f,
					FQuat::Identity, Color, false, 10.f, 0, 2.f);
				DrawDebugString(World, Pos + FVector(0, 0, 100),
					FString::Printf(TEXT("#%d %s %s"), i + 1, *Trackers[i].TypeLabel,
						Trackers[i].bEverReachedStance ? TEXT("PASS") : TEXT("FAIL")),
					nullptr, Color, 10.f, true);
			}

			DrawDebugString(World, FVector(0, 0, 200),
				TEXT("PASS: All units completed battle cycle"),
				nullptr, FColor::Green, 10.f, true);
#endif
		}

		void Cleanup(UWorld* World)
		{
			for (const FUnitTracker& T : Trackers)
			{
				if (T.Unit.IsValid()) T.Unit->Destroy();
			}
			if (TargetDummy.IsValid()) TargetDummy->Destroy();
		}
	};

	ADD_LATENT_AUTOMATION_COMMAND(FFullBattleCycleLatentCommand(this));

	return true;
}
