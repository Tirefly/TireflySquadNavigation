// TsnDeveloperSettings.cpp

#include "Settings/TsnDeveloperSettings.h"

const UTsnDeveloperSettings* UTsnDeveloperSettings::Get()
{
	return GetDefault<UTsnDeveloperSettings>();
}