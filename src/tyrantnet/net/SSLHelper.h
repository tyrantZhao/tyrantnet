#pragma once

#include <string>
#include <memory>

#include <tyrantnet/common/NonCopyable.h>
#include <tyrantnet/net/Noexcept.h>

#ifdef USE_OPENSSL

#ifdef  __cplusplus
extern "C" {
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>
#ifdef  __cplusplus
}
#endif

#endif

namespace tyrantnet { namespace net {

    class SSLHelper : public common::NonCopyable, public std::enable_shared_from_this<SSLHelper>
    {
    public:
        using Ptr = std::shared_ptr<SSLHelper>;

#ifdef USE_OPENSSL
        bool                                initSSL(const std::string& certificate,
                                                const std::string& privatekey);
        void                                destroySSL();
        SSL_CTX*                            getOpenSSLCTX();
#endif
        static  Ptr                         Create();

    private:
        SSLHelper() TYRANTNET_NOEXCEPT;
        virtual ~SSLHelper() TYRANTNET_NOEXCEPT;

    private:
#ifdef USE_OPENSSL
        SSL_CTX*                            mOpenSSLCTX;
#endif
    };

} }
