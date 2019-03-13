#pragma once

#include <tyrantnet/common/CPP_VERSION.h>

#ifdef HAVE_LANG_CXX17
#include <any>
#else
#include <cstdint>
#endif

namespace tyrantnet { namespace net {

#ifdef HAVE_LANG_CXX17
    using TyrantnetAny = std::any;

    template<typename T>
    auto cast(const TyrantnetAny& ud)
    {
        return std::any_cast<T>(&ud);
    }
#else
    using TyrantnetAny = int64_t;
    template<typename T>
    const T* cast(const TyrantnetAny& ud)
    {
        return static_cast<const T*>(&ud);
    }
#endif

} }
