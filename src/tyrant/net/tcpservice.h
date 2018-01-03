#ifndef __NET_TCPSERVICE_H__
#define __NET_TCPSERVICE_H__

#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <cstdint>
#include <memory>

#include <tyrant/common/noncopyable.h>
#include <tyrant/common/typeids.h>
#include <tyrant/net/noexcept.h>
#include <tyrant/net/datasocket.h>
#include <tyrant/net/sslhelper.h>

namespace tyrant
{
    namespace net
    {
        class EventLoop;
        class ListenThread;
        class IOLoopData;

        class TcpService : public NonCopyable, public std::enable_shared_from_this<TcpService>
        {
        public:
            typedef ::std::shared_ptr<TcpService>                                           PTR;
            typedef int64_t                                                                 SESSION_TYPE;

            typedef ::std::function<void(const EventLoop::PTR&)>                            FRAME_CALLBACK;
            typedef ::std::function<void(SESSION_TYPE, const std::string&)>                 ENTER_CALLBACK;
            typedef ::std::function<void(SESSION_TYPE)>                                     DISCONNECT_CALLBACK;
            typedef ::std::function<size_t(SESSION_TYPE, const char* buffer, size_t len)>   DATA_CALLBACK;

        public:
            static  PTR                         Create();

        public:
            void                                send(SESSION_TYPE id, 
                                                    const DataSocket::PACKET_PTR& packet, 
                                                    const DataSocket::PACKED_SENDED_CALLBACK& callback = nullptr) const;

            void                                postShutdown(SESSION_TYPE id) const;
            /* still will cause dis connect callback */
            void                                postDisConnect(SESSION_TYPE id) const;

            void                                setHeartBeat(SESSION_TYPE id, 
                                                                 std::chrono::nanoseconds checktime);

            /* pass nullptr sslHelper if fd is client socket, either is a server side socket */
            bool                                addDataSocket(sock fd,
                                                                const SSLHelper::PTR& sslHelper,
                                                                bool isUseSSL,
                                                                const TcpService::ENTER_CALLBACK& enterCallback,
                                                                const TcpService::DISCONNECT_CALLBACK& disConnectCallback,
                                                                const TcpService::DATA_CALLBACK& dataCallback,
                                                                size_t maxRecvBufferSize,
                                                                bool forceSameThreadLoop = false);

            void                                startWorkerThread(size_t threadNum, FRAME_CALLBACK callback = nullptr);
            void                                stopWorkerThread();

            void                                wakeup(SESSION_TYPE id) const;
            void                                wakeupAll() const;
            EventLoop::PTR                      getRandomEventLoop();
            EventLoop::PTR                      getEventLoopBySocketID(SESSION_TYPE id) const TYRANT_NOEXCEPT;
            std::shared_ptr<IOLoopData>         getIOLoopDataBySocketID(SESSION_TYPE id) const TYRANT_NOEXCEPT;

        private:
            TcpService() TYRANT_NOEXCEPT;
            virtual ~TcpService() TYRANT_NOEXCEPT;

            bool                                helpAddChannel(DataSocket::PTR channel,
                                                                const std::string& ip,
                                                                const TcpService::ENTER_CALLBACK& enterCallback,
                                                                const TcpService::DISCONNECT_CALLBACK& disConnectCallback,
                                                                const TcpService::DATA_CALLBACK& dataCallback,
                                                                bool forceSameThreadLoop = false);
        private:
            SESSION_TYPE                        MakeID(size_t loopIndex, const std::shared_ptr<IOLoopData>&);
            void                                procDataSocketClose(DataSocket::PTR);
            void                                postSessionAsyncProc(SESSION_TYPE id, 
                std::function<void(DataSocket::PTR)> callback) const;

        private:
            std::vector<std::shared_ptr<IOLoopData>>    mIOLoopDatas;
            mutable std::mutex                  mIOLoopGuard;
            bool                                mRunIOLoop;

            std::mutex                          mServiceGuard;
        };

        void IOLoopDataSend(const std::shared_ptr<IOLoopData>&, 
            TcpService::SESSION_TYPE id, 
            const DataSocket::PACKET_PTR& packet, 
            const DataSocket::PACKED_SENDED_CALLBACK& callback);

        const EventLoop::PTR& IOLoopDataGetEventLoop(const std::shared_ptr<IOLoopData>&);
    } // net
} // tyrant

#endif // tcpservice.h
