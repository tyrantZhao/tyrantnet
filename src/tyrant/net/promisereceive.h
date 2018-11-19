#ifndef __NET_PROMISERECEIVE_H__
#define __NET_PROMISERECEIVE_H__

#include <tyrant/net/wraptcpservice.h>

namespace tyrant
{
    namespace net
    {
        /* binary search in memory */
        void memsearch(const char *hay, size_t haysize, const char *needle, size_t needlesize, size_t& result, bool& isOK)
        {
            size_t haypos, needlepos;
            haysize -= needlesize;

            for (haypos = 0; haypos <= haysize; haypos++)
            {
                for (needlepos = 0; needlepos < needlesize; needlepos++)
                {
                    if (hay[haypos + needlepos] != needle[needlepos])
                    {
                        // Next character in haystack.
                        break;
                    }
                }
                if (needlepos == needlesize)
                {
                    result = haypos;
                    isOK = true;
                    return;
                }
            }

            isOK = false;
        }

        class PromiseReceive;

        std::shared_ptr<PromiseReceive> setupPromiseReceive(const TCPSession::PTR& session);

        class PromiseReceive : public std::enable_shared_from_this<PromiseReceive>
        {
        public:
            typedef std::shared_ptr<PromiseReceive> PTR;
            typedef std::function<bool(const char* buffer, size_t len)> Handle;

            PromiseReceive::PTR receive(size_t len, Handle handle)
            {
                return receive(std::make_shared<size_t>(len), std::move(handle));
            }

            PromiseReceive::PTR receive(std::shared_ptr<size_t> len, Handle handle)
            {
                if (*len < 0)
                {
                    throw std::runtime_error("len less than zero");
                }

                return helpReceive(std::move(len), "", std::move(handle));
            }

            PromiseReceive::PTR receiveUntil(std::string str, Handle handle)
            {
                if (str.empty())
                {
                    throw std::runtime_error("str is empty");
                }

                return helpReceive(nullptr, std::move(str), std::move(handle));
            }

        private:
            PromiseReceive::PTR helpReceive(std::shared_ptr<size_t> len, std::string str, Handle handle)
            {
                auto pr = std::make_shared<PendingReceive>();
                pr->len = std::move(len);
                pr->str = std::move(str);
                pr->handle = std::move(handle);
                mPendingReceives.push_back(std::move(pr));

                return shared_from_this();
            }

            size_t process(const char* buffer, const size_t len)
            {
                size_t procLen = 0;

                while (!mPendingReceives.empty() && len >= procLen)
                {
                    auto pendingReceive = mPendingReceives.front();
                    if (pendingReceive->len != nullptr)
                    {
                        const auto tryReceiveLen = *pendingReceive->len;
                        if (tryReceiveLen < (len - procLen))
                        {
                            break;
                        }

                        mPendingReceives.pop_front();
                        procLen += tryReceiveLen;
                        if (pendingReceive->handle(buffer + procLen - tryReceiveLen, tryReceiveLen) && tryReceiveLen > 0)
                        {
                            mPendingReceives.push_front(pendingReceive);
                        }
                    }
                    else if (!pendingReceive->str.empty())
                    {
                        size_t pos = 0;
                        bool isOK = false;
                        memsearch(buffer + procLen,
                            len-procLen, 
                            pendingReceive->str.c_str(), 
                            pendingReceive->str.size(),
                            pos,
                            isOK);

                        if (!isOK)
                        {
                            break;
                        }

                        mPendingReceives.pop_front();
                        auto findLen = pos + pendingReceive->str.size();
                        procLen += findLen;
                        if (pendingReceive->handle(buffer + procLen - findLen, findLen))
                        {
                            mPendingReceives.push_front(pendingReceive);
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                return procLen;
            }

        private:
            struct PendingReceive
            {
                std::shared_ptr<size_t> len;
                std::string str;
                Handle handle;
            };

            std::deque<std::shared_ptr<PendingReceive>> mPendingReceives;

            friend std::shared_ptr<PromiseReceive> setupPromiseReceive(const TCPSession::PTR& session);
        };

        std::shared_ptr<PromiseReceive> setupPromiseReceive(const TCPSession::PTR& session)
        {
            auto promiseReceive = std::make_shared<PromiseReceive>();
            session->setDataCallback([promiseReceive](const TCPSession::PTR& session, 
                const char* buffer, 
                size_t len) {
                return promiseReceive->process(buffer, len);
            });

            return promiseReceive;
        }
    } // net
} // tyrant

#endif //promisereceive.h
