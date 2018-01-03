#ifndef __COMMON_NONCOPYABLE_H_INCLUDED__
#define __COMMON_NONCOPYABLE_H_INCLUDED__

namespace tyrant
{
    class NonCopyable
    {
    protected:
        NonCopyable()   {}
        virtual ~NonCopyable()  {}

    private:
        NonCopyable(const NonCopyable&) = delete;
        const NonCopyable& operator=(const NonCopyable&) = delete;
    };
}

#endif // noncopyable.h__
