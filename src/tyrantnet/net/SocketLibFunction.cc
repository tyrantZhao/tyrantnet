#include <stdio.h>
#include <string.h>

#include <tyrantnet/net/SocketLibFunction.h>

namespace tyrantnet { namespace net { namespace base {

    bool InitSocket(void)
    {
        bool ret = true;
#if defined PLATFORM_WINDOWS
        static WSADATA g_WSAData;
        static bool WinSockIsInit = false;
        if (WinSockIsInit)
        {
            return true;
        }
        if (WSAStartup(MAKEWORD(2, 2), &g_WSAData) == 0)
        {
            WinSockIsInit = true;
        }
        else
        {
            ret = false;
        }
#else
        signal(SIGPIPE, SIG_IGN);
#endif

        return ret;
    }

    void DestroySocket(void)
    {
#if defined PLATFORM_WINDOWS
        WSACleanup();
#endif
    }

    int SocketNodelay(sock fd)
    {
        const int flag = 1;
        return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, static_cast<const void*>(&flag), static_cast<socklen_t>(sizeof(flag)));
    }

    bool SocketBlock(sock fd)
    {
        int err;
        unsigned long ul = false;
#if defined PLATFORM_WINDOWS
        err = ioctlsocket(fd, FIONBIO, &ul);
#else
        err = ioctl(fd, FIONBIO, &ul);
#endif

        return err != SOCKET_ERROR;
    }

    bool SocketNonblock(sock fd)
    {
        int err;
        unsigned long ul = true;
#if defined PLATFORM_WINDOWS
        err = ioctlsocket(fd, FIONBIO, &ul);
#else
        err = ioctl(fd, FIONBIO, &ul);
#endif

        return err != SOCKET_ERROR;
    }

    int SocketSetSendSize(sock fd, int sd_size)
    {
        return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, static_cast<const void*>(&sd_size), static_cast<socklen_t>(sizeof(sd_size)));
    }

