#ifndef __NET_CURRENTTHREAD_H__
#define __NET_CURRENTTHREAD_H__

#include <thread>

#include <tyrant/net/platform.h>

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#include <winsock2.h>
#else
#include <sys/types.h>
#endif

namespace tyrant
{
    namespace net
    {
        namespace CurrentThread
        {
#ifdef PLATFORM_WINDOWS
            typedef DWORD THREAD_ID_TYPE;
            extern __declspec(thread) THREAD_ID_TYPE cachedTid;
#else
            typedef int THREAD_ID_TYPE;
            extern __thread THREAD_ID_TYPE cachedTid;
#endif

            THREAD_ID_TYPE& tid();
        }
    }
}

#endif //currentthread.h
