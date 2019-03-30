#pragma once

#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <cstdint>
#include <memory>

#include <tyrantnet/net/TcpConnection.h>
#include <tyrantnet/common/NonCopyable.h>
#include <tyrantnet/common/Typeids.h>
#include <tyrantnet/net/SSLHelper.h>
#include <tyrantnet/net/Socket.h>

namespace tyrantnet { namespace net {

    class EventLoop;
    class IOLoopData;
    using IOLoopDataPtr = std::shared_ptr<IOLoopData>;

    class TcpService : public common::NonCopyable, public std::enable_shared_from_this<TcpService>
    {
    public:
        using Ptr = std::shared_ptr<TcpService>;

        using FrameCallback = std::function<void(const EventLoop::Ptr&)>;
        using EnterCallback = std::function<void(const TcpConnection::Ptr&)>;

        class AddSocketOption
        {
        public:
            struct Options;

            using AddSocketOptionFunc = std::function<void(Options& option)>;

            static AddSocketOptionFunc WithEnterCallback(TcpService::EnterCallback callback);
            static AddSocketOptionFunc WithClientSideSSL();
            static AddSocketOptionFunc WithServerSideSSL(SSLHelper::Ptr sslHelper);
            static AddSocketOptionFunc WithMaxRecvBufferSize(size_t size);
            static AddSocketOptionFunc WithForceSameThreadLoop(bool same);
        };

    public:
        static  Ptr                         Create();

        void                                startWorkerThread(size_t threadNum, FrameCallback callback = nullptr);
        void                                stopWorkerThread();
        template<class... Options>
        bool                                addTcpConnection(TcpSocket::Ptr socket, const Options& ... options)
        {
            return _addTcpConnection(std::move(socket), { options... });
        }

        EventLoop::Ptr                      getRandomEventLoop();

    protected:
        TcpService() ;
        virtual ~TcpService() ;

        bool                                _addTcpConnection(TcpSocket::Ptr socket,
            const std::vector<AddSocketOption::AddSocketOptionFunc>&);
        EventLoop::Ptr                      getSameThreadEventLoop();

    private:
        std::vector<IOLoopDataPtr>          mIOLoopDatas;
        mutable std::mutex                  mIOLoopGuard;
        std::shared_ptr<bool>               mRunIOLoop;

        std::mutex                          mServiceGuard;
    };

} }
