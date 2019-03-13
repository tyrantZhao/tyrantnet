#pragma once

#include <chrono>
#include <string>
#include <vector>

#include <tyrantnet/net/TCPService.h>
#include <tyrantnet/net/Connector.h>

namespace tyrantnet { namespace net {

    TcpSocket::Ptr SyncConnectSocket(std::string ip,
        int port,
        std::chrono::milliseconds timeout,
        tyrantnet::net::AsyncConnector::Ptr asyncConnector = nullptr);

    TcpConnection::Ptr SyncConnectSession(std::string ip,
        int port,
        std::chrono::milliseconds timeout,
        tyrantnet::net::TcpService::Ptr service,
        const std::vector<TcpService::AddSocketOption::AddSocketOptionFunc>& options,
        tyrantnet::net::AsyncConnector::Ptr asyncConnector = nullptr);

} }
