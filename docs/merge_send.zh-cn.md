`tyrantnet`不会对每一个消息都进行`send`系统调用.

当上层需要发送消息时，是投递一个`std::function`投递到`EventLoop`中。

在`std::function`执行时会将消息放入`DataSocket`的待发送队列中，见[DataSocket::sendPacket](https://github.com/tyrantZhao/tyrantnet/blob/master/src/tyrant/net/datasocket.cc#L118)。

在此异步回调函数中又会继续投递(有且仅有)一个延迟函数[DataSocket::runAfterFlush](https://github.com/tyrantZhao/tyrantnet/blob/master/src/tyrant/net/datasocket.cc#L192)，延迟函数均在`EventLoop::loop`调用结束之前被调用。

这个延迟函数就负责将待发送队列里的数据尝试一次性批量发送(Linux下使用writev，Windows下会进行合并)，见[DataSocket::quickFlush](https://github.com/tyrantZhao/tyrantnet/blob/master/src/tyrant/net/datasocket.cc#L446)和[DataSocket::normalFlush](https://github.com/tyrantZhao/tyrantnet/blob/master/src/tyrant/net/datasocket.cc#L333)。</br>
