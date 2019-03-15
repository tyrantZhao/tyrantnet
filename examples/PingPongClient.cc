#include <iostream>
#include <string>

#include <tyrantnet/net/SocketLibFunction.h>
#include <tyrantnet/net/TCPService.h>
#include <tyrantnet/net/Connector.h>

using namespace tyrantnet::net;

int main(int argc, char **argv)
{
    if (argc != 6)
    {
        fprintf(stderr, "Usage: <host> <port> <net work thread num> <session num> <packet size> \n");
        exit(-1);
    }

    std::string tmp(atoi(argv[5]), 'a');

    auto server = TcpService::Create();
    server->startWorkerThread(atoi(argv[3]));

    auto connector = AsyncConnector::Create();
    connector->startWorkerThread();

    for (auto i = 0; i < atoi(argv[4]); i++)
    {
        try
        {
            connector->asyncConnect(argv[1], atoi(argv[2]), std::chrono::seconds(10), [server, tmp](TcpSocket::Ptr socket) {
                std::cout << "connect success" << std::endl;
                socket->setNodelay();

                auto enterCallback = [tmp](const TcpConnection::Ptr& session) {
                    session->setDataCallback([session](const char* buffer, size_t len) {
                        session->send(buffer, len);
                        return len;
                    });
                    session->send(tmp.c_str(), tmp.size());
                };

                server->addTcpConnection(std::move(socket),
                                         TcpService::AddSocketOption::WithEnterCallback(enterCallback),
                                         TcpService::AddSocketOption::WithMaxRecvBufferSize(1024 * 1024));
            }, []() {
                std::cout << "connect failed" << std::endl;
            });
        }
        catch (std::runtime_error& e)
        {
            std::cout << "error:" << e.what() << std::endl;
        }
    }

    std::cin.get();
}
