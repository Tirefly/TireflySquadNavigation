// TsnRepulsionVectorTest.cpp
// 单元测试：验证排斥力双阶段衰减数学模型

#include "Misc/AutomationTest.h"

/**
 * 测试排斥力子系统的力度计算公式。
 *
 * 复现 UTsnStanceRepulsionSubsystem::Tick 中的双阶段衰减：
 * - 内层 (Dist < NavModR): Strength * (1 - Dist/NavModR)^2
 * - 外层 (NavModR <= Dist < RepR): Strength * OuterRatio * (1 - (Dist-NavModR)/(RepR-NavModR))
 * - 外部 (Dist >= RepR): 0
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTsnRepulsionVectorTest,
	"TireflySquadNavigation.Test.Unit.RepulsionVector",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FTsnRepulsionVectorTest::RunTest(const FString& Parameters)
{
	// 标准参数
	const float NavModR = 60.f;
	const float RepR = 150.f;
	const float Strength = 800.f;
	const float OuterRatio = 0.1f;

	// 复现子系统力度计算
	auto CalcMagnitude = [&](float Dist) -> float
	{
		if (Dist <= KINDA_SMALL_NUMBER || Dist >= RepR)
		{
			return 0.f;
		}
		if (Dist < NavModR)
		{
			// 内层：二次衰减
			float Penetration = 1.f - (Dist / NavModR);
			return Strength * FMath::Square(Penetration);
		}
		else
		{
			// 外层：线性衰减
			float OuterRange = RepR - NavModR;
			float OuterDist = Dist - NavModR;
			float OuterFactor = 1.f - (OuterDist / OuterRange);
			return Strength * OuterRatio * OuterFactor;
		}
	};

	struct FTestCase
	{
		float Dist;
		float ExpectedMagnitude;
		FString Description;
	};

	const float Tolerance = 0.5f;

	// 用例 1: 中心 (Dist=0 → KINDA_SMALL_NUMBER 截断 → 力度 = 0)
	// 注：实际子系统在 Dist <= KINDA_SMALL_NUMBER 时 continue（跳过），力度为 0
	{
		float Result = CalcMagnitude(0.f);
		TestNearlyEqual(TEXT("Case 1: Dist=0 (center, skip)"), Result, 0.f, Tolerance);
	}

	// 用例 2: 内层中间 (Dist=30)
	// 800 * (1 - 30/60)^2 = 800 * 0.25 = 200
	{
		float Result = CalcMagnitude(30.f);
		TestNearlyEqual(TEXT("Case 2: Inner half (Dist=30)"), Result, 200.f, Tolerance);
	}

	// 用例 3: 内层边界 (Dist=60)
	// 800 * (1 - 60/60)^2 = 0
	{
		float Result = CalcMagnitude(60.f);
		// Dist=60 >= NavModR → 进入外层：OuterDist=0, OuterFactor=1
		// 800 * 0.1 * 1.0 = 80
		TestNearlyEqual(TEXT("Case 3: Inner boundary (Dist=60)"), Result, 80.f, Tolerance);
	}

	// 用例 4: 外层入口 (Dist=60.01)
	// 800 * 0.1 * (1 - 0.01/90) ≈ 80
	{
		float Result = CalcMagnitude(60.01f);
		float Expected = Strength * OuterRatio * (1.f - 0.01f / 90.f);
		TestNearlyEqual(TEXT("Case 4: Outer entrance (Dist=60.01)"), Result, Expected, Tolerance);
	}

	// 用例 5: 外层中间 (Dist=105)
	// OuterDist=45, OuterRange=90 → OuterFactor=0.5
	// 800 * 0.1 * 0.5 = 40
	{
		float Result = CalcMagnitude(105.f);
		TestNearlyEqual(TEXT("Case 5: Outer middle (Dist=105)"), Result, 40.f, Tolerance);
	}

	// 用例 6: 外层边界 (Dist=150)
	// Dist >= RepR → 0
	{
		float Result = CalcMagnitude(150.f);
		TestNearlyEqual(TEXT("Case 6: Outer boundary (Dist=150)"), Result, 0.f, Tolerance);
	}

	// 用例 7: 排斥半径外 (Dist=200)
	{
		float Result = CalcMagnitude(200.f);
		TestNearlyEqual(TEXT("Case 7: Outside repulsion (Dist=200)"), Result, 0.f, Tolerance);
	}

	// 用例 8: 内外层过渡不连续性
	{
		float InnerEnd = CalcMagnitude(59.99f);   // 内层末端
		float OuterStart = CalcMagnitude(60.0f);  // 外层入口
		// 内层末端 ≈ 800 * (1-59.99/60)^2 ≈ 0.00002..., 接近 0
		// 外层入口 = 80
		// 允许的阶跃：outer > inner
		AddInfo(FString::Printf(
			TEXT("Case 8: Transition - InnerEnd=%.4f, OuterStart=%.4f (expected step)"),
			InnerEnd, OuterStart));
		TestTrue(TEXT("Case 8: Outer start > Inner end (designed discontinuity)"),
			OuterStart > InnerEnd);
	}

	return true;
}
