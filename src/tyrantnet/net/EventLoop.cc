﻿#include <cassert>
#include <iostream>
#include <algorithm>

#include <tyrantnet/net/Channel.h>
#include <tyrantnet/net/EventLoop.h>

namespace tyrantnet { namespace net {

#ifdef PLATFORM_WINDOWS
    class WakeupChannel final : public Channel, public common::NonCopyable
    {
    public:
        explicit WakeupChannel(HANDLE iocp) : mIOCP(iocp), mWakeupOvl(port::Win::OverlappedType::OverlappedRecv)
        {
        }

        bool    wakeup() 
        {
            return PostQueuedCompletionStatus(mIOCP, 0, reinterpret_cast<ULONG_PTR>(this), &mWakeupOvl.base);
        }

    private:
        void    canRecv()  override
        {
        }

        void    canSend()  override
        {
        }

        void    onClose()  override
        {
        }

    private:
        HANDLE                      mIOCP;
        port::Win::OverlappedExt    mWakeupOvl;
    };
#else
    class WakeupChannel final : public Channel, public common::NonCopyable
    {
    public:
        explicit WakeupChannel(sock fd) : mFd(fd)
        {
        }

        bool    wakeup()
        {
            uint64_t one = 1;
            return write(mFd, &one, sizeof one) > 0;
        }

        ~WakeupChannel() override
        {
            close(mFd);
            mFd = INVALID_SOCKET;
        }

    private:
        void    canRecv() override
        {
            char temp[1024 * 10];
            while (true)
            {
                auto n = read(mFd, temp, sizeof(temp));
                if (n == -1 || static_cast<size_t>(n) < sizeof(temp))
                {
                    break;
                }
            }
        }

        void    canSend() override
        {
        }

        void    onClose() override
        {
        }

    private:
        sock    mFd;
    };
#endif
    EventLoop::EventLoop()
#ifdef PLATFORM_WINDOWS
        
        : 
    mIOCP(CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 1)), 
    mWakeupChannel(std::make_unique<WakeupChannel>(mIOCP))
#else
    
        :
    mEpollFd(epoll_create(1))
#endif
    {
#ifdef PLATFORM_WINDOWS
        mPGetQueuedCompletionStatusEx = NULL;
        auto kernel32_module = GetModuleHandleA("kernel32.dll");
        if (kernel32_module != NULL) {
            mPGetQueuedCompletionStatusEx = reinterpret_cast<sGetQueuedCompletionStatusEx>(GetProcAddress(
                kernel32_module,
                "GetQueuedCompletionStatusEx"));
            FreeLibrary(kernel32_module);
        }
#else
        auto eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        mWakeupChannel.reset(new WakeupChannel(eventfd));
        linkChannel(eventfd, mWakeupChannel.get());
#endif

        mIsAlreadyPostWakeup = false;
        mIsInBlock = true;

        reallocEventSize(1024);
        mSelfThreadID = -1;
        mTimer = std::make_shared<timer::TimerMgr>();
    }


    EventLoop::~EventLoop() 
    {
#ifdef PLATFORM_WINDOWS
        CloseHandle(mIOCP);
        mIOCP = INVALID_HANDLE_VALUE;
#else
        close(mEpollFd);
        mEpollFd = -1;
#endif
    }

    Timer::WeakPtr EventLoop::runAfter(nanoseconds timeout, std::function<void(void)> callback)
    {
        auto timer = std::make_shared<tyrantnet::timer::Timer>(
            steady_clock::now(),
            nanoseconds(timeout),
            callback);

        if (isInLoopThread())
        {
            mTimer->addTimer(timeout, timer);
        }
        else
        {
            auto timerMgr = mTimer;
            runAsyncFunctor([timerMgr, timeout, timer]() {
                timerMgr->addTimer(timeout, timer);
            });
        }

        return timer;
    }

    inline void EventLoop::tryInitThreadID()
    {
        std::call_once(mOnceInitThreadID, [this]() {
            mSelfThreadID = current_thread::tid();
        });
    }

