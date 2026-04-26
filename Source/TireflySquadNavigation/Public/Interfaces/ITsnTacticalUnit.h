// ITsnTacticalUnit.h
// 宿主项目战斗单位接口定义

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ITsnTacticalUnit.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UTsnTacticalUnit : public UInterface
{
	GENERATED_BODY()
};

/**
 * 宿主项目的战斗单位 Actor 需要实现此接口。
 * 插件中所有需要读取单位属性的地方均通过此接口访问，
 * 不依赖任何具体的宿主类。
 *
 * 实现方式：C++ 继承 或 蓝图实现接口函数，两者均可。
 */
class TIREFLYSQUADNAVIGATION_API ITsnTacticalUnit
{
	GENERATED_BODY()

public:
	/**
	 * 返回该单位的交战距离（攻击/施法/射程距离）。
	 * 插件用此值决定 TsnEngagementSlotComponent 的环半径，
	 * 以及 BT 节点中"进入交战距离"的判断阈值。
	 *
	 * @return 交战距离（单位：cm）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TireflySquadNavigation")
	float GetEngagementRange() const;

	/**
	 * 返回该单位当前是否处于站姿模式（Stance Mode）。
	 * 插件子系统在遍历场景中的战斗单位时，
	 * 通过此接口判断是否需要对其应用排斥力。
	 *
	 * @return true 表示单位处于原地站立战斗状态
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TireflySquadNavigation")
	bool IsInStanceMode() const;
};
