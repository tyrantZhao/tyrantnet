#include <iostream>
#include <string>
#include <mutex>
#include <condition_variable>

#include <tyrant/net/socketlibfunction.h>
#include <tyrant/net/http/HttpService.h>
#include <tyrant/net/http/HttpFormat.h>

using namespace tyrant::net;

int main(int argc, char **argv)
{

    auto service = std::make_shared<WrapTcpService>();
    service->startWorkThread(2);

    sock fd = base::Connect(false, "191.236.16.125", 80);
    if (fd != SOCKET_ERROR)
    {
        auto socket = TcpSocket::Create(fd, false);
        auto enterCallback = [](const TCPSession::PTR& session) {
            HttpService::setup(session, [](const HttpSession::PTR& httpSession) {
                HttpRequest request;
                request.setMethod(HttpRequest::HTTP_METHOD::HTTP_METHOD_GET);
                request.setUrl("/httpgallery/chunked/chunkedimage.aspx");
                request.addHeadValue("Host", "www.httpwatch.com");

                std::string requestStr = request.getResult();
                httpSession->send(requestStr.c_str(), requestStr.size());
                httpSession->setHttpCallback([](const HTTPParser& httpParser, const HttpSession::PTR& session) {
                    //http response handle
                    std::cout << httpParser.getBody() << std::endl;
                });
            });
        };

        service->addSession(std::move(socket),
            AddSessionOption::WithEnterCallback(enterCallback),
            AddSessionOption::WithMaxRecvBufferSize(10));
    }

    std::cin.get();
    return 0;
}
