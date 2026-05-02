// TsnTestLog.h
// 测试模块统一日志分类声明

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTsnTest, Log, All);

/** 通过命令行参数 -TsnDebugControllerFlow 打开临时控制链诊断日志。 */
bool TsnTestShouldLogControllerFlow();
