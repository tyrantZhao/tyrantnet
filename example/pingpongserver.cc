#include <cstdio>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

#include <tyrant/net/socketlibfunction.h>
#include <tyrant/net/wraptcpservice.h>
#include <tyrant/net/eventloop.h>
#include <tyrant/net/listenthread.h>

using namespace tyrant;
using namespace tyrant::net;

::std::atomic_llong nRecvBufferSize    = ATOMIC_VAR_INIT(0);
::std::atomic_llong nTotalClientCnt    = ATOMIC_VAR_INIT(0);
::std::atomic_llong nTotalPackCnt      = ATOMIC_VAR_INIT(0);

int main(int argc, char** argv)
{
    if (3 != argc) {
        fprintf(stderr, "Usage : %s <listen-port> <thread-cnt>", argv[0]);
        return EXIT_FAILURE;
    }

    int nListenPort     = atoi(argv[1]);
    int nThreadCnt      = atoi(argv[2]);
    auto pServer        = ::std::make_shared<WrapTcpService>();
    auto pListenThread  = ListenThread::Create();

    pListenThread->startListen(false, "0.0.0.0", nListenPort, [=](sock fd){
        base::SocketNodelay(fd);
        pServer->addSession(fd, [](const TCPSession::PTR& session){
            nTotalClientCnt++;

            session->setDataCallback([](const TCPSession::PTR& session, const char* buffer, size_t len){
                session->send(buffer, len);
                nRecvBufferSize += len;
                nTotalPackCnt++;
                return len;
            });

            session->setDisConnectCallback([](const TCPSession::PTR& session){
                nTotalClientCnt--;
            });
        }, false, nullptr, 1024*1024);
    });

    pServer->startWorkThread(nThreadCnt);
    EventLoop oMainLoop;
    while(true) {
        oMainLoop.loop(1000);
        ::std::cout << "total recv : " << (nRecvBufferSize / 1024) / 1024 << " M /s, of client num:" << nTotalClientCnt << ::std::endl;
        ::std::cout << "nTotalPackCnt : " << nTotalPackCnt << ::std::endl;
        nRecvBufferSize = 0;
        nTotalPackCnt   = 0;
    }
}
