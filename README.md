# Simple Tbus
模仿tbus写的简易跨进程/网络间通信中间件

# complie

create_channel_tool

`cmake --build "/path/to/build/dir" --target create_channels`


tbusd

`cmake --build "/path/to/build/dir" --target tbusd_run`


receive_api

`cmake --build "/path/to/build/dir" --target api_channel_receive`


receive_api

`cmake --build "/path/to/build/dir" --target api_channel_send`

# run
建立 0.0.0.1 -> 0.0.0.2 容量为409600的通道

`.\create_channels simple_tbus_test 40960 simple_channel_test 409600 0.0.0.1 0.0.0.2`

打开tbusd

`.\tbusd_run`

打开接收进程0.0.0.2

`./api_channel_receive 0.0.0.2`

打开发送进程0.0.0.1 向0.0.0.2 发送 “hello!world”

`./api_channel_send 0.0.0.1 0.0.0.2 hello!world'`

## **note**

由于`api_channel_receive`1秒收一次消息，`api_channel_send`3秒发一次消息，所以会出现很多`check faild: fail to resv`请不要意外



## 依赖
`boost 1.6`以上版本

## 各个文件内容
`SimpleTbusAPI` tbus 的API封装

`SimpleTbusd`  tbusd的实现，

`SimpleTbusdConn` tbusd的实现，几乎所有通信逻辑都在这个文件，包括tbusd与本地进程、tbusd与tbusd的通信逻辑

`SimpleTbusCtl` 创建全局通道表的实现

`SimpleChannel` 基于共享内存的单消费者单生产者无锁循环的实现


# todo
脚本功能完善

第一次写 asio 代码写得太丑（尤其是`asyn_write`, `asyn_read`不允许连续调用，需要不停在回调函数中嵌套）