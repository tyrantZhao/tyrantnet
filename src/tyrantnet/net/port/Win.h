#pragma once

#include <tyrantnet/net/SocketLibFunction.h>

namespace tyrantnet { namespace net { namespace port {

#ifdef PLATFORM_WINDOWS
    class Win
    {
    public:
        enum class OverlappedType
        {
            OverlappedNone = 0,
            OverlappedRecv,
            OverlappedSend,
        };

        struct OverlappedExt
        {
            OVERLAPPED  base;
            const OverlappedType  OP;

            OverlappedExt(OverlappedType op) TYRANTNET_NOEXCEPT : OP(op)
            {
                memset(&base, 0, sizeof(base));
            }
        };
    };
#endif

} } }