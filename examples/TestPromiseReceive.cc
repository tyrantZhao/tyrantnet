#include <iostream>
#include <mutex>
#include <atomic>

#include <tyrantnet/net/SocketLibFunction.h>
#include <tyrantnet/net/EventLoop.h>
#include <tyrantnet/net/TCPService.h>
#include <tyrantnet/net/PromiseReceive.h>
#include <tyrantnet/net/http/HttpFormat.h>
#include <tyrantnet/net/ListenThread.h>

using namespace tyrantnet;
using namespace tyrantnet::net;
using namespace tyrantnet::net::http;

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: <listen port> <net work thread num>\n");
        exit(-1);
    }

    auto server = TcpService::Create();
    auto listenThread = ListenThread::Create();

    listenThread->startListen(false, "0.0.0.0", atoi(argv[1]), [=](TcpSocket::Ptr socket){
        socket->setNodelay();
        auto enterCallback = [](const TcpConnection::Ptr& session) {
            auto promiseReceive = setupPromiseReceive(session);
            auto contentLength = std::make_shared<size_t>();

            promiseReceive->receiveUntil("\r\n", [](const char* buffer, size_t len) {
                auto headline = std::string(buffer, len);
                std::cout << headline << std::endl;
                return false;
            })->receiveUntil("\r\n", [promiseReceive, contentLength](const char* buffer, size_t len) {
                auto headerValue = std::string(buffer, len);
                std::cout << headerValue << std::endl;
                if (len > 2)
                {
                    const static std::string ContentLenghtFlag = "Content-Length: ";
                    auto pos = headerValue.find(ContentLenghtFlag);
                    if (pos != std::string::npos)
                    {
                        auto lenStr = headerValue.substr(pos+ ContentLenghtFlag.size(), headerValue.size());
                        *contentLength = std::stoi(lenStr);
                    }
                    return true;
                }
                return false;
            })->receive(contentLength, [session](const char* buffer, size_t len) {
                HttpResponse response;
                response.setStatus(HttpResponse::HTTP_RESPONSE_STATUS::OK);
                response.setContentType("text/html; charset=utf-8");
                response.setBody("<html>hello world </html>");

                auto result = response.getResult();
                session->send(result.c_str(), result.size());
                session->postShutdown();

                return false;
            });
        };
        server->addTcpConnection(std::move(socket),
            tyrantnet::net::TcpService::AddSocketOption::WithEnterCallback(enterCallback),
            tyrantnet::net::TcpService::AddSocketOption::WithMaxRecvBufferSize(10));
    });

    server->startWorkerThread(atoi(argv[2]));

    EventLoop mainLoop;
    while (true)
    {
        mainLoop.loop(1000);
    }
}
