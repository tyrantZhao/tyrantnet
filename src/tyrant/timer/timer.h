#ifndef __TIMER_TIMER_H_INCLUDED__
#define __TIMER_TIMER_H_INCLUDED__

#include <chrono>
#include <functional>
#include <queue>
#include <memory>
#include <vector>

namespace tyrant
{
    class TimerMgr;

    //定时器
    class Timer
    {
    public:
        typedef ::std::shared_ptr<Timer>          Ptr;
        typedef ::std::weak_ptr<Timer>            WeakPtr;
        typedef ::std::function<void(void)>       Callback;

        Timer(::std::chrono::steady_clock::time_point startTime,
            ::std::chrono::nanoseconds lastTime,
            Callback f);
        ~Timer() = default;

        //获取定时器开始时间
        const ::std::chrono::steady_clock::time_point&    getStartTime() const;
        //获取定时器剩余时间
        const ::std::chrono::nanoseconds&                 getLastTime() const;
        //取消定时器
        void                                            cancel();

    private:
        void operator()                         ();

    private:
        bool                                    IsActive_;      //开启状态
        Callback                                Callback_;      //回调
        const ::std::chrono::steady_clock::time_point StartTime_; //开始时间
        ::std::chrono::nanoseconds                LastTime_;      //剩余时间

        friend class TimerMgr;
    };

    //定时器管理器
    class TimerMgr
    {
    public:
        typedef ::std::shared_ptr<TimerMgr>   PTR;

        template<typename F, typename ...TArgs>
        Timer::WeakPtr                          addTimer(::std::chrono::nanoseconds timeout,
                                                         F callback,
                                                         TArgs&& ...args)
        {
            auto timer = ::std::make_shared<Timer>(::std::chrono::steady_clock::now(),
                                                ::std::chrono::nanoseconds(timeout),
                                                ::std::bind(::std::move(callback), ::std::forward<TArgs>(args)...));
            Timers_.push(timer);

            return timer;
        }

        //调度
        void                                    schedule();
        //判断管理器有无定时器
        bool                                    isEmpty() const;
        //最近定时器剩余时间，如果无定时器,返回0
        ::std::chrono::nanoseconds              nearLeftTime() const;
        //晴空
        void                                    clear();

    private:
        //定时器优先级排序
        class CompareTimer
        {
        public:
            bool operator() (const Timer::Ptr& left, const Timer::Ptr& right) const
            {
                auto startDiff = (left->getStartTime() - right->getStartTime());
                auto lastdiff = left->getLastTime() - right->getLastTime();
                auto diff = startDiff.count() + lastdiff.count();
                return diff > 0;
            }
        };

        //定时器管理器
        ::std::priority_queue<Timer::Ptr, ::std::vector<Timer::Ptr>, CompareTimer>  Timers_;
    };
}

#endif //__TIMER_TIMER_H_INCLUDED__
