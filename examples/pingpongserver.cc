#include <iostream>
#include <mutex>
#include <atomic>

#include <tyrant/net/socketlibfunction.h>
#include <tyrant/net/eventloop.h>
#include <tyrant/net/wraptcpservice.h>
#include <tyrant/net/listenthread.h>
#include <tyrant/net/socket.h>

using namespace tyrant;
using namespace tyrant::net;

std::atomic_llong TotalRecvSize = ATOMIC_VAR_INIT(0);
std::atomic_llong total_client_num = ATOMIC_VAR_INIT(0);
std::atomic_llong total_packet_num = ATOMIC_VAR_INIT(0);

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: <listen port> <net work thread num>\n");
        exit(-1);
    }

    auto server = std::make_shared<WrapTcpService>();
    auto listenThread = ListenThread::Create();

    listenThread->startListen(false, "0.0.0.0", atoi(argv[1]), [=](TcpSocket::PTR socket){
        socket->SocketNodelay();

        auto enterCallback = [](const TCPSession::PTR& session) {
            total_client_num++;

            session->setDataCallback([](const TCPSession::PTR& session, const char* buffer, size_t len) {
                session->send(buffer, len);
                TotalRecvSize += len;
                total_packet_num++;
                return len;
            });

            session->setDisConnectCallback([](const TCPSession::PTR& session) {
                total_client_num--;
            });
        };

        server->addSession(std::move(socket),
                AddSessionOption::WithEnterCallback(enterCallback),
                AddSessionOption::WithMaxRecvBufferSize(1024*1024));
    });

    server->startWorkThread(atoi(argv[2]));

    EventLoop mainLoop;
    while (true)
    {
        mainLoop.loop(1000);
        std::cout << "total recv : " << (TotalRecvSize / 1024) / 1024 << " M /s, of client num:" << total_client_num << std::endl;
        std::cout << "packet num:" << total_packet_num << std::endl;
        total_packet_num = 0;
        TotalRecvSize = 0;
    }
}
