#ifndef __TYRANTNET__NET_SSLHELPER_H__
#define __TYRANTNET__NET_SSLHELPER_H__

#include <string>
#include <memory>

#include <tyrant/common/noncopyable.h>
#include <tyrant/net/noexcept.h>

#ifdef USE_OPENSSL

#ifdef  __cplusplus
extern "C" {
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>
#ifdef  __cplusplus
}
#endif

#endif // USE_OPENSSL

namespace tyrant { namespace net {
    class SSLHelper : public common::NonCopyable, public std::enable_shared_from_this<SSLHelper>
    {
    public:
        typedef std::shared_ptr<SSLHelper>   PTR;

#ifdef USE_OPENSSL
        bool                                initSSL(const std::string& certificate,
                                                    const std::string& privatekey);
        void                                destroySSL();
        SSL_CTX*                            getOpenSSLCTX();
#endif //USE_OPENSSL
        static  PTR                         Create();

    private:
        SSLHelper() TYRANT_NOEXCEPT;
        virtual ~SSLHelper() TYRANT_NOEXCEPT;

    private:
#ifdef USE_OPENSSL
        SSL_CTX*                            mOpenSSLCTX;
#endif // USE_OPENSSL
    };
}}

#endif //__TYRANTNET__NET_SSLHELPER_H__
