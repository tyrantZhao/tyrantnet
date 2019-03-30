#pragma once

#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <mutex>

#include <tyrantnet/common/NonCopyable.h>
#include <tyrantnet/common/Typeids.h>
#include <tyrantnet/net/SocketLibFunction.h>
#include <tyrantnet/net/Socket.h>

namespace tyrantnet { namespace net {

    class ListenThread : public common::NonCopyable, public std::enable_shared_from_this<ListenThread>
    {
    public:
        using Ptr = std::shared_ptr<ListenThread>;
        using AcceptCallback = std::function<void(TcpSocket::Ptr)>;

        static  Ptr                         Create(bool isIPV6,
                                                const std::string& ip,
                                                const int port,
                                                const AcceptCallback& callback);
        void                                startListen();
        void                                stopListen();

    private:
        ListenThread(bool isIPV6, const std::string& ip, const int port, const AcceptCallback& callback);
        virtual ~ListenThread();

    private:
        bool                                mIsIPV6;
        std::string                         mIP;
        int                                 mPort;
        AcceptCallback                      mCallback;
        std::shared_ptr<bool>               mRunListen;
        std::shared_ptr<std::thread>        mListenThread;
        std::mutex                          mListenThreadGuard;
    };

} }
