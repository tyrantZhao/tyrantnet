#include <iostream>
#include <ctime>
#include <cstdio>
#include <functional>
#include <thread>
#include <assert.h>
#include <chrono>
#include <memory>
#include <thread>
#include <atomic>

#include <tyrant/common/packet.h>
#include <tyrant/timer/timer.h>

#include <tyrant/net/socketlibfunction.h>

#include <tyrant/net/eventloop.h>
#include <tyrant/net/datasocket.h>
#include <brynet/timer/Timer.h>

using namespace brynet;
using namespace brynet::net;

std::atomic_llong TotalRecvPacketNum = std::ATOMIC_VAR_INIT(0);
std::atomic_llong TotalRecvSize = std::ATOMIC_VAR_INIT(0);

int main(int argc, char** argv)
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: <server ip> <server port> <session num> <packet size>\n");
        return -1;
    }

    std::string ip = argv[1];
    int port = atoi(argv[2]);
    int clientNum = atoi(argv[3]);
    int packetLen = atoi(argv[4]);

    base::InitSocket();

    auto clientEventLoop = std::make_shared<EventLoop>();

    for (int i = 0; i < clientNum; i++)
    {
        auto fd = base::Connect(false, ip.c_str(), port);
        base::SocketSetSendSize(fd, 32 * 1024);
        base::SocketSetRecvSize(fd, 32 * 1024);
        base::SocketNodelay(fd);

        DataSocket::PTR datasSocket = new DataSocket(TcpSocket::Create(fd, false), 1024 * 1024);
        datasSocket->setEnterCallback([packetLen](DataSocket::PTR datasSocket) {
            static_assert(sizeof(datasSocket) <= sizeof(int64_t), "dataSocket size > int64_t");

            auto HEAD_LEN = sizeof(uint32_t) + sizeof(uint16_t);

            std::shared_ptr<BigPacket> sp = std::make_shared<BigPacket>(1);
            sp->writeUINT32(HEAD_LEN+sizeof(int64_t) + packetLen);
            sp->writeUINT16(1);
            sp->writeINT64((int64_t)datasSocket);
            sp->writeBinary(std::string(packetLen, '_'));

            for (int i = 0; i < 1; ++i)
            {
                datasSocket->send(sp->getData(), sp->getPos());
            }

            datasSocket->setDataCallback([](DataSocket::PTR datasSocket, const char* buffer, size_t len) {
                const char* parseStr = buffer;
                int totalProcLen = 0;
                size_t leftLen = len;

                while (true)
                {
                    bool flag = false;
                    auto HEAD_LEN = sizeof(uint32_t) + sizeof(uint16_t);
                    if (leftLen >= HEAD_LEN)
                    {
                        BasePacketReader rp(parseStr, leftLen);
                        auto packet_len = rp.readUINT32();
                        if (leftLen >= packet_len && packet_len >= HEAD_LEN)
                        {
                            TotalRecvSize += packet_len;
                            TotalRecvPacketNum++;

                            BasePacketReader rp(parseStr, packet_len);
                            rp.readUINT32();
                            rp.readUINT16();
                            int64_t addr = rp.readINT64();

                            if (addr == (int64_t)(datasSocket))
                            {
                                datasSocket->send(parseStr, packet_len);
                            }

                            totalProcLen += packet_len;
                            parseStr += packet_len;
                            leftLen -= packet_len;
                            flag = true;
                            rp.skipAll();
                        }
                        rp.skipAll();
                    }

                    if (!flag)
                    {
                        break;
                    }
                }

                return totalProcLen;
            });

            datasSocket->setDisConnectCallback([](DataSocket::PTR datasSocket) {
                delete datasSocket;
            });
        });

        clientEventLoop->pushAsyncProc([clientEventLoop, datasSocket]() {
            if (!datasSocket->onEnterEventLoop(clientEventLoop))
            {
                delete datasSocket;
            }
        });
    }

    auto now = std::chrono::steady_clock::now();
    while (true)
    {
        clientEventLoop->loop(10);
        if ((std::chrono::steady_clock::now() - now) >= std::chrono::seconds(1))
        {
            cout << "total recv:" << (TotalRecvSize / 1024) / 1024 << " M /s" << " , num " <<  TotalRecvPacketNum << endl;

            now = std::chrono::steady_clock::now();
            TotalRecvSize = 0;
            TotalRecvPacketNum = 0;
        }
    }

    return 0;
}
