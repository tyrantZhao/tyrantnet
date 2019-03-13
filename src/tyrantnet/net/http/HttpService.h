#pragma once

#include <memory>

#include <tyrantnet/net/TCPService.h>
#include <tyrantnet/net/http/HttpParser.h>
#include <tyrantnet/common/NonCopyable.h>
#include <tyrantnet/net/http/WebSocketFormat.h>
#include <tyrantnet/net/ListenThread.h>
#include <tyrantnet/net/Any.h>

namespace tyrantnet { namespace net { namespace http {

    class HttpService;

    class HttpSession : public common::NonCopyable
    {
    public:
        using Ptr = std::shared_ptr<HttpSession>;

        using EnterCallback = std::function <void(const HttpSession::Ptr&)>;
        using HttpParserCallback = std::function <void(const HTTPParser&, const HttpSession::Ptr&)>;
        using WsCallback =  std::function < void(   const HttpSession::Ptr&,
                                                    WebSocketFormat::WebSocketFrameType opcode,
                                                    const std::string& payload)>;

        using ClosedCallback = std::function <void(const HttpSession::Ptr&)>;
        using WsConnectedCallback = std::function <void(const HttpSession::Ptr&, const HTTPParser&)>;

    public:
        void                        setHttpCallback(HttpParserCallback callback);
        void                        setClosedCallback(ClosedCallback callback);
        void                        setWSCallback(WsCallback callback);
        void                        setWSConnected(WsConnectedCallback callback);

        void                        send(const TcpConnection::PacketPtr& packet,
                                        const TcpConnection::PacketSendedCallback& callback = nullptr);
        void                        send(const char* packet,
                                        size_t len,
                                        const TcpConnection::PacketSendedCallback& callback = nullptr);

        void                        postShutdown() const;
        void                        postClose() const;

        const TyrantnetAny&            getUD() const;
        void                        setUD(TyrantnetAny);

    protected:
        explicit HttpSession(TcpConnection::Ptr);
        virtual ~HttpSession() = default;

        static  Ptr                 Create(TcpConnection::Ptr session);

        const TcpConnection::Ptr&   getSession() const;

        const HttpParserCallback&   getHttpCallback() const;
        const ClosedCallback&       getCloseCallback() const;
        const WsCallback&           getWSCallback() const;
        const WsConnectedCallback&  getWSConnectedCallback() const;

    private:
        TcpConnection::Ptr          mSession;
        TyrantnetAny                   mUD;
        HttpParserCallback          mHttpRequestCallback;
        WsCallback                  mWSCallback;
        ClosedCallback              mCloseCallback;
        WsConnectedCallback         mWSConnectedCallback;

        friend class HttpService;
    };

    class HttpService
    {
    public:
        static void setup(const TcpConnection::Ptr& session, const HttpSession::EnterCallback& enterCallback);

    private:
        static void handle(const HttpSession::Ptr& httpSession);

        static size_t ProcessWebSocket(const char* buffer,
            size_t len,
            const HTTPParser::Ptr& httpParser,
            const HttpSession::Ptr& httpSession);

        static size_t ProcessHttp(const char* buffer,
            size_t len,
            const HTTPParser::Ptr& httpParser,
            const HttpSession::Ptr& httpSession);
    };

} } }