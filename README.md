tyrantnet
=======
A cross platform network library(support Windows and Linux), using c++11(use some C++17 feature), no third ilb is depended.

## Features
* support linux and Windows
* multi-thread safety and high performance
* none extention depend
* ipv6 support
* OpenSSL support(use macro USE-OPENSSL when compiling)
* support http, websocket, https

##Documentation
- [简体中文](https://github.com/tyrantZhao/tyrantnet/blob/master/docs/main.zh-cn.md)

## Examples
* [pingpongclient](https://github.com/tyrantZhao/tyrantnet/blob/master/example/pingpongclient.cc)
* [pingpongserver](https://github.com/tyrantZhao/tyrantnet/blob/master/example/pingpongserver.cc)
* [broadcastclient](https://github.com/tyrantZhao/tyrantnet/blob/master/example/broadcastclient.cc)
* [broadcastserver](https://github.com/tyrantZhao/tyrantnet/blob/master/example/broadcastserver.cc)
* See more examples in [examples](https://github.com/tyrantZhao/tyrantnet/tree/master/example)

## Compatibility
* Visual C++ 2013+ on Windows (32/64-bit)
* GCC 4.8+ on Linux (32/64-bit)
* Not Support Mac OS X

## About session safety
  This library uses three ways to identify one session(also is three way to use this library).
  * First, use raw pointer named [DataSocket](https://github.com/tyrantZhao/tyrantnet/blob/master/src/tyrant/net/datasocket.h#L36), combined with [EventLoop](https://github.com/tyrantZhao/tyrantnet/blob/master/src/tyrant/net/eventloop.h#L25).
  * Second, use int64_t number identify one session, use [TCPService](https://github.com/tyrantZhao/tyrantnet/blob/master/src/tyrant/net/tcpservice.h#L25) which wraps DataSocket of first layer.
  * Thrid, use smart pointer named [TCPSession::PTR](https://github.com/tyrantZhao/tyrantnet/blob/master/src/tyrant/net/wraptcpservice.h#L24), combined with [WrapTcpService](https://github.com/tyrantZhao/tyrantnet/blob/master/src/tyrant/net/wraptcpservice.h#L96), you can control session by [TCPSession::PTR](https://github.com/tyrantZhao/tyrantnet/blob/master/src/tyrant/net/wraptcpservice.h#L24)

The third way is strongly recommanded.
