#pragma once

#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <mutex>

#include <tyrantnet/common/NonCopyable.h>
#include <tyrantnet/common/Typeids.h>
#include <tyrantnet/net/SocketLibFunction.h>
#include <tyrantnet/net/Noexcept.h>
#include <tyrantnet/net/Socket.h>

namespace tyrantnet { namespace net {

    class ListenThread : public common::NonCopyable, public std::enable_shared_from_this<ListenThread>
    {
    public:
        using Ptr = std::shared_ptr<ListenThread>;
        using AccepCallback = std::function<void(TcpSocket::Ptr)>;

        void                                startListen(bool isIPV6,
                                                const std::string& ip,
                                                int port,
                                                AccepCallback callback);
        void                                stopListen();
        static  Ptr                         Create();

    private:
        ListenThread() TYRANTNET_NOEXCEPT;
        virtual ~ListenThread() TYRANTNET_NOEXCEPT;

    private:
        bool                                mIsIPV6;
        std::string                         mIP;
        int                                 mPort;
        std::shared_ptr<bool>               mRunListen;
        std::shared_ptr<std::thread>        mListenThread;
        std::mutex                          mListenThreadGuard;
    };

} }
