#pragma once

#include <memory>
#include <functional>
#include <deque>
#include <chrono>

#include <tyrantnet/net/Channel.h>
#include <tyrantnet/net/EventLoop.h>
#include <tyrantnet/timer/Timer.h>
#include <tyrantnet/common/NonCopyable.h>
#include <tyrantnet/net/Any.h>
#include <tyrantnet/net/Socket.h>
#include <tyrantnet/common/buffer.h>

#ifdef USE_OPENSSL

#ifdef  __cplusplus
extern "C" {
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>
#ifdef  __cplusplus
}
#endif

#endif

struct buffer_s;

namespace tyrantnet { namespace net {

    class TcpConnection : public Channel, public common::NonCopyable, public std::enable_shared_from_this<TcpConnection>
    {
    public:
        using Ptr = std::shared_ptr<TcpConnection>;

        using EnterCallback = std::function<void(Ptr)>;
        using DataCallback = std::function<size_t(const char* buffer, size_t len)>;
        using DisconnectedCallback = std::function<void(Ptr)>;
        using PacketSendedCallback = std::function<void(void)>;

        using PacketPtr = std::shared_ptr<std::string>;

    public:
        Ptr static Create(TcpSocket::Ptr, size_t maxRecvBufferSize, EnterCallback, EventLoop::Ptr);

        /* must called in network thread */
        bool                            onEnterEventLoop();
        const EventLoop::Ptr&           getEventLoop() const;

        //TODO::如果所属EventLoop已经没有工作，则可能导致内存无限大，因为所投递的请求都没有得到处理

        void                            send(const char* buffer, size_t len, const PacketSendedCallback& callback = nullptr);
        void                            send(const PacketPtr&, const PacketSendedCallback& callback = nullptr);

        void                            setDataCallback(DataCallback cb);
        void                            setDisConnectCallback(DisconnectedCallback cb);

        /* if checkTime is zero, will cancel check heartbeat */
        void                            setHeartBeat(std::chrono::nanoseconds checkTime);
        void                            postDisConnect();
        void                            postShutdown();

        void                            setUD(TyrantnetAny value);
        const TyrantnetAny&                getUD() const;

        const std::string&              getIP() const;

#ifdef USE_OPENSSL
        bool                            initAcceptSSL(SSL_CTX*);
        bool                            initConnectSSL();
#endif

        static  TcpConnection::PacketPtr  makePacket(const char* buffer, size_t len);

    protected:
        TcpConnection(TcpSocket::Ptr, size_t maxRecvBufferSize, EnterCallback, EventLoop::Ptr) ;
        virtual ~TcpConnection() ;

    private:
        void                            growRecvBuffer();

        void                            pingCheck();
        void                            startPingCheckTimer();

        void                            canRecv() override;
        void                            canSend() override;

        bool                            checkRead();
        bool                            checkWrite();

        void                            recv();
        void                            flush();
        void                            normalFlush();
        void                            quickFlush();

        void                            onClose() override;
        void                            procCloseInLoop();
        void                            procShutdownInLoop();

        void                            runAfterFlush();
#ifdef PLATFORM_LINUX
        void                            removeCheckWrite();
#endif
#ifdef USE_OPENSSL
        bool                            processSSLHandshake();
#endif
        void                            causeEnterCallback();
    private:

#ifdef PLATFORM_WINDOWS
        struct port::Win::OverlappedExt mOvlRecv;
        struct port::Win::OverlappedExt mOvlSend;

        bool                            mPostRecvCheck;
        bool                            mPostWriteCheck;
        bool                            mPostClose;
#endif
        const std::string               mIP;
        const TcpSocket::Ptr            mSocket;
        const EventLoop::Ptr            mEventLoop;
        bool                            mCanWrite;
        bool                            mAlreadyClose;

        struct BufferDeleter
        {
            void operator()(struct buffer_s* ptr) const
            {
                ox_buffer_delete(ptr);
            }
        };
        std::unique_ptr<struct buffer_s, BufferDeleter> mRecvBuffer;
        const size_t                    mMaxRecvBufferSize;

        struct PendingPacket
        {
            PacketPtr  data;
            size_t      left;
            PacketSendedCallback  mCompleteCallback;
        };

        using PacketListType = std::deque<PendingPacket>;
        PacketListType                  mSendList;

        EnterCallback                   mEnterCallback;
        DataCallback                    mDataCallback;
        DisconnectedCallback            mDisConnectCallback;

        bool                            mIsPostFlush;

        TyrantnetAny                       mUD;

#ifdef USE_OPENSSL
        SSL_CTX*                        mSSLCtx;
        SSL*                            mSSL;
        bool                            mIsHandsharked;
#endif
        bool                            mRecvData;
        std::chrono::nanoseconds        mCheckTime;
        timer::Timer::WeakPtr           mTimer;
    };

} }
