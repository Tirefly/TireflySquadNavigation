// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

/** TireflySquadNavigation 测试 / 演示模块（Editor-only） */
class FTireflySquadNavigationTestModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