    int SocketSetRecvSize(sock fd, int rd_size)
    {
        return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, static_cast<const void*>(&rd_size), static_cast<socklen_t>(sizeof(rd_size)));
    }

    // TODO::Connect�Ƿ�ֱ�ӷ���TcpSocket::Ptr
    sock Connect(bool isIPV6, const std::string& server_ip, int port)
    {
        InitSocket();

        struct sockaddr_in ip4Addr = sockaddr_in();
        struct sockaddr_in6 ip6Addr = sockaddr_in6();
        struct sockaddr_in* paddr = &ip4Addr;
        int addrLen = sizeof(ip4Addr);

        sock clientfd = isIPV6 ?
            SocketCreate(AF_INET6, SOCK_STREAM, 0) :
            SocketCreate(AF_INET, SOCK_STREAM, 0);

        if (clientfd == INVALID_SOCKET)
        {
            return clientfd;
        }

        bool ptonResult = false;
        if (isIPV6)
        {
            ip6Addr.sin6_family = AF_INET6;
            ip6Addr.sin6_port = htons(port);
            ptonResult = inet_pton(AF_INET6, server_ip.c_str(), &ip6Addr.sin6_addr) > 0;
            paddr = (struct sockaddr_in*)&ip6Addr;
            addrLen = sizeof(ip6Addr);
        }
        else
        {
            ip4Addr.sin_family = AF_INET;
            ip4Addr.sin_port = htons(port);
            ptonResult = inet_pton(AF_INET, server_ip.c_str(), &ip4Addr.sin_addr) > 0;
        }

        if (!ptonResult)
        {
            SocketClose(clientfd);
            return INVALID_SOCKET;
        }

        while (connect(clientfd, (struct sockaddr*)paddr, addrLen) < 0)
        {
            if (EINTR == sErrno)
            {
                continue;
            }

            SocketClose(clientfd);
            return INVALID_SOCKET;
        }

        return clientfd;
    }

    sock Listen(bool isIPV6, const char* ip, int port, int back_num)
    {
        InitSocket();

        struct sockaddr_in ip4Addr = sockaddr_in();
        struct sockaddr_in6 ip6Addr = sockaddr_in6();
        struct sockaddr_in* paddr = &ip4Addr;
        int addrLen = sizeof(ip4Addr);

        sock socketfd = isIPV6 ?
            socket(AF_INET6, SOCK_STREAM, 0) :
            socket(AF_INET, SOCK_STREAM, 0);
        if (socketfd == INVALID_SOCKET)
        {
            return INVALID_SOCKET;
        }

        bool ptonResult = false;
        if (isIPV6)
        {
            ip6Addr.sin6_family = AF_INET6;
            ip6Addr.sin6_port = htons(port);
            ptonResult = inet_pton(AF_INET6, ip, &ip6Addr.sin6_addr) > 0;
            paddr = (struct sockaddr_in*)&ip6Addr;
            addrLen = sizeof(ip6Addr);
        }
        else
        {
            ip4Addr.sin_family = AF_INET;
            ip4Addr.sin_port = htons(port);
            ip4Addr.sin_addr.s_addr = INADDR_ANY;
            ptonResult = inet_pton(AF_INET, ip, &ip4Addr.sin_addr) > 0;
        }

        const int reuseaddr_value = 1;
        if (!ptonResult ||
            setsockopt(socketfd,
                SOL_SOCKET,
                SO_REUSEADDR,
                (const char *)&reuseaddr_value,
                sizeof(int)) < 0)
        {
            SocketClose(socketfd);
            return INVALID_SOCKET;
        }

        const int bindRet = bind(socketfd, (struct sockaddr*)paddr, addrLen);
        if (bindRet == SOCKET_ERROR ||
            listen(socketfd, back_num) == SOCKET_ERROR)
        {
            SocketClose(socketfd);
            return INVALID_SOCKET;
        }

        return socketfd;
    }

    void SocketClose(sock fd)
    {
#if defined PLATFORM_WINDOWS
        closesocket(fd);
#else
        close(fd);
#endif
    }

    static std::string getIPString(const struct sockaddr *sa)
    {
        char tmp[INET6_ADDRSTRLEN] = { 0 };
        switch (sa->sa_family)
        {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                tmp, sizeof(tmp));
            break;
        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                tmp, sizeof(tmp));
            break;
        default:
            return "Unknown AF";
        }

        return tmp;
    }

    std::string GetIPOfSocket(sock fd)
    {
#if defined PLATFORM_WINDOWS
        struct sockaddr name = sockaddr();
        int namelen = sizeof(name);
        if (getpeername(fd, static_cast<struct sockaddr*>(&name), &namelen) == 0)
        {
            return getIPString(&name);
        }
#else
        struct sockaddr_in name;
        socklen_t namelen = sizeof(name);
        if (getpeername(fd, (struct sockaddr*)(&name), static_cast<socklen_t*>(&namelen)) == 0)
        {
            return getIPString((const struct sockaddr*)&name);
        }
#endif

        return "";
    }

    int SocketSend(sock fd, const char* buffer, int len)
    {
        int transnum = send(fd, buffer, len, 0);
        if (transnum < 0 && S_EWOULDBLOCK == sErrno)
        {
            transnum = 0;
        }

        /*  send error if transnum < 0  */
        return transnum;
    }

    sock Accept(sock listenSocket, struct sockaddr* addr, socklen_t* addrLen)
    {
        return accept(listenSocket, addr, addrLen);
    }

    sock SocketCreate(int af, int type, int protocol)
    {
        return socket(af, type, protocol);
    }

    static struct sockaddr_in6 getLocalAddr(sock sockfd)
    {
        struct sockaddr_in6 localaddr = sockaddr_in6();
        socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
        if (::getsockname(sockfd, (struct sockaddr*)(&localaddr), &addrlen) < 0)
        {
            return localaddr;
        }
        return localaddr;
    }

    static struct sockaddr_in6 getPeerAddr(sock sockfd)
    {
        struct sockaddr_in6 peeraddr = sockaddr_in6();
        socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
        if (::getpeername(sockfd, (struct sockaddr*)(&peeraddr), &addrlen) < 0)
        {
            return peeraddr;
        }
        return peeraddr;
    }

    /* copy from muduo */
    bool IsSelfConnect(sock fd)
    {
        struct sockaddr_in6 localaddr = getLocalAddr(fd);
        struct sockaddr_in6 peeraddr = getPeerAddr(fd);

        if (localaddr.sin6_family == AF_INET)
        {
            const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
            const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
            return laddr4->sin_port == raddr4->sin_port
                && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
        }
        else if (localaddr.sin6_family == AF_INET6)
        {
            return localaddr.sin6_port == peeraddr.sin6_port
                && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
        }
        else
        {
            return false;
        }
    }

} } }
