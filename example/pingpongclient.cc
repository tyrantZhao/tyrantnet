#include <iostream>
#include <string>

#include <tyrant/net/socketlibfunction.h>
#include <tyrant/net/wraptcpservice.h>
#include <tyrant/net/connector.h>

using namespace ::tyrant;
using namespace ::tyrant::net;

int main(int argc, char **argv)
{
    if (6 != argc) {
        perror("Usage: <host> <port> <net work thread num> <session num> <packet size> \n");
        exit(EXIT_FAILURE);
    }

    ::std::string sTmp(atoi(argv[5]), 'a');

    auto pServer = ::std::make_shared<WrapTcpService>();
    pServer->startWorkThread(atoi(argv[3]));

    auto pAsyncConnector = AsyncConnector::Create();
    pAsyncConnector->startWorkerThread();

    for (auto i = 0; i < atoi(argv[4]); i++) {
        try {
            pAsyncConnector->asyncConnect(argv[1], atoi(argv[2]), ::std::chrono::seconds(10), [pServer, sTmp](sock nSockfd) {
                ::std::cout << "connect success\n" << ::std::endl;
                base::SocketNonblock(nSockfd);
                pServer->addSession(nSockfd, [sTmp](const TCPSession::PTR& pSession) {
                    pSession->setDataCallback([](const TCPSession::PTR& session, const char* buffer, size_t len) {
                        session->send(buffer, len);
                        return len;
                    });
                    pSession->send(sTmp.c_str(), sTmp.size());
                }, false, nullptr, 1024 * 1024); }, []() {
                ::std::cout << "connected failed" << ::std::endl;
            });
        } catch (::std::runtime_error& e) {
            ::std::cout << "error:" << e.what() << std::endl;
        }
    }

    ::std::cin.get();
}

