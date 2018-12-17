#include <cstdlib>
#include <iostream>
#include <mutex>
#include <atomic>

#include <tyrant/net/socketlibfunction.h>
#include <tyrant/net/eventloop.h>
#include <tyrant/net/listenthread.h>
#include <tyrant/net/wraptcpservice.h>
#include <tyrant/net/promisereceive.h>
#include <tyrant/net/http/HttpFormat.h>

using namespace tyrant::net;

int main(int argc, char **argv)
{
    if (4 != argc)
    {
        std::cerr << "Usage: ./promiseReceive_test  <ip_address> <listen_port> <net_thread_num>" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    auto ip_address         = argv[1];
    auto listen_port        = std::atoi(argv[2]);
    auto thread_work_num    = std::atoi(argv[3]);

    auto server = std::make_shared<WrapTcpService>();
    auto listen_thread = ListenThread::Create();

    listen_thread->startListen(false, ip_address, listen_port, [=](TcpSocket::PTR socket)
    {
        socket->SocketNodelay();
        auto enterCallback = [](const TCPSession::PTR& session) {

            auto promiseReceive = setupPromiseReceive(session);
            auto contentLength = std::make_shared<std::size_t>();

            promiseReceive->receiveUntil("\r\n", [](const char* buffer, size_t len)
            {
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
                        *contentLength = static_cast<std::size_t>(std::stoi(lenStr));
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
        server->addSession(std::move(socket),
            AddSessionOption::WithEnterCallback(enterCallback),
            AddSessionOption::WithMaxRecvBufferSize(10));
    });

    server->startWorkThread(thread_work_num);

    EventLoop mainLoop;
    while (true)
    {
        mainLoop.loop(1000);
    }
}
