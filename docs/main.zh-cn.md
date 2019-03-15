# 概述
* `tyrantnet` 是一个多线程的异步网络库，能够运行在Linux和Windows环境下, 仅仅需要C++ 11编译器(部分用了较新的特性做替代)，且没有其他任何第三方依赖。

# 架构
* `tyrantnet` 核心分为事件层和网络连接层。
## 事件层
* 由`EventLoop`提供,用于检测socket的可读可写,并包含一个事件通知,可用于线程间通信(消息队列)。
  详情见[EventLoop](https://github.com/tyrantZhao/tyrantnet/blob/master/docs/EventLoop.zh-cn.md)。
## 网络连接层
* 由`ListenThread`和`AsyncConnector`提供,前者用于接收外部的新链接请求,并绑定一个EventLoop处理器，后者用于向外部进行异步地进行网络连接。
  详情见[ListenThread](https://github.com/tyrantZhao/tyrantnet/blob/master/docs/ListenThread.zh-cn.md)和[AsyncConnector](https://github.com/tyrantZhao/tyrantnet/blob/master/docs/Connector.zh-cn.md)。

## 安全的Socket对象
* `tyrantnet`不对用户暴露原始的socket fd，而是提供`TcpSocket::Ptr`，详见[Socket](https://github.com/tyrantZhao/tyrantnet/blob/master/docs/Socket.zh-cn.md)

## 高级特性
* 提供 `PromiseReceive` 方便解析网络消息，详见:[PromiseReceive](https://github.com/tyrantZhao/tyrantnet/blob/master/docs/PromiseReceive.zh-cn.md)

# 编译选项
## YRANTNET_BUILD_TYPE_DEBUG
* 是否开启调试模式，默认关闭。
## TYRANTNET_BUILD_EXAMPLES
* 是否编译范例， 默认开启。
## TYRANTNET_BUILD_TESTINGS
* 是否编译测试用例，默认关闭。
## TYRANTNET_USE_OPENSSL
* 是否使用openssl, 默认开启。（会先检测openssl， 若为未找到， 会输出一条警告信息）。

# 完整示例
请查看[example](https://github.com/tyrantZhao/tyrantnet/tree/master/example)
