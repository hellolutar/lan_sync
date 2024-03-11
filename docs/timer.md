
sync cli 维护一个设备发现表：
- 发送局域网广播；给特定ip发送udp报文

sync cli 维护tcp连接表
- 请求table index

以上内容都需要定时器去触发。

- timer 为定时器
- trigger 为触发器,需要将触发对象注册到timer
- trigger_decorator 包装以下两个子触发器
  - 负责与syncserver通信，然后负责与子触发器通信
  - discover_trigger 维护设备发现表
  - sync_trigger 维护tcp连接表
