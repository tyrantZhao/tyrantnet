#include <tyrant/net/currentthread.h>

#ifdef PLATFORM_WINDOWS
#else
#include <unistd.h>
#include <sys/syscall.h>
#endif

namespace tyrant
{
    namespace net
    {
        namespace CurrentThread
        {
#ifdef PLATFORM_WINDOWS
__declspec(thread) THREAD_ID_TYPE cachedTid = 0;
#else
__thread THREAD_ID_TYPE cachedTid = 0;
#endif
        } // CurrentThread
    } // net
} // tyrant

::tyrant::net::CurrentThread::THREAD_ID_TYPE& tyrant::net::CurrentThread::tid()
{
    if (cachedTid == 0)
    {
#ifdef PLATFORM_WINDOWS
        cachedTid = GetCurrentThreadId();
#else
        cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
#endif
    }

    return cachedTid;
}
