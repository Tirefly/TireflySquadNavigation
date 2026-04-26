// TsnTestTypes.h
// 测试模块公共类型定义

#pragma once

#include "CoreMinimal.h"
#include "TsnTestTypes.generated.h"

/** 生成器阵型类型 */
UENUM(BlueprintType)
enum class ETsnTestFormation : uint8
{
	/** 横排一字排开 */
	Line		UMETA(DisplayName = "Line"),
	/** 纵列排开 */
	Column		UMETA(DisplayName = "Column"),
	/** 方阵（自动计算行列数） */
	Grid		UMETA(DisplayName = "Grid"),
	/** 圆形环绕 Spawner 中心 */
	Circle		UMETA(DisplayName = "Circle"),
};
