#include <tyrantnet/timer/Timer.h>

namespace tyrantnet{ namespace timer {

    const steady_clock::time_point& Timer::getStartTime() const
    {
        return mStartTime;
    }

    const nanoseconds& Timer::getLastTime() const
    {
        return mLastTime;
    }

    nanoseconds Timer::getLeftTime() const
    {
        return getLastTime() - (steady_clock::now() - getStartTime());
    }

    void Timer::cancel()
    {
        mActive = false;
    }

    Timer::Timer(steady_clock::time_point startTime, 
        nanoseconds lastTime, 
        Callback callback) TYRANTNET_NOEXCEPT
        :
        mActive(true),
        mCallback(std::move(callback)),
        mStartTime(startTime),
        mLastTime(lastTime)
    {
    }

    void Timer::operator() ()
    {
        if (mActive)
        {
            mCallback();
        }
    }

    void TimerMgr::schedule()
    {
        while (!mTimers.empty())
        {
            auto tmp = mTimers.top();
            if (tmp->getLeftTime() > nanoseconds::zero())
            {
                break;
            }

            mTimers.pop();
            (*tmp)();
        }
    }

    bool TimerMgr::isEmpty() const
    {
        return mTimers.empty();
    }

    nanoseconds TimerMgr::nearLeftTime() const
    {
        if (mTimers.empty())
        {
            return nanoseconds::zero();
        }

        auto result = mTimers.top()->getLeftTime();
        if (result < nanoseconds::zero())
        {
            return nanoseconds::zero();
        }

        return result;
    }

    void TimerMgr::clear()
    {
        while (!mTimers.empty())
        {
            mTimers.pop();
        }
    }

} }
