#ifndef __TYRANTNET_NET_ANY_H__
#define __TYRANTNET_NET_ANY_H__

#include <tyrant/common/cppversion.h>

#ifdef HAVE_LANG_CXX17
#include <any>
#else
#include <cstdint>
#endif

namespace tyrant { namespace net {
    #ifdef HAVE_LANG_CXX17
    typedef ::std::any TyrantAny;

    template<typename T>
    auto cast(const TyrantAny& ud)
    {
        return std::any_cast<T>(&ud);
    }
    #else
    typedef int64_t TyrantAny;
    template<typename T>
    const T* cast(const TyrantAny& ud)
    {
        return static_cast<const T*>(&ud);
    }
    #endif
}}

#endif //__TYRANTNET_NET_ANY_H__
