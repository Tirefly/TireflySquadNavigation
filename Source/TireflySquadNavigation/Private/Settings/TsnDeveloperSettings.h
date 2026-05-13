// TsnDeveloperSettings.h
// TSN 插件开发者设置

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Components/TsnStanceObstacleComponent.h"
#include "TsnDeveloperSettings.generated.h"

/**
 * TSN 插件级默认几何设置。
 *
 * 这层设置只负责提供插件自己的默认 obstacle / nav modifier / repulsion 预算，
 * 不负责推导宿主项目的玩法尺度。
 */
UCLASS(Config = TireflySquadNavigation, DefaultConfig)
class TIREFLYSQUADNAVIGATION_API UTsnDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** 获取设置所在容器。 */
	virtual FName GetContainerName() const override { return FName("Project"); }

	/** 获取设置分类。 */
	virtual FName GetCategoryName() const override { return FName("Plugins"); }

	/** 获取设置分区名。 */
	virtual FName GetSectionName() const override { return FName("TireflySquadNavigation"); }

#if WITH_EDITOR
protected:
	/** 获取设置分区显示文本。 */
	virtual FText GetSectionText() const override
	{
		return FText::FromString(TEXT("Tirefly Squad Navigation"));
	}

	/** 获取设置分区描述。 */
	virtual FText GetSectionDescription() const override
	{
		return FText::FromString(TEXT("Default TSN geometry budgets for stance obstacle, nav modifier, and repulsion."));
	}
#endif

public:
	/** 获取 TSN 插件级默认设置。 */
	static const UTsnDeveloperSettings* Get();

	/** 获取默认 NavModifier 开关。 */
	bool GetDefaultUseNavModifier() const { return bDefaultUseNavModifier; }

	/** 获取默认 NavModifier 模式。 */
	ETsnNavModifierMode GetDefaultNavModifierMode() const { return DefaultNavModifierMode; }

	/** 获取默认站姿障碍半径。 */
	float GetDefaultObstacleRadius() const { return DefaultObstacleRadius; }

	/** 获取默认导航扩张半径。 */
	float GetDefaultNavModifierExtraRadius() const { return DefaultNavModifierExtraRadius; }

	/** 获取默认排斥感知半径。 */
	float GetDefaultRepulsionRadius() const { return DefaultRepulsionRadius; }

	/** 获取默认排斥力强度。 */
	float GetDefaultRepulsionStrength() const { return DefaultRepulsionStrength; }

	/** 获取默认 NavModifier 延迟关闭时间。 */
	float GetDefaultNavModifierDeactivationDelay() const
	{
		return DefaultNavModifierDeactivationDelay;
	}

protected:
	/** 默认是否启用 NavModifier。 */
	UPROPERTY(Config, EditAnywhere, Category = "StanceObstacle Defaults")
	bool bDefaultUseNavModifier = true;

	/** 默认 NavModifier 区域策略。 */
	UPROPERTY(Config, EditAnywhere, Category = "StanceObstacle Defaults",
		meta = (EditCondition = "bDefaultUseNavModifier"))
	ETsnNavModifierMode DefaultNavModifierMode = ETsnNavModifierMode::Impassable;

	/** 默认站姿障碍半径。 */
	UPROPERTY(Config, EditAnywhere, Category = "StanceObstacle Defaults",
		meta = (ClampMin = "10.0"))
	float DefaultObstacleRadius = 60.f;

	/** 默认导航扩张半径。 */
	UPROPERTY(Config, EditAnywhere, Category = "StanceObstacle Defaults",
		meta = (ClampMin = "0.0", EditCondition = "bDefaultUseNavModifier"))
	float DefaultNavModifierExtraRadius = 45.f;

	/** 默认排斥感知半径。 */
	UPROPERTY(Config, EditAnywhere, Category = "StanceObstacle Defaults",
		meta = (ClampMin = "10.0"))
	float DefaultRepulsionRadius = 150.f;

	/** 默认排斥力强度。 */
	UPROPERTY(Config, EditAnywhere, Category = "StanceObstacle Defaults",
		meta = (ClampMin = "0.0"))
	float DefaultRepulsionStrength = 800.f;

	/** 默认 NavModifier 延迟关闭时间。 */
	UPROPERTY(Config, EditAnywhere, Category = "StanceObstacle Defaults",
		meta = (ClampMin = "0.0", EditCondition = "bDefaultUseNavModifier"))
	float DefaultNavModifierDeactivationDelay = 0.3f;
};