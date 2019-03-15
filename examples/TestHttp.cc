#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>

#include <tyrantnet/net/SSLHelper.h>
#include <tyrantnet/net/SocketLibFunction.h>
#include <tyrantnet/net/http/HttpService.h>
#include <tyrantnet/net/http/HttpFormat.h>
#include <tyrantnet/net/http/WebSocketFormat.h>

using namespace tyrantnet;
using namespace tyrantnet::net;
using namespace tyrantnet::net::http;

int main(int argc, char **argv)
{
    std::string body = "<html>hello world </html>";

    auto service = TcpService::Create();
    service->startWorkerThread(2);

    auto listenThread = ListenThread::Create();
    listenThread->startListen(false, "127.0.0.1", 8080, [service, body](TcpSocket::Ptr socket) {
        auto enterCallback = [body](const TcpConnection::Ptr& session) {
            HttpService::setup(session, [body](const HttpSession::Ptr& httpSession) {
                httpSession->setHttpCallback([body](const HTTPParser& httpParser,
                    const HttpSession::Ptr& session) {
                    HttpResponse response;
                    response.setBody(body);
                    std::string result = response.getResult();
                    session->send(result.c_str(), result.size(), [session]() {
                        session->postShutdown();
                    });
                });

                httpSession->setWSCallback([](const HttpSession::Ptr& httpSession,
                    WebSocketFormat::WebSocketFrameType opcode,
                    const std::string& payload) {
                    // ping pong
                    auto frame = std::make_shared<std::string>();
                    WebSocketFormat::wsFrameBuild(payload.c_str(),
                        payload.size(),
                        *frame,
                        WebSocketFormat::WebSocketFrameType::TEXT_FRAME,
                        true,
                        true);
                    httpSession->send(frame);
                });
            });
        };
        service->addTcpConnection(std::move(socket),
            tyrantnet::net::TcpService::AddSocketOption::WithEnterCallback(enterCallback),
            tyrantnet::net::TcpService::AddSocketOption::WithMaxRecvBufferSize(10));
    });


    sock fd = tyrantnet::net::base::Connect(false, "191.236.16.125", 80);
    if (fd != INVALID_SOCKET)
    {
        auto socket = TcpSocket::Create(fd, false);
        auto enterCallback = [](const TcpConnection::Ptr& session) {
            HttpService::setup(session, [](const HttpSession::Ptr& httpSession) {
                HttpRequest request;
                request.setMethod(HttpRequest::HTTP_METHOD::HTTP_METHOD_GET);
                request.setUrl("/httpgallery/chunked/chunkedimage.aspx");
                request.addHeadValue("Host", "www.httpwatch.com");

                std::string requestStr = request.getResult();
                httpSession->send(requestStr.c_str(), requestStr.size());
                httpSession->setHttpCallback([](const HTTPParser& httpParser, const HttpSession::Ptr& session) {
                    //http response handle
                    std::cout << httpParser.getBody() << std::endl;
                    std::cout << "len:" << httpParser.getBody().size() << std::endl;
                    std::flush(std::cout);
                });
            });
        };

        service->addTcpConnection(std::move(socket),
            tyrantnet::net::TcpService::AddSocketOption::WithEnterCallback(enterCallback),
            tyrantnet::net::TcpService::AddSocketOption::WithMaxRecvBufferSize(10));
    }

    std::cin.get();
    return 0;
}
