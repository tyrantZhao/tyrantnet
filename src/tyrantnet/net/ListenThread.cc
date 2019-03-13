#include <cstdlib>
#include <iostream>

#include <tyrantnet/net/SocketLibFunction.h>
#include <tyrantnet/net/Noexcept.h>
#include <tyrantnet/net/Socket.h>
#include <tyrantnet/net/SyncConnector.h>

#include <tyrantnet/net/ListenThread.h>

namespace tyrantnet { namespace net {

    ListenThread::Ptr ListenThread::Create()
    {
        struct make_shared_enabler : public ListenThread {};
        return std::make_shared<make_shared_enabler>();
    }

    ListenThread::ListenThread() TYRANTNET_NOEXCEPT
    {
        mIsIPV6 = false;
        mPort = 0;
        mRunListen = std::make_shared<bool>(false);
    }

    ListenThread::~ListenThread() TYRANTNET_NOEXCEPT
    {
        stopListen();
    }

    static tyrantnet::net::TcpSocket::Ptr runOnceListen(const std::shared_ptr<ListenSocket>& listenSocket)
    {
        try
        {
            auto clientSocket = listenSocket->accept();
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

    void ListenThread::startListen(bool isIPV6,
        const std::string& ip,
        int port,
        AccepCallback callback)
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

        const sock fd = tyrantnet::net::base::Listen(isIPV6, ip.c_str(), port, 512);
        if (fd == INVALID_SOCKET)
        {
            throw std::runtime_error(std::string("listen error of:") + std::to_string(sErrno));;
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
        tyrantnet::net::SyncConnectSocket(selfIP, mPort, std::chrono::seconds(10));

        try
        {
            if (mListenThread->joinable())
            {
                mListenThread->join();
            }
        }
        catch (...)
        {
        }
        mListenThread = nullptr;
    }

} }