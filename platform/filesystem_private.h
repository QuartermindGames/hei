#pragma once

#include <PL/platform_filesystem.h>

#ifdef _DEBUG
#   define FSLog(...) plLogMessage(LOG_LEVEL_FILESYSTEM, __VA_ARGS__)
#else
#   define FSLog(...)
#endif