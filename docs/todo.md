- 利用TCP完成数据同步
  - 大数据量，分批写文件
  - cli 请求 资源
  - cli 更新 资源 ： 新增、替换
- HTTP监控服务


discover 内部设置 libevent。
libevent事件处理函数传入处理特定任务的对象。


周期性任务：
1. hello ack

udp 读任务：
  - 接收到响应，即可以拿到对方addr。 然后依据此addr，建立tcp连接，即bufferevent。
  - 支持连接多台服务器，需要维护tcp表。



系统测试
- 利用ovs qos 做测试。





