    void EventLoop::loop(int64_t milliseconds)
    {
        tryInitThreadID();

#ifndef NDEBUG
        assert(isInLoopThread());
#endif
        if (!isInLoopThread())
        {
            throw std::runtime_error("only loop in io thread");
        }

        if (!mAfterLoopFunctors.empty())
        {
            milliseconds = 0;
        }

#ifdef PLATFORM_WINDOWS
        ULONG numComplete = 0;
        if (mPGetQueuedCompletionStatusEx != nullptr)
        {
            if (!mPGetQueuedCompletionStatusEx(mIOCP,
                mEventEntries.data(),
                mEventEntries.size(),
                &numComplete,
                static_cast<DWORD>(milliseconds),
                false))
            {
                numComplete = 0;
            }
        }
        else
        {
            for (auto& e : mEventEntries)
            {
                const auto timeout = (numComplete == 0) ? static_cast<DWORD>(milliseconds) : 0;
                /* don't check the return value of GQCS */
                GetQueuedCompletionStatus(mIOCP,
                    &e.dwNumberOfBytesTransferred,
                    &e.lpCompletionKey,
                    &e.lpOverlapped,
                    timeout);
                if (e.lpOverlapped == nullptr)
                {
                    break;
                }
                ++numComplete;
            }
        }

        mIsInBlock = false;

        for (ULONG i = 0; i < numComplete; ++i)
        {
            auto channel = (Channel*)mEventEntries[i].lpCompletionKey;
            assert(channel != nullptr);
            const auto ovl = reinterpret_cast<const port::Win::OverlappedExt*>(mEventEntries[i].lpOverlapped);
            if (ovl->OP == port::Win::OverlappedType::OverlappedRecv)
            {
                channel->canRecv();
            }
            else if (ovl->OP == port::Win::OverlappedType::OverlappedSend)
            {
                channel->canSend();
            }
            else
            {
                assert(false);
            }
        }
#else
        int numComplete = epoll_wait(mEpollFd, mEventEntries.data(), mEventEntries.size(), milliseconds);

        mIsInBlock = false;

        for (int i = 0; i < numComplete; ++i)
        {
            auto    channel = (Channel*)(mEventEntries[i].data.ptr);
            auto    event_data = mEventEntries[i].events;

            if (event_data & EPOLLRDHUP)
            {
                channel->canRecv();
                channel->onClose();
                continue;
            }

            if (event_data & EPOLLIN)
            {
                channel->canRecv();
            }

            if (event_data & EPOLLOUT)
            {
                channel->canSend();
            }
        }
#endif

        mIsAlreadyPostWakeup = false;
        mIsInBlock = true;

        processAsyncFunctors();
        processAfterLoopFunctors();

        if (static_cast<size_t>(numComplete) == mEventEntries.size())
        {
            reallocEventSize(mEventEntries.size() + 128);
        }

        mTimer->schedule();
    }

    void EventLoop::loopCompareNearTimer(int64_t milliseconds)
    {
        tryInitThreadID();

#ifndef NDEBUG
        assert(isInLoopThread());
#endif
        if (!isInLoopThread())
        {
            throw std::runtime_error("only loop in io thread");
        }

        if (!mTimer->isEmpty())
        {
            auto nearTimeout = std::chrono::duration_cast<std::chrono::milliseconds>(mTimer->nearLeftTime());
            milliseconds = std::min<int64_t>(milliseconds, nearTimeout.count());
        }

        loop(milliseconds);
    }

    void EventLoop::processAfterLoopFunctors()
    {
        mCopyAfterLoopFunctors.swap(mAfterLoopFunctors);
        for (const auto& x : mCopyAfterLoopFunctors)
        {
            x();
        }
        mCopyAfterLoopFunctors.clear();
    }

    void EventLoop::processAsyncFunctors()
    {
        {
            std::lock_guard<std::mutex> lck(mAsyncFunctorsMutex);
            mCopyAsyncFunctors.swap(mAsyncFunctors);
        }

        for (const auto& x : mCopyAsyncFunctors)
        {
            x();
        }
        mCopyAsyncFunctors.clear();
    }

    bool EventLoop::wakeup()
    {
        if (!isInLoopThread() && mIsInBlock && !mIsAlreadyPostWakeup.exchange(true))
        {
            return mWakeupChannel->wakeup();
        }

        return false;
    }

    bool EventLoop::linkChannel(sock fd, const Channel* ptr) 
    {
#ifdef PLATFORM_WINDOWS
        return CreateIoCompletionPort((HANDLE)fd, mIOCP, (ULONG_PTR)ptr, 0) != nullptr;
#else
        struct epoll_event ev = { 0, { nullptr } };
        ev.events = EPOLLET | EPOLLIN | EPOLLRDHUP;
        ev.data.ptr = (void*)ptr;
        return epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &ev) == 0;
#endif
    }

    TcpConnectionPtr EventLoop::getTcpConnection(sock fd)
    {
        auto it = mTcpConnections.find(fd);
        if (it != mTcpConnections.end())
        {
            return (*it).second;
        }
        return nullptr;
    }

    void EventLoop::addTcpConnection(sock fd, TcpConnectionPtr tcpConnection)
    {
        mTcpConnections[fd] = std::move(tcpConnection);
    }

    void EventLoop::removeTcpConnection(sock fd)
    {
        mTcpConnections.erase(fd);
    }

    void EventLoop::runAsyncFunctor(UserFunctor f)
    {
        if (isInLoopThread())
        {
            f();
        }
        else
        {
            {
                std::lock_guard<std::mutex> lck(mAsyncFunctorsMutex);
                mAsyncFunctors.emplace_back(std::move(f));
            }
            wakeup();
        }
    }

    void EventLoop::runFunctorAfterLoop(UserFunctor f)
    {
        assert(isInLoopThread());
        if (!isInLoopThread())
        {
            throw std::runtime_error("only push after functor in io thread");
        }

        mAfterLoopFunctors.emplace_back(std::move(f));
    }

#ifndef PLATFORM_WINDOWS
    int EventLoop::getEpollHandle() const
    {
        return mEpollFd;
    }
#endif

    void EventLoop::reallocEventSize(size_t size)
    {
        mEventEntries.resize(size);
    }

} }
