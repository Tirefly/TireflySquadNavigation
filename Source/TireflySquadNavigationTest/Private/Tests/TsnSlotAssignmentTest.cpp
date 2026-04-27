// TsnSlotAssignmentTest.cpp
// 功能测试：验证多环角度槽位分配逻辑（直接调用 API，不依赖 BT）

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/TsnTestAutomationHelpers.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "TsnTestTargetDummy.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
	FTsnSlotAssignmentTest,
	"TireflySquadNavigation.Test.Functional.SlotAssignment",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

void FTsnSlotAssignmentTest::GetTests(
	TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(TEXT("SlotAssignment_6Requesters_3Rings"));
	OutTestCommands.Add(TsnTestAutomation::DemoAutomationMapPath);
}

bool FTsnSlotAssignmentTest::RunTest(const FString& Parameters)
{
	// Step 1: 加载地图
	ADD_LATENT_AUTOMATION_COMMAND(FLoadGameMapCommand(Parameters));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForMapToLoadCommand());

	// Step 2-4: 测试逻辑在自定义 Latent Command 中
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.5f));

	/**
	 * 自定义 Latent Command：Spawn + RequestSlot + 验证 + 清理
	 */
	class FSlotAssignmentLatentCommand : public IAutomationLatentCommand
	{
	public:
		explicit FSlotAssignmentLatentCommand(FAutomationTestBase* InTest)
			: Test(InTest) {}

		virtual bool Update() override
		{
			UWorld* World = GEngine ? GEngine->GetWorldContexts()[0].World() : nullptr;
			if (!World)
			{
				Test->AddError(TEXT("No valid World"));
				return true;
			}

			TsnTestAutomation::ResetShowcaseActors(World);

			// Spawn TargetDummy at origin
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ATsnTestTargetDummy* Dummy = World->SpawnActor<ATsnTestTargetDummy>(
				FVector::ZeroVector, FRotator::ZeroRotator, Params);
			if (!Dummy)
			{
				Test->AddError(TEXT("Failed to spawn TargetDummy"));
				return true;
			}

			UTsnEngagementSlotComponent* SlotComp = Dummy->EngagementSlotComp;
			if (!SlotComp)
			{
				Test->AddError(TEXT("TargetDummy has no EngagementSlotComponent"));
				Dummy->Destroy();
				return true;
			}

			// Spawn 6 requesters as simple AActor
			struct FRequesterInfo
			{
				AActor* Actor;
				float AttackRange;
				FVector SlotWorldPos;
				FString Label;
			};

			TArray<FRequesterInfo> Requesters;
			const float Ranges[] = { 150.f, 150.f, 300.f, 300.f, 600.f, 600.f };
			const TCHAR* Labels[] = {
				TEXT("#1 Melee"), TEXT("#2 Melee"),
				TEXT("#3 Spear"), TEXT("#4 Spear"),
				TEXT("#5 Ranged"), TEXT("#6 Ranged")
			};
			const float StartAngles[] = { 0.f, 180.f, 90.f, 270.f, 45.f, 225.f };

			for (int32 i = 0; i < 6; ++i)
			{
				// 从不同方位角接近
				const float Rad = FMath::DegreesToRadians(StartAngles[i]);
				const FVector SpawnLoc = FVector(
					FMath::Cos(Rad) * 2000.f,
					FMath::Sin(Rad) * 2000.f, 0.f);

				AActor* Req = World->SpawnActor<AActor>(
					AActor::StaticClass(), FTransform(SpawnLoc));
				if (!Req)
				{
					// Fallback: use a primitive component holder
					Req = World->SpawnActor<AActor>();
				}

				if (Req)
				{
					Req->SetActorLocation(SpawnLoc);
					FRequesterInfo Info;
					Info.Actor = Req;
					Info.AttackRange = Ranges[i];
					Info.Label = Labels[i];
					Info.SlotWorldPos = FVector::ZeroVector;
					Requesters.Add(Info);
				}
			}

			// Request slots
			for (FRequesterInfo& R : Requesters)
			{
				R.SlotWorldPos = SlotComp->RequestSlot(R.Actor, R.AttackRange);
			}

			// 验证
			const int32 OccupiedCount = SlotComp->GetOccupiedSlotCount();
			Test->TestEqual(TEXT("OccupiedSlotCount == 6"), OccupiedCount, 6);

			// 验证不同环的半径（通过世界位置到 TargetDummy 的距离推断）
			const FVector DummyLoc = Dummy->GetActorLocation();
			const float SlotRadiusOffset = SlotComp->SlotRadiusOffset;
			const float RingTolerance = SlotComp->SameRingRadiusTolerance;

			for (const FRequesterInfo& R : Requesters)
			{
				const float ActualDist = FVector::Dist2D(R.SlotWorldPos, DummyLoc);
				const float ExpectedRadius = R.AttackRange + SlotRadiusOffset;

				Test->TestTrue(
					FString::Printf(TEXT("%s: Slot radius %.0f ≈ expected %.0f (±%.0f)"),
						*R.Label, ActualDist, ExpectedRadius, RingTolerance),
					FMath::Abs(ActualDist - ExpectedRadius) <= RingTolerance + 1.f);
			}

			// 验证同环内角度不完全重叠
			for (int32 i = 0; i < Requesters.Num(); ++i)
			{
				for (int32 j = i + 1; j < Requesters.Num(); ++j)
				{
					if (FMath::Abs(Requesters[i].AttackRange - Requesters[j].AttackRange) < 1.f)
					{
						// 同环
						const float Dist = FVector::Dist2D(
							Requesters[i].SlotWorldPos, Requesters[j].SlotWorldPos);
						Test->TestTrue(
							FString::Printf(TEXT("%s vs %s: same ring dist=%.0f > 0"),
								*Requesters[i].Label, *Requesters[j].Label, Dist),
							Dist > 1.f);
					}
				}
			}

			// DrawDebug 可视化 (Duration=10s)
#if ENABLE_DRAW_DEBUG
			const FColor RingColors[] = { FColor::Red, FColor::Green, FColor::Blue };
			const float RingRadii[] = {
				150.f + SlotRadiusOffset,
				300.f + SlotRadiusOffset,
				600.f + SlotRadiusOffset
			};

			for (int32 i = 0; i < 3; ++i)
			{
				DrawDebugCircle(World, DummyLoc, RingRadii[i], 48,
					RingColors[i], false, 10.f, 0, 2.f,
					FVector(1, 0, 0), FVector(0, 1, 0), false);
			}

			for (const FRequesterInfo& R : Requesters)
			{
				const float Dist = FVector::Dist2D(R.SlotWorldPos, DummyLoc);
				FColor SlotColor = (R.AttackRange <= 200.f) ? FColor::Red
					: (R.AttackRange <= 400.f) ? FColor::Green : FColor::Blue;

				DrawDebugSphere(World, R.SlotWorldPos, 15.f, 8, SlotColor, false, 10.f);
				DrawDebugLine(World, DummyLoc, R.SlotWorldPos,
					FColor(255, 255, 255, 128), false, 10.f, 0, 1.f);
				DrawDebugString(World, R.SlotWorldPos + FVector(0, 0, 30),
					FString::Printf(TEXT("%s R=%.0f"), *R.Label, Dist),
					nullptr, FColor::White, 10.f, true);
			}

			DrawDebugBox(World, DummyLoc, FVector(20), FColor::Yellow, false, 10.f, 0, 3.f);
#endif

			// 清理
			for (const FRequesterInfo& R : Requesters)
			{
				if (R.Actor) R.Actor->Destroy();
			}
			Dummy->Destroy();

			return true;
		}

	private:
		FAutomationTestBase* Test;
	};

	ADD_LATENT_AUTOMATION_COMMAND(FSlotAssignmentLatentCommand(this));

	return true;
}
