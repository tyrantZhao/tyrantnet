#include <tyrantnet/timer/Timer.h>

namespace tyrant
{
    const ::std::chrono::steady_clock::time_point& Timer::getStartTime() const
    {
        return StartTime_;
    }

    const ::std::chrono::nanoseconds& Timer::getLastTime() const
    {
        return LastTime_;
    }

    void Timer::cancel()
    {
        IsActive_ = false;
    }

    Timer::Timer(::std::chrono::steady_clock::time_point startTime,
        ::std::chrono::nanoseconds lastTime,
        Callback callback):
        StartTime_(::std::move(startTime)),
        LastTime_(::std::move(lastTime)),
        Callback_(::std::move(callback)),
        IsActive_(true)
    {
    }

    void Timer::operator() ()
    {
        if (IsActive_)
            Callback_();
    }

    void TimerMgr::schedule()
    {
        while (!Timers_.empty()) {
            auto tmp = Timers_.top();
            if ((::std::chrono::steady_clock::now() - tmp->getStartTime()) < tmp->getLastTime())
                break;
            Timers_.pop();
            tmp->operator() ();
        }
    }

    bool TimerMgr::isEmpty() const
    {
        return Timers_.empty();
    }

    ::std::chrono::nanoseconds TimerMgr::nearLeftTime() const
    {
        if (Timers_.empty())
            return ::std::chrono::nanoseconds::zero();

        auto result = Timers_.top()->getLastTime() -
        (::std::chrono::steady_clock::now() - Timers_.top()->getStartTime());

        if (result.count() < 0)
            return ::std::chrono::nanoseconds::zero();

        return result;
    }

    void TimerMgr::clear()
    {
        while (!Timers_.empty())
        Timers_.pop();
    }
} // tyrant
