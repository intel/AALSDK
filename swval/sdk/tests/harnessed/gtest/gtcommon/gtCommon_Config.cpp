// INTEL CONFIDENTIAL - For Intel Internal Use Only
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif // HAVE_CONFIG_H
#include "gtCommon.h"

GlobalTestConfig GlobalTestConfig::sm_Instance;
GlobalTestConfig & GlobalTestConfig::GetInstance()
{
   return GlobalTestConfig::sm_Instance;
}

GlobalTestConfig::GlobalTestConfig() {}
GlobalTestConfig::~GlobalTestConfig() {}

