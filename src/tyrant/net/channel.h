#ifndef __NET_CHANNEL_H__
#define __NET_CHANNEL_H__

namespace tyrant
{
    namespace net
    {
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
    } // net
} // tyrant

#endif //channel.h