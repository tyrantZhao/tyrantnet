﻿#include <functional>
#include <time.h>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <assert.h>
#include <chrono>
#include <vector>
#include <atomic>

#include <tyrantnet/common/packet.h>
#include <tyrantnet/net/SocketLibFunction.h>

#include <tyrantnet/net/EventLoop.h>
#include <tyrantnet/net/TcpConnection.h>
#include <tyrantnet/net/TCPService.h>
#include <tyrantnet/net/ListenThread.h>

using namespace tyrantnet;
using namespace tyrantnet::common;
using namespace tyrantnet::net;

std::atomic_llong TotalSendLen = ATOMIC_VAR_INIT(0);
std::atomic_llong TotalRecvLen = ATOMIC_VAR_INIT(0);

std::atomic_llong  SendPacketNum = ATOMIC_VAR_INIT(0);
std::atomic_llong  RecvPacketNum = ATOMIC_VAR_INIT(0);

std::vector<TcpConnection::Ptr> clients;
TcpService::Ptr service;

static void addClientID(const TcpConnection::Ptr& session)
{
    clients.push_back(session);
}

static void removeClientID(const TcpConnection::Ptr& session)
{
    for (auto it = clients.begin(); it != clients.end(); ++it)
    {
        if (*it == session)
        {
            clients.erase(it);
            break;
        }
    }
}

static size_t getClientNum()
{
    return clients.size();
}

static void broadCastPacket(const TcpConnection::PacketPtr& packet)
{
    auto packetLen = packet->size();
    RecvPacketNum++;
    TotalRecvLen += packetLen;

    for (const auto& session : clients)
    {
        session->send(packet);
    }

    SendPacketNum += clients.size();
    TotalSendLen += (clients.size() * packetLen);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage : <listen port> \n");
        exit(-1);
    }

    service = TcpService::Create();
    auto mainLoop = std::make_shared<EventLoop>();
    auto listenThread = ListenThread::Create(false, "127.0.0.1", atoi(argv[1]), [mainLoop](TcpSocket::Ptr socket) {
        socket->setNodelay();
        socket->setSendSize(32 * 1024);
        socket->setRecvSize(32 * 1024);

        auto enterCallback = [mainLoop](const TcpConnection::Ptr& session) {
            mainLoop->runAsyncFunctor([session]() {
                addClientID(session);
            });

            session->setDisConnectCallback([mainLoop](const TcpConnection::Ptr& session) {
                mainLoop->runAsyncFunctor([session]() {
                    removeClientID(session);
                });
            });

            session->setDataCallback([mainLoop](const char* buffer, size_t len) {
                const char* parseStr = buffer;
                size_t totalProcLen = 0;
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
                            auto packet = TcpConnection::makePacket(parseStr, packet_len);
                            mainLoop->runAsyncFunctor([packet]() {
                                broadCastPacket(packet);
                            });

                            totalProcLen += packet_len;
                            parseStr += packet_len;
                            leftLen -= packet_len;
                            flag = true;
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
        };
        service->addTcpConnection(std::move(socket),
            tyrantnet::net::TcpService::AddSocketOption::WithEnterCallback(enterCallback),
                tyrantnet::net::TcpService::AddSocketOption::WithMaxRecvBufferSize(1024 * 1024));
    });

    listenThread->startListen();

    service->startWorkerThread(2);

    auto now = std::chrono::steady_clock::now();
    while (true)
    {
        mainLoop->loop(10);
        auto diff = std::chrono::steady_clock::now() - now;
        if (diff >= std::chrono::seconds(1))
        {
            auto msDiff = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
            std::cout << "cost " << 
                msDiff << " ms, clientnum:" << 
                getClientNum() << ", recv" <<
                (TotalRecvLen / 1024) * 1000 / msDiff  << 
                " K/s, " << "num : " << 
                RecvPacketNum * 1000 / msDiff << ", send " <<
                (TotalSendLen / 1024) / 1024 * 1000 / msDiff <<
                " M/s, " << " num: " << 
                SendPacketNum * 1000 / msDiff << std::endl;
            TotalRecvLen = 0;
            TotalSendLen = 0;
            RecvPacketNum = 0;
            SendPacketNum = 0;
            now = std::chrono::steady_clock::now();
        }
    }

    service->stopWorkerThread();

    return 0;
}
