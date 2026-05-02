// TsnTestLog.cpp
// 测试模块统一日志分类定义

#include "TsnTestLog.h"

#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

DEFINE_LOG_CATEGORY(LogTsnTest);

bool TsnTestShouldLogControllerFlow()
{
	static const bool bEnabled = FParse::Param(
		FCommandLine::Get(), TEXT("TsnDebugControllerFlow"));
	return bEnabled;
}
