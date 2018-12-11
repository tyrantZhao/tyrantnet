#ifndef __TYRANTNET_NET_CHANNEL_H__
#define __TYRANTNET_NET_CHANNEL_H__

namespace tyrant { namespace net {
    class EventLoop;

    class Channel
    {
    public:
        virtual ~Channel() = default;

    private:
        virtual void    canSend() = 0;
        virtual void    canRecv() = 0;
        virtual void    onClose() = 0;

        friend class EventLoop;
    };
}}

#endif //__TYRANTNET_NET_CHANNEL_H__