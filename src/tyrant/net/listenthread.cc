#include <tyrant/net/listenthread.h>

#include <cstdlib>
#include <iostream>

#include <tyrant/net/socketlibfunction.h>
#include <tyrant/net/noexcept.h>
#include <tyrant/net/socket.h>

using namespace tyrant;
using namespace tyrant::common;
using namespace tyrant::net;

ListenThread::PTR ListenThread::Create()
{
    struct make_shared_enabler : public ListenThread {};
    return std::make_shared<make_shared_enabler>();
}

ListenThread::ListenThread() TYRANT_NOEXCEPT
{
    mIsIPV6 = false;
    mPort = 0;
    mRunListen = std::make_shared<bool>(false);
}
ListenThread::~ListenThread() TYRANT_NOEXCEPT
{
    stopListen();
}

static TcpSocket::PTR runOnceListen(const std::shared_ptr<ListenSocket>& listenSocket)
{
    try
    {
        auto clientSocket = listenSocket->Accept();
        return clientSocket;
    }
    catch (const EintrError& e)
    {
        std::cerr << "accept eintr execption:" << e.what() << std::endl;
    }
    catch (const AcceptError& e)
    {
        std::cerr << "accept execption:" << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "accept unknow execption:" << std::endl;
    }

    return nullptr;
}

void ListenThread::startListen(bool isIPV6, const std::string& ip, int port, ACCEPT_CALLBACK callback)
{
    std::lock_guard<std::mutex> lck(mListenThreadGuard);

    if (mListenThread != nullptr)
    {
        return;
    }
    if (callback == nullptr)
    {
        throw std::runtime_error("accept callback is nullptr");
    }

    sock fd = base::Listen(isIPV6, ip.c_str(), port, 512);
    if (fd == SOCKET_ERROR)
    {
        throw std::runtime_error("listen error of:" + sErrno);
    }

    mIsIPV6 = isIPV6;
    mRunListen = std::make_shared<bool>(true);
    mIP = ip;
    mPort = port;

    auto listenSocket = std::shared_ptr<ListenSocket>(ListenSocket::Create(fd));
    auto isRunListen = mRunListen;

    mListenThread = std::make_shared<std::thread>([isRunListen, listenSocket, callback]() mutable {
        while (*isRunListen)
        {
            auto clientSocket = runOnceListen(listenSocket);
            if (clientSocket == nullptr)
            {
                continue;
            }

            if (*isRunListen)
            {
                callback(std::move(clientSocket));
            }
        }
    });
}

void ListenThread::stopListen()
{
    std::lock_guard<std::mutex> lck(mListenThreadGuard);

    if (mListenThread == nullptr)
    {
        return;
    }

    *mRunListen = false;
    auto selfIP = mIP;
    if (selfIP == "0.0.0.0")
    {
        selfIP = "127.0.0.1";
    }

    try
    {
        if (mListenThread->joinable())
        {
            mListenThread->join();
        }
    }
    catch(...)
    { }
    mListenThread = nullptr;
}
