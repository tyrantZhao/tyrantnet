#ifndef __TYRANTNET_NET_SOCKET_H__
#define __TYRANTNET_NET_SOCKET_H__

#include <tyrant/common/noncopyable.h>
#include <tyrant/net/socketlibfunction.h>

#include <memory>
#include <string>
#include <exception>
#include <stdexcept>

namespace tyrant { namespace net {
    class DataSocket;

    class TcpSocket : public NonCopyable
    {
    private:
        class TcpSocketDeleter
        {
        public:
            void operator()(TcpSocket* ptr) const
            {
                delete ptr;
            }
        };
    public:
        typedef std::unique_ptr<TcpSocket, TcpSocketDeleter> PTR;

    public:
        static PTR Create(sock fd, bool serverSide);

    public:
        void    SocketNodelay() const;
        bool    SocketNonblock() const;
        void    SetSendSize(int sdSize) const;
        void    SetRecvSize(int rdSize) const;
        std::string GetIP() const;
        bool    isServerSide() const;

    protected:
        TcpSocket(sock fd, bool serverSide);
        virtual ~TcpSocket();

        sock    getFD() const;

    private:
        const sock    mFD;
        const bool    mServerSide;

        friend class DataSocket;
    };

    class EintrError : public std::exception
    {
    };

    class AcceptError : public std::runtime_error
    {
    public:
        AcceptError(int errorCode)
            :std::runtime_error(std::to_string(errorCode)),
            mErrorCode(errorCode)
        {}

        int getErrorCode() const
        {
            return mErrorCode;
        }
    private:
        int mErrorCode;
    };

    class ListenSocket : public NonCopyable
    {
    private:
        class ListenSocketDeleter
        {
        public:
            void operator()(ListenSocket* ptr) const
            {
                delete ptr;
            }
        };
    public:
        typedef std::unique_ptr<ListenSocket, ListenSocketDeleter> PTR;

    public:
        TcpSocket::PTR Accept();

    public:
        static PTR Create(sock fd);

    protected:
        explicit ListenSocket(sock fd);
        virtual ~ListenSocket();

    private:
        const sock  mFD;

        friend class DataSocket;
    };
}}

#endif //__TYRANTNET_NET_SOCKET_H__
