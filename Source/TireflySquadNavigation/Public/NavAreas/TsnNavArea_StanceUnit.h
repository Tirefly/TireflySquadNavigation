// TsnNavArea_StanceUnit.h
// 站姿单位高代价导航区域

#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "TsnNavArea_StanceUnit.generated.h"

/**
 * 站姿单位高代价导航区域。
 *
 * 在 ETsnNavModifierMode::HighCost 策略下使用。
 * DefaultCost = 10.0：寻路算法强烈倾向绕行，但不完全封死。
 * 棋盘格地图下推荐使用 ETsnNavModifierMode::Impassable（NavArea_Null），
 * 彻底封路以确保绕行。
 */
UCLASS()
class TIREFLYSQUADNAVIGATION_API UTsnNavArea_StanceUnit : public UNavArea
{
	GENERATED_BODY()

public:
	UTsnNavArea_StanceUnit()
	{
		// 穿过站桩区域代价 = 正常路径 × 10
		DefaultCost = 10.f;

		// 进入该区域的一次性额外代价，劝退"短暂切过一个角"的路径
		FixedAreaEnteringCost = 5.f;

		// 橙色，便于 NavMesh 可视化中辨认
		DrawColor = FColor(255, 100, 0, 255);
	}
};
