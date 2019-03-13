#pragma once

#include <functional>
#include <queue>
#include <memory>
#include <vector>
#include <chrono>

#include <tyrantnet/net/Noexcept.h>
namespace tyrantnet { namespace timer {

    using namespace std::chrono;
    class TimerMgr;

    class Timer final
    {
    public:
        using Ptr = std::shared_ptr<Timer>;
        using WeakPtr = std::weak_ptr<Timer>;
        using Callback = std::function<void(void)>;

        Timer(steady_clock::time_point startTime, nanoseconds lastTime, Callback f) TYRANTNET_NOEXCEPT;

        const steady_clock::time_point&         getStartTime() const;
        const nanoseconds&                      getLastTime() const;

        nanoseconds                             getLeftTime() const;
        void                                    cancel();

    private:
        void operator()                         ();

    private:
        bool                                    mActive;
        Callback                                mCallback;
        const steady_clock::time_point          mStartTime;
        nanoseconds                             mLastTime;

        friend class TimerMgr;
    };

    class TimerMgr final
    {
    public:
        using Ptr = std::shared_ptr<TimerMgr>;

        template<typename F, typename ...TArgs>
        Timer::WeakPtr                          addTimer(nanoseconds timeout, F callback, TArgs&& ...args)
        {
            auto timer = std::make_shared<Timer>(
                steady_clock::now(), 
                nanoseconds(timeout),
                std::bind(std::move(callback), std::forward<TArgs>(args)...));
            mTimers.push(timer);

            return timer;
        }

        void                                    addTimer(nanoseconds timeout, Timer::Ptr timer)
        {
            mTimers.push(timer);
        }

        void                                    schedule();
        bool                                    isEmpty() const;
        // if timer empty, return zero
        nanoseconds                             nearLeftTime() const;
        void                                    clear();

    private:
        class CompareTimer
        {
        public:
            bool operator() (const Timer::Ptr& left, const Timer::Ptr& right) const
            {
                const auto startDiff = left->getStartTime() - right->getStartTime();
                const auto lastDiff = left->getLastTime() - right->getLastTime();
                const auto diff = startDiff.count() + lastDiff.count();
                return diff > 0;
            }
        };

        std::priority_queue<Timer::Ptr, std::vector<Timer::Ptr>, CompareTimer>  mTimers;
    };

} }
