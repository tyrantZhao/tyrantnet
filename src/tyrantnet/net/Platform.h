#pragma once

#if defined _MSC_VER || defined __MINGW32__
#define PLATFORM_WINDOWS
#else
#define PLATFORM_LINUX
#endif
