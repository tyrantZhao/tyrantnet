#ifndef __TYRANTNET_NET_PLATFORM_H__
#define __TYRANTNET_NET_PLATFORM_H__

#if defined _MSC_VER || defined __MINGW32__
#define PLATFORM_WINDOWS
#else
#define PLATFORM_LINUX
#endif

#endif //__TYRANTNET_NET_PLATFORM_H__
