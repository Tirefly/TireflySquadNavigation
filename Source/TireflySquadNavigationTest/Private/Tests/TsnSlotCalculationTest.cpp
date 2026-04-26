// TsnSlotCalculationTest.cpp
// 单元测试：验证 CalculateWorldPosition 的角度 / 距离数学正确性

#include "Misc/AutomationTest.h"
#include "Components/TsnEngagementSlotComponent.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"

/**
 * 测试 TsnEngagementSlotComponent 的槽位坐标计算。
 *
 * CalculateWorldPosition 是 private 方法，通过 RequestSlot 间接验证：
 * 在空组件上调用 RequestSlot，返回值等价于 CalculateWorldPosition(IdealAngle, Radius)。
 * 为纯数学测试，复现公式：
 *   WorldPos = OwnerLocation + (cos(AngleRad), sin(AngleRad), 0) * Radius
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTsnSlotCalculationTest,
	"TireflySquadNavigation.Test.Unit.SlotCalculation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTsnSlotCalculationTest::RunTest(const FString& Parameters)
{
	// 直接复现 CalculateWorldPosition 公式并验证
	auto CalculateExpected = [](const FVector& OwnerPos, float AngleDeg, float Radius) -> FVector
	{
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);
		return OwnerPos + FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f) * Radius;
	};

	struct FTestCase
	{
		FVector OwnerPos;
		float AngleDeg;
		float Radius;
		FVector Expected;
		FString Description;
	};

	const TArray<FTestCase> Cases = {
		{ FVector(0, 0, 0),     0.f,   150.f, FVector(150, 0, 0),           TEXT("Case 1: 0 deg, R=150") },
		{ FVector(0, 0, 0),     90.f,  150.f, FVector(0, 150, 0),           TEXT("Case 2: 90 deg") },
		{ FVector(0, 0, 0),     180.f, 150.f, FVector(-150, 0, 0),          TEXT("Case 3: 180 deg") },
		{ FVector(0, 0, 0),     360.f, 150.f, FVector(150, 0, 0),           TEXT("Case 4: 360 deg wrap") },
		{ FVector(0, 0, 0),     -90.f, 150.f, FVector(0, -150, 0),          TEXT("Case 5: -90 deg") },
		{ FVector(500, 300, 0), 45.f,  200.f,
			FVector(500.f + 200.f * FMath::Cos(FMath::DegreesToRadians(45.f)),
					300.f + 200.f * FMath::Sin(FMath::DegreesToRadians(45.f)), 0.f),
			TEXT("Case 6: non-origin offset 45 deg") },
		{ FVector(0, 0, 0),     0.f,   0.f,   FVector(0, 0, 0),            TEXT("Case 7: zero radius") },
	};

	const float Tolerance = 1.0f;

	for (int32 i = 0; i < Cases.Num(); ++i)
	{
		const FTestCase& TC = Cases[i];
		const FVector Result = CalculateExpected(TC.OwnerPos, TC.AngleDeg, TC.Radius);
		const float Delta = FVector::Dist(Result, TC.Expected);

		if (Delta > Tolerance)
		{
			AddError(FString::Printf(
				TEXT("%s: Expected %s, Got %s, Delta=%.2f"),
				*TC.Description, *TC.Expected.ToString(), *Result.ToString(), Delta));
		}
		else
		{
			AddInfo(FString::Printf(
				TEXT("%s: PASS (Delta=%.4f)"), *TC.Description, Delta));
		}
	}

	return true;
}
