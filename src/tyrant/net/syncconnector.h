#ifndef __TYRANTNET_NET_SYNC_CONNECT_H__
#define __TYRANTNET_NET_SYNC_CONNECT_H__

#include <chrono>
#include <string>
#include <vector>

#include <tyrant/net/wraptcpservice.h>
#include <tyrant/net/connector.h>
#include <tyrant/net/socket.h>

namespace tyrant
{
    namespace net
    {
        TcpSocket::PTR SyncConnectSocket(const std::string& ip,
            int port,
            std::chrono::milliseconds timeout,
            AsyncConnector::PTR asyncConnector = nullptr);

        TCPSession::PTR SyncConnectSession(const std::string& ip,
            int port,
            std::chrono::milliseconds timeout,
            WrapTcpService::PTR service,
            const std::vector<AddSessionOption::AddSessionOptionFunc>& options,
            AsyncConnector::PTR asyncConnector = nullptr);
    } // net
} // tyrant

#endif //__TYRANTNET_NET_SYNC_CONNECT_H__
