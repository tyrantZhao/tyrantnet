#include <tyrant/net/http/httpservice.h>

#include <string>
#include <cstring>
#include <cassert>

#include <tyrant/common/sha1.h>
#include <tyrant/common/base64.h>
#include <tyrant/net/http/http_parser.h>
#include <tyrant/net/http/websocketformat.h>

using namespace ::tyrant::net;

HttpSession::HttpSession(TCPSession::PTR session)
{
    mSession = std::move(session);
}

TCPSession::PTR& HttpSession::getSession()
{
    return mSession;
}

const TyrantAny& HttpSession::getUD() const
{
    return mUD;
}

void HttpSession::setUD(TyrantAny ud)
{
    mUD = ud;
}

void HttpSession::setHttpCallback(HTTPPARSER_CALLBACK callback)
{
    mHttpRequestCallback = std::move(callback);
}

void HttpSession::setCloseCallback(CLOSE_CALLBACK callback)
{
    mCloseCallback = std::move(callback);
}

void HttpSession::setWSCallback(WS_CALLBACK callback)
{
    mWSCallback = std::move(callback);
}

void HttpSession::setWSConnected(WS_CONNECTED_CALLBACK callback)
{
    mWSConnectedCallback = std::move(callback);
}

const HttpSession::HTTPPARSER_CALLBACK& HttpSession::getHttpCallback()
{
    return mHttpRequestCallback;
}

const HttpSession::CLOSE_CALLBACK& HttpSession::getCloseCallback()
{
    return mCloseCallback;
}

const HttpSession::WS_CALLBACK& HttpSession::getWSCallback()
{
    return mWSCallback;
}

const HttpSession::WS_CONNECTED_CALLBACK& HttpSession::getWSConnectedCallback()
{
    return mWSConnectedCallback;
}

void HttpSession::send(const DataSocket::PACKET_PTR& packet, 
    const DataSocket::PACKED_SENDED_CALLBACK& callback /* = nullptr */)
{
    mSession->send(packet, callback);
}

void HttpSession::send(const char* packet, 
    size_t len, 
    const DataSocket::PACKED_SENDED_CALLBACK& callback)
{
    mSession->send(packet, len, callback);
}

void HttpSession::postShutdown() const
{
    mSession->postShutdown();
}

void HttpSession::postClose() const
{
    mSession->postDisConnect();
}

HttpSession::PTR HttpSession::Create(TCPSession::PTR session)
{
    struct make_shared_enabler : public HttpSession
    {
    public:
        make_shared_enabler(TCPSession::PTR session) : HttpSession(std::move(session))
        {}
    };

    return std::make_shared<make_shared_enabler>(std::move(session));
}

void HttpService::setup(const TCPSession::PTR& session, 
    const HttpSession::ENTER_CALLBACK& enterCallback)
{
    auto httpSession = HttpSession::Create(session);
    if (enterCallback != nullptr)
    {
        enterCallback(httpSession);
    }
    HttpService::handle(httpSession);
}

size_t HttpService::ProcessWebSocket(const char* buffer, 
    size_t len,
    const HTTPParser::PTR& httpParser,
    const HttpSession::PTR& httpSession)
{
    size_t retlen = 0;

    const char* parse_str = buffer;
    size_t leftLen = len;

    auto& cacheFrame = httpParser->getWSCacheFrame();
    auto& parseString = httpParser->getWSParseString();

    while (leftLen > 0)
    {
        parseString.clear();

        auto opcode = WebSocketFormat::WebSocketFrameType::ERROR_FRAME;
        size_t frameSize = 0;
        bool isFin = false;
        if (!WebSocketFormat::wsFrameExtractBuffer(parse_str, leftLen, parseString, opcode, frameSize, isFin))
        {
            break;
        }

        if ( isFin &&
            (opcode == WebSocketFormat::WebSocketFrameType::TEXT_FRAME ||
             opcode == WebSocketFormat::WebSocketFrameType::BINARY_FRAME))
        {
            if (!cacheFrame.empty())
            {
                cacheFrame += parseString;
                parseString = std::move(cacheFrame);
                cacheFrame.clear();
            }

            const auto& wsCallback = httpSession->getWSCallback();
            if (wsCallback != nullptr)
            {
                wsCallback(httpSession, opcode, parseString);
            }
        }
        else if (opcode == WebSocketFormat::WebSocketFrameType::CONTINUATION_FRAME)
        {
            cacheFrame += parseString;
            parseString.clear();
        }
        else if (opcode == WebSocketFormat::WebSocketFrameType::PING_FRAME ||
            opcode == WebSocketFormat::WebSocketFrameType::PONG_FRAME ||
            opcode == WebSocketFormat::WebSocketFrameType::CLOSE_FRAME)
        {
            const auto& wsCallback = httpSession->getWSCallback();
            if (wsCallback != nullptr)
            {
                wsCallback(httpSession, opcode, parseString);
            }
        }
        else
        {
            assert(false);
        }

        leftLen -= frameSize;
        retlen += frameSize;
        parse_str += frameSize;
    }

    return retlen;
}

size_t HttpService::ProcessHttp(const char* buffer,
    size_t len,
    const HTTPParser::PTR& httpParser,
    const HttpSession::PTR& httpSession)
{
    const auto retlen = httpParser->tryParse(buffer, len);
    if (!httpParser->isCompleted())
    {
        return retlen;
    }

    if (httpParser->isWebSocket())
    {
        if (httpParser->hasKey("Sec-WebSocket-Key"))
        {
            auto response = WebSocketFormat::wsHandshake(httpParser->getValue("Sec-WebSocket-Key"));
            httpSession->send(response.c_str(), response.size());
        }

        const auto& wsConnectedCallback = httpSession->getWSConnectedCallback();
        if (wsConnectedCallback != nullptr)
        {
            wsConnectedCallback(httpSession, *httpParser);
        }
    }
    else
    {
        const auto& httpCallback = httpSession->getHttpCallback();
        if (httpCallback != nullptr)
        {
            httpCallback(*httpParser, httpSession);
        }
        if (httpParser->isKeepAlive())
        {
            httpParser->clearParse();
        }
    }

    return retlen;
}

void HttpService::handle(const HttpSession::PTR& httpSession)
{
    /*TODO::keep alive and timeout close */
    auto& session = httpSession->getSession();

    session->setDisConnectCallback([httpSession](const TCPSession::PTR& session){
        const auto& tmp = httpSession->getCloseCallback();
        if (tmp != nullptr)
        {
            tmp(httpSession);
        }
    });

    auto httpParser = std::make_shared<HTTPParser>(HTTP_BOTH);
    session->setDataCallback([httpSession, httpParser](
                                const TCPSession::PTR& session, 
                                const char* buffer, size_t len){
        size_t retlen = 0;

        if (httpParser->isWebSocket())
        {
            retlen = HttpService::ProcessWebSocket(buffer, len, httpParser, httpSession);
        }
        else
        {
            retlen = HttpService::ProcessHttp(buffer, len, httpParser, httpSession);
        }

        return retlen;
    });
}
