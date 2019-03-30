#include <cstdlib>
#include <iostream>

#include <tyrantnet/net/SocketLibFunction.h>
#include <tyrantnet/net/Socket.h>
#include <tyrantnet/net/SyncConnector.h>

#include <tyrantnet/net/ListenThread.h>

namespace tyrantnet { namespace net {

    ListenThread::Ptr ListenThread::Create(bool isIPV6, const std::string& ip, const int port, const AcceptCallback& callback)
    {
        struct make_shared_enabler : public ListenThread
        {
            make_shared_enabler(bool isIPV6, const std::string& ip, const int port, const AcceptCallback& callback)
                : ListenThread(isIPV6, ip, port, callback)
            {
            }
        };
        return std::make_shared<make_shared_enabler>(isIPV6, ip, port, callback);
    }

    ListenThread::ListenThread(bool isIPV6, const std::string& ip, const int port, const AcceptCallback& callback)
        : mIsIPV6(isIPV6),
        mIP(ip),
        mPort(port),
        mCallback(callback),
        mRunListen(std::make_shared<bool>(false)),
        mListenThread(nullptr)
    {

        if (callback == nullptr)
        {
            throw std::runtime_error("accept callback is nullptr");
        }
    }

    ListenThread::~ListenThread()
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

    void ListenThread::startListen()
    {
        std::lock_guard<std::mutex> lck(mListenThreadGuard);

        if (mListenThread != nullptr)
        {
            return;
        }
        const sock fd = tyrantnet::net::base::Listen(mIsIPV6, mIP.c_str(), mPort, 512);
        if (fd == INVALID_SOCKET)
        {
            throw std::runtime_error(std::string("listen error of:") + std::to_string(sErrno));;
        }

        mRunListen			= std::make_shared<bool>(true);

        auto listenSocket	= std::shared_ptr<ListenSocket>(ListenSocket::Create(fd));
        auto isRunListen	= mRunListen;
        auto callback		= mCallback;

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
