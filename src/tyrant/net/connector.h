#ifndef __NET_CONNECTOR_H__
#define __NET_CONNECTOR_H__

#include <functional>
#include <memory>

#include <tyrant/common/noncopyable.h>
#include <tyrant/common/cppversion.h>
#include <tyrant/net/any.h>
#include <tyrant/net/socketlibtypes.h>
#include <tyrant/net/eventloop.h>
#include <tyrant/net/noexcept.h>

#ifdef HAVE_LANG_CXX17
#include <shared_mutex>
#else
#include <mutex>
#endif

namespace tyrant
{
    namespace net
    {
        class ConnectorWorkInfo;

        class AsyncConnector : NonCopyable, public std::enable_shared_from_this<AsyncConnector>
        {
        public:
            typedef std::shared_ptr<AsyncConnector> PTR;
            typedef std::function<void(sock)> COMPLETED_CALLBACK;
            typedef std::function<void()> FAILED_CALLBACK;

            void                startWorkerThread();
            void                stopWorkerThread();
            void                asyncConnect(const std::string& ip, 
                                             int port, 
                                             std::chrono::nanoseconds timeout,
                                             COMPLETED_CALLBACK, 
                                             FAILED_CALLBACK);

            static  PTR         Create();

        private:
            AsyncConnector();
            virtual ~AsyncConnector();
            void                run();

        private:
            ::std::shared_ptr<EventLoop>      mEventLoop;

            ::std::shared_ptr<ConnectorWorkInfo> mWorkInfo;
            ::std::shared_ptr<std::thread>    mThread;
            ::std::mutex                      mThreadGuard;

            bool                            mIsRun;
        };
    } // net
} //tyrant

#endif //connector.h