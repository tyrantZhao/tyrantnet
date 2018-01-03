#include <tyrant/net/listenthread.h>

using namespace ::tyrant;
using namespace ::tyrant::net;

ListenThread::PTR ListenThread::Create()
{
    struct make_shared_enabler : public ListenThread {};
    return ::std::make_shared<make_shared_enabler>();
}

ListenThread::ListenThread() TYRANT_NOEXCEPT
{
    mIsIPV6 = false;
    mAcceptCallback = nullptr;
    mPort = 0;
    mRunListen = false;
}

ListenThread::~ListenThread() TYRANT_NOEXCEPT
{
    stopListen();
}

void ListenThread::startListen(bool isIPV6, 
    const std::string& ip,
    int port,
    ACCEPT_CALLBACK callback)
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

    // TODO::socket leak
    sock fd = base::Listen(isIPV6, ip.c_str(), port, 512);
    if (SOCKET_ERROR == fd)
    {
        throw std::runtime_error("listen error of:");
    }

    mIsIPV6 = isIPV6;
    mRunListen = true;
    mIP = ip;
    mPort = port;
    mAcceptCallback = callback;

    auto shared_this = shared_from_this();
    mListenThread = std::make_shared<std::thread>([shared_this, fd]() {
        shared_this->runListen(fd);
        base::SocketClose(fd);
    });
}

void ListenThread::stopListen()
{
    std::lock_guard<std::mutex> lck(mListenThreadGuard);

    if (mListenThread == nullptr)
    {
        return;
    }

    mRunListen = false;

    sock tmp = base::Connect(mIsIPV6, mIP.c_str(), mPort);
    base::SocketClose(tmp);
    tmp = SOCKET_ERROR;

    if (mListenThread->joinable())
    {
        mListenThread->join();
    }
    mListenThread = nullptr;
}

void ListenThread::runListen(sock fd)
{
    struct sockaddr_in socketaddress;
    struct sockaddr_in6 ip6Addr;
    socklen_t addrLen = sizeof(struct sockaddr);
    sockaddr_in* pAddr = &socketaddress;

    if (mIsIPV6)
    {
        addrLen = sizeof(ip6Addr);
        pAddr = (sockaddr_in*)&ip6Addr;
    }

    for (; mRunListen;)
    {
        sock client_fd = SOCKET_ERROR;
        while ((client_fd = base::Accept(fd, (struct sockaddr*)pAddr, &addrLen)) == SOCKET_ERROR)
        {
            if (EINTR == sErrno)
            {
                continue;
            }
        }

        if (SOCKET_ERROR == client_fd)
        {
            continue;
        }
        if (!mRunListen)
        {
            base::SocketClose(client_fd);
            continue;
        }

        mAcceptCallback(client_fd);
    }
}