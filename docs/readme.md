


``` mermaid 
sequenceDiagram

CLI ->> SER : SYNC
SER -->> CLI : ACK, SYNC_READY状态
CLI收到ACK后，进入SYNC_READY状态

// 建立 HTTP连接
CLI ->> SER: GET RES_TABLE
SER -->> CLI: GET RES_TABLE

CLI ->> SER: GET RESOURCE
SER --> CLI: GET RESOURCE

// 断开 HTTP连接

```



### 握手阶段
1. 若选择接收方，则启动udp服务器，端口58080
2. 若选择发送方，则发送udp报文，并且要周期性发送
3. 接收方选择“发送方”，立即进入同步阶段


### 同步阶段
1. 发送方、接收方各自形成自己的“资源表”
2. 互相请求对方的“资源表”
3. 收到对方的资源表后，比较各自的“资源表”，谁的文件大就保留谁的


### p2p阶段
一个中央资源服务器，其余都是客户端，各终端之前的资源在中央服务器形成


### 断开连接阶段