


``` mermaid 
sequenceDiagram

CLI ->> SER : HELLO
SER -->> CLI : ACK, SYNC_READY状态
CLI收到ACK后，进入SYNC_READY状态

// 建立 TCP连接
CLI ->> SER: GET RES_TABLE
SER -->> CLI: GET RES_TABLE

CLI ->> SER: GET RESOURCE
SER -->> CLI: GET RESOURCE

// 断开 TCP连接

```


CLI状态机
``` mermaid
graph TD

DISCOVERING --> DISCOVERING
DISCOVERING --HELLO ACK --> SYNC_READY
```




SER状态机
``` mermaid
graph TD

DISCOVERING --> DISCOVERING
DISCOVERING --HELLO--> SYNC_READY
```

- 握手阶段：起始状态都是 DISCOVERING
- 握手阶段：收到HELLO/HELLO ACK报文后进入SYNC_READY
- 同步阶段: 进入SYNC_READY状态后启动TCP服务器、客户端
- 同步阶段: 同步资源表
- 同步阶段: 同步资源
- 同步阶段: 断开TCP连接
- 同步阶段：FIN
- 断开阶段：状态变为DISCOVERING

### 握手阶段
1. 若选择接收方，则启动udp服务器，端口58080
2. 若选择发送方，则发送udp报文，并且要周期性发送 // TODO 引入libevent
3. 接收方选择“发送方”，立即进入同步阶段

discover_server
1. 监听UDP报文，如果是HELLO报文，则进入SYNC_READY。 

discover
1. 获取本机IP、获取广播地址，发送广播报文
2. 收到HELLO_ACK，进入SYNC_READY


### 同步阶段
1. 发送方、接收方各自形成自己的“资源表”
2. 互相请求对方的“资源表”
3. 收到对方的资源表后，比较各自的“资源表”，谁的文件大就保留谁的

discover_server

1. 启动TCP SERVER,  将本机IP、对端IP，记入SYN_CON_TABLE, 回复UDP报文时，告知对方我的TCP端口，让对方与我建立TCP连接。
2. 我开始统计我的资源列表，形成RESOURCE_TABLE
3. 客户端向我请求RESOURCE_TABLE，我响应RESOURCE_TABLE
4. 客户端向我提交更新的文件，我比较后决定拒绝，或更新RESOURCE_TABLE，完成后，将最新的RESOURCE_TABLE再次发送给客户端
5. 客户端向我请求资源文件REQ RESOURCE，我响应资源文件。

discover
1. 从响应中读取TCP端口号，建立TCP连接
2. 请求RESOURCE_TABLE
3. 比较RESOURCE_TABLE
4.1 发现本地有更新的文件，则提交更新请求
    接收更新请求后的响应，再次比较RESOURCE_TABLE, 若依旧由更新的文件，则再次提交请求
4.2 发现本地缺少文件，则发送REQ RESOURCE，接收并更新本地文件

### p2p阶段 TODO 
一个中央资源服务器，其余都是客户端，各终端之前的资源在中央服务器形成


### 断开连接阶段


