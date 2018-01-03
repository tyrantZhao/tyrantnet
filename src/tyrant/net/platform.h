#ifndef __NET_PLATFORM_H__
#define __NET_PLATFORM_H__

#if defined _MSC_VER || defined __MINGW32__
#define PLATFORM_WINDOWS
#else
#define PLATFORM_LINUX
#endif

#endif //platform.h
