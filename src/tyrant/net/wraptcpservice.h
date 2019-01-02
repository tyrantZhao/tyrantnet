#ifndef __TYRANTNET_NET_WRAPTCPSERVICE_H__
#define __TYRANTNET_NET_WRAPTCPSERVICE_H__

#include <string>
#include <cstdint>
#include <vector>

#include <tyrant/common/noncopyable.h>
#include <tyrant/net/tcpservice.h>
#include <tyrant/net/any.h>
#include <tyrant/net/sslhelper.h>
#include <tyrant/net/socket.h>

namespace tyrant { namespace net {
    class WrapTcpService;
    class IOLoopData;

    class TCPSession : public common::NonCopyable
    {
    public:
        typedef std::shared_ptr<TCPSession>     PTR;
        typedef std::weak_ptr<TCPSession>       WEAK_PTR;

        typedef std::function<void(const TCPSession::PTR&)>   DISCONNECT_CALLBACK;
        typedef std::function<size_t(const TCPSession::PTR&, const char*, size_t)>   DATA_CALLBACK;
        typedef std::function<void(const TCPSession::PTR&)>   SESSION_ENTER_CALLBACK;

    public:
        const TyrantAny&            getUD() const;
        void                        setUD(TyrantAny ud);

        const std::string&          getIP() const;
        TcpService::SESSION_TYPE    getSocketID() const;

        void                        send(const char* buffer,
                                         size_t len, const
                                         DataSocket::PACKED_SENDED_CALLBACK& callback = nullptr) const;

        void                        send(const DataSocket::PACKET_PTR& packet,
                                         const DataSocket::PACKED_SENDED_CALLBACK& callback = nullptr) const;

        void                        postShutdown() const;
        void                        postDisConnect() const;

        void                        setDisConnectCallback(DISCONNECT_CALLBACK callback);
        void                        setDataCallback(DATA_CALLBACK callback);
        const EventLoop::PTR&       getEventLoop() const;

        void                        setHeartBeat(std::chrono::nanoseconds checkTime);

    private:
        TCPSession() TYRANT_NOEXCEPT;
        virtual ~TCPSession() TYRANT_NOEXCEPT;

        void                        setSocketID(TcpService::SESSION_TYPE id);
        void                        setIP(const std::string& ip);

        void                        setIOLoopData(std::shared_ptr<IOLoopData> ioLoopData);
        void                        setService(const TcpService::PTR& service);

        const DISCONNECT_CALLBACK&  getCloseCallback();
        const DATA_CALLBACK&        getDataCallback();

        static  PTR                 Create();

    private:
        TcpService::PTR             mService;
        std::shared_ptr<IOLoopData> mIoLoopData;
        TcpService::SESSION_TYPE    mSocketID;
        std::string                 mIP;
        TyrantAny                   mUD;

        DISCONNECT_CALLBACK         mDisConnectCallback;
        DATA_CALLBACK               mDataCallback;

        friend class WrapTcpService;
    };

    class AddSessionOption
    {
    public:
        struct Options;

        typedef std::function<void(Options& option)> AddSessionOptionFunc;

        static AddSessionOptionFunc WithEnterCallback(TCPSession::SESSION_ENTER_CALLBACK callback);
        static AddSessionOptionFunc WithClientSideSSL();
        static AddSessionOptionFunc WithServerSideSSL(SSLHelper::PTR sslHelper);
        static AddSessionOptionFunc WithMaxRecvBufferSize(size_t size);
        static AddSessionOptionFunc WithForceSameThreadLoop(bool same);
    };

    class WrapTcpService : public common::NonCopyable
    {
    public:
        typedef std::shared_ptr<WrapTcpService> PTR;
        typedef std::weak_ptr<WrapTcpService>   WEAK_PTR;


        WrapTcpService() TYRANT_NOEXCEPT;
        virtual ~WrapTcpService() TYRANT_NOEXCEPT;

        const TcpService::PTR&      getService() const;
        void                        startWorkThread(size_t threadNum,
                                                    TcpService::FRAME_CALLBACK callback = nullptr);
        void                        stopWorkThread();

        template<class... Args>
        bool                        addSession(TcpSocket::PTR socket, const Args& ... args)
        {
            return _addSession(std::move(socket), { args... });
        }
        template<class... Args>
        bool                        addSession(TcpSocket::PTR socket,
            const std::vector<AddSessionOption::AddSessionOptionFunc>& options)
        {
            return _addSession(std::move(socket), options);
        }

    private:
        bool                        _addSession(TcpSocket::PTR socket,
            const std::vector<AddSessionOption::AddSessionOptionFunc>&);

    private:
        TcpService::PTR             mTCPService;
    };
}}

#endif // __TYRANTNET_NET_WRAPTCPSERVICE_H__
