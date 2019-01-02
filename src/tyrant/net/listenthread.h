#ifndef __TYRANTNET_NET_LISTENTHREAD_H__
#define __TYRANTNET_NET_LISTENTHREAD_H__

#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <mutex>

#include <tyrant/common/noncopyable.h>
#include <tyrant/common/typeids.h>
#include <tyrant/net/noexcept.h>
#include <tyrant/net/socketlibfunction.h>
#include <tyrant/net/socket.h>

namespace tyrant { namespace net {
    class ListenThread : public common::NonCopyable, public std::enable_shared_from_this<ListenThread>
    {
    public:
        typedef std::shared_ptr<ListenThread>   PTR;
        typedef std::function<void(TcpSocket::PTR)> ACCEPT_CALLBACK;

        void                                startListen(bool isIPV6,
                                                        const std::string& ip,
                                                        int port,
                                                        ACCEPT_CALLBACK callback);
        void                                stopListen();
        static  PTR                         Create();

    private:
        ListenThread() TYRANT_NOEXCEPT;
        virtual ~ListenThread() TYRANT_NOEXCEPT;

        void                                runListen();

    private:
        bool                                mIsIPV6;
        std::string                         mIP;
        int                                 mPort;
        std::shared_ptr<bool>               mRunListen;
        std::shared_ptr<std::thread>        mListenThread;
        std::mutex                          mListenThreadGuard;
    };
}}

#endif //__TYRANTNET_NET_LISTENTHREAD_H__
