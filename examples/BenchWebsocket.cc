#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <tyrantnet/common/packet.h>
#include <tyrantnet/net/SocketLibFunction.h>
#include <tyrantnet/net/http/HttpService.h>
#include <tyrantnet/net/http/HttpFormat.h>
#include <tyrantnet/net/http/WebSocketFormat.h>


using namespace tyrantnet;
using namespace tyrantnet::common;
using namespace tyrantnet::net;
using namespace tyrantnet::net::http;

std::atomic<int32_t> g_count;

static void sendPacket(HttpSession::Ptr session, const char* data, size_t len)
{
    char buff[1024];
    BasePacketWriter bw(buff, sizeof(buff), true, true);
    bw.writeINT8('{');
    bw.writeBuffer("\"data\":\"", 8);
    bw.writeBuffer(data, len);
    bw.writeINT8('"');
    bw.writeINT8('}');

    auto frame = std::make_shared<std::string>();
    WebSocketFormat::wsFrameBuild(bw.getData(),
        bw.getPos(), 
        *frame, 
        WebSocketFormat::WebSocketFrameType::TEXT_FRAME, 
        true, 
        true);
    session->send(frame);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "./benchwebsocket host port [ connections workers ]";
        return 1;
    }
    
    const char* host = argv[1];
    int port = std::atoi(argv[2]);
    int connections = argc > 3 ? std::atoi(argv[3]) : 200;
    size_t workers = argc > 4 ? std::atoi(argv[4]) : std::thread::hardware_concurrency();
    
    std::cout << "host: " << host << ':' << port << " | connections: " << connections << " | workers: " << workers << std::endl;
    
    auto service = TcpService::Create();
    service->startWorkerThread(workers);

    for (int i = 0; i < connections; i++)
    {
        sock fd = tyrantnet::net::base::Connect(false, host, port);
        auto socket = TcpSocket::Create(fd, false);
        tyrantnet::net::base::SocketNodelay(fd);

        auto enterCallback = [host](const TcpConnection::Ptr& session) {
            HttpService::setup(session, [host](const HttpSession::Ptr& httpSession) {
                HttpRequest request;
                request.setMethod(HttpRequest::HTTP_METHOD::HTTP_METHOD_GET);
                request.setUrl("/ws");
                request.addHeadValue("Host", host);
                request.addHeadValue("Upgrade", "websocket");
                request.addHeadValue("Connection", "Upgrade");
                request.addHeadValue("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ==");
                request.addHeadValue("Sec-WebSocket-Version", "13");

                std::string requestStr = request.getResult();
                httpSession->send(requestStr.c_str(), requestStr.size());

                httpSession->setWSConnected([](const HttpSession::Ptr& session, const HTTPParser&) {
                    for (int i = 0; i < 200; i++)
                    {
                        sendPacket(session, "hello, world!", 13);
                    }
                });

                httpSession->setWSCallback([](const HttpSession::Ptr& session,
                    WebSocketFormat::WebSocketFrameType, const std::string& payload) {
                    std::cout << payload << std::endl;
                    sendPacket(session, "hello, world!", 13);
                    g_count += 1;
                });
            });
        };

        service->addTcpConnection(std::move(socket),
            tyrantnet::net::TcpService::AddSocketOption::WithEnterCallback(enterCallback),
            tyrantnet::net::TcpService::AddSocketOption::WithMaxRecvBufferSize(1024*1024));
    }

    tyrantnet::net::EventLoop mainLoop;
    while (true)
    {
        mainLoop.loop(5000);
        std::cout << (g_count / 5) << std::endl;
        g_count = 0;
    }
    std::cin.get();
}
