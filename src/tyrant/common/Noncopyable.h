#ifndef __TYRANTNET_COMMON_NONCOPYABLE_H__
#define __TYRANTNET_COMMON_NONCOPYABLE_H__

namespace tyrantnet { namespace common {
    class NonCopyable
    {
    protected:
        NonCopyable()   {}
        virtual ~NonCopyable()  {}

    private:
        NonCopyable(const NonCopyable&) = delete;
        const NonCopyable& operator=(const NonCopyable&) = delete;
    };
}}

#endif //__TYRANTNET_COMMON_NONCOPYABLE_H__
