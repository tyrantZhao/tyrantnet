#include <iostream>
#include <mutex>
#include <atomic>

#include <tyrantnet/net/SocketLibFunction.h>
#include <tyrantnet/net/Socket.h>
#include <tyrantnet/net/EventLoop.h>
#include <tyrantnet/net/ListenThread.h>
#include <tyrantnet/net/TCPService.h>


using namespace tyrantnet;
using namespace tyrantnet::net;

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

    auto server = TcpService::Create();
    auto listenThread = ListenThread::Create();

    listenThread->startListen(false, "127.0.0.1", atoi(argv[1]), [=](TcpSocket::Ptr socket){
        socket->setNodelay();

        auto enterCallback = [](const TcpConnection::Ptr& session) {
            total_client_num++;

            session->setDataCallback([session](const char* buffer, size_t len) {
                session->send(buffer, len);
                TotalRecvSize += len;
                total_packet_num++;
                return len;
            });

            session->setDisConnectCallback([](const TcpConnection::Ptr& session) {
                total_client_num--;
            });
        };

        server->addTcpConnection(std::move(socket),
                                 TcpService::AddSocketOption::WithEnterCallback(enterCallback),
                                 TcpService::AddSocketOption::WithMaxRecvBufferSize(1024 * 1024));
    });

    server->startWorkerThread(atoi(argv[2]));

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
