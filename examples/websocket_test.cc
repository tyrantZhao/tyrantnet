#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <tyrant/common/packet.h>
#include <tyrant/net/socketlibfunction.h>
#include <tyrant/net/http/HttpService.h>
#include <tyrant/net/http/HttpFormat.h>
#include <tyrant/net/http/WebSocketFormat.h>

using namespace tyrant;
using namespace tyrant::common;
using namespace tyrant::net;

std::atomic<int32_t> count;

static void sendPacket(HttpSession::PTR session, const char* data, size_t len)
{
    char buff[1024];
    BasePacketWriter bw(buff, sizeof(buff), true, true);
    bw.writeINT8('{');
    bw.writeBuffer("\"data\":\"", 8);
    bw.writeBuffer(data, len);
    bw.writeINT8('"');
    bw.writeINT8('}');

    auto frame = std::make_shared<std::string>();
    WebSocketFormat::wsFrameBuild(bw.getData(), bw.getPos(), *frame, WebSocketFormat::WebSocketFrameType::TEXT_FRAME, true, true);
    session->send(frame);
}

void BenchmarkWebsocket(const char* host, int port, int connections, std::size_t workers)
{
    auto service = std::make_shared<WrapTcpService>();
    service->startWorkThread(workers);

    for (int i = 0; i < connections; i++)
    {
        sock fd = base::Connect(false, host, port);
        auto socket = TcpSocket::Create(fd, false);
        socket->SocketNodelay();

        auto enterCallback = [host](const TCPSession::PTR& session) {
            HttpService::setup(session, [host](const HttpSession::PTR& httpSession) {
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

                httpSession->setWSConnected([](const HttpSession::PTR& session, const HTTPParser&) {
                    for (int i = 0; i < 200; i++)
                    {
                        sendPacket(session, "hello, world!", 13);
                    }
                });

                httpSession->setWSCallback([](const HttpSession::PTR& session,
                                              WebSocketFormat::WebSocketFrameType, const std::string& payload) {
                    std::cout << payload << std::endl;
                    sendPacket(session, "hello, world!", 13);
                    count += 1;
                });
            });
        };

        service->addSession(std::move(socket),
                            AddSessionOption::WithEnterCallback(enterCallback),
                            AddSessionOption::WithMaxRecvBufferSize(1024*1024));
    }

    EventLoop mainLoop;
    while (true)
    {
        mainLoop.loop(5000);
        std::cout << (count / 5) << std::endl;
        count = 0;
    }
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: ./websocket_test host port [ connections workers ]");
        return(EXIT_FAILURE);
    }

    const char* host = argv[1];
    int port = std::atoi(argv[2]);
    int connections = argc > 3 ? std::atoi(argv[3]) : 200;
    size_t workers = argc > 4 ? std::atoi(argv[4]) : std::thread::hardware_concurrency();

    std::cout << "host: " << host << ':' << port << " | connections: " << connections << " | workers: " << workers << std::endl;

    BenchmarkWebsocket(host, port, connections, workers);
    std::cin.get();
}
