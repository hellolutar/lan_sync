
``` shell
git clone --depth=1 https://github.com/libevent/libevent.git
cmake -S libevent/ -B libevent/build
cmake --build libevent/build
sudo cmake --install libevent/build

git clone --depth=1 https://github.com/gabime/spdlog.git
cmake -S spdlog/ -B spdlog/build
cmake --build spdlog/build
sudo cmake --install spdlog/build

git clone --depth=1 https://github.com/openssl/openssl.git
cd openssl/ 
./Configure
make -j12
sudo make install

git clone --depth=1 https://github.com/google/googletest.git
cmake -S googletest/ -B googletest/build
cmake --build googletest/build
sudo cmake --install googletest/build

apt-get install bridge-utils

sudo ldconfig

```


``` mermaid
sequenceDiagram

CLI ->> SER : HELLO
SER -->> CLI : ACK, SYNC_READY状态

note left of CLI: CLI收到ACK后，进入SYNC_READY状态
note left of CLI: 建立 TCP连接

CLI ->> SER: GET RES_TABLE
SER -->> CLI: REPLY RES_TABLE

CLI ->> SER: GET RESOURCE
SER -->> CLI: REPLY RESOURCE
note left of CLI: 断开 TCP连接
```


CLI状态机, SER状态机
``` mermaid
graph TD

in --> DISCOVERING

subgraph UDP
DISCOVERING --5s--> DISCOVERING
end
subgraph TCP
DISCOVERING -- recv HELLO --> SYNC_READY
SYNC_READY --recv HELLO / GET / REPLY --> SYNC_READY
end

SYNC_READY --> out
```

UDP报文
- 握手阶段：起始状态都是 DISCOVERING
- 握手阶段：收到HELLO/HELLO ACK报文后进入SYNC_READY

<br>


<br/>
TCP报文
- 同步阶段: 进入SYNC_READY状态后启动TCP服务器、客户端
- 同步阶段: 同步资源表
- 同步阶段: 同步资源
- 同步阶段: 断开TCP连接
- 同步阶段：FIN
- 断开阶段：状态变为DISCOVERING
<br/>



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
   - 同步过程中用hash判断文件是否同步成功
   - 若不成功，则重新请求资源： todo
   - 引入 “正在同步的资源表”

<br/>

通信双方都是平等点，都运行着discover_server、discover。
程序启动后，discover的服务去寻找资源，并请求资源，如此各节点的资源都同步了。  

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
4 发现本地缺少文件，则发送REQ RESOURCE，接收并更新本地文件


#### 正在同步资源表
| peer    | uri   | status        | update_time         |
| ------- | ----- | ------------- | ------------------- |
| ip:port | /x/xx | pending       | 2024/01/01 16:33:11 |
| ip:port | /x/xx | synchronizing | 2024/01/01 16:33:11 |
| ip:port | /x/xx | failed        | 2024/01/01 16:33:11 |
 
 ``` mermaid
graph TD

in --> pending
pending --> syncing
syncing --> success
syncing --> fail
fail --> pending
success --> out
 ```

pending: 想要的同步的资源加入此表时，状态为pending




#### 模块间通信
mine.server 收到 peer.cli 的udp广播:
- 此时mine.server 要告知 mine.cli 去发现peer.server，然后请求peer.server的资源。

peer tcp 断开(此时mine与peer是邻居关系)
- mine.server 要告知mine.cli应移除针对peer的广播（计数器实现）。 TODO



### p2p阶段 TODO 
一个中央资源服务器，其余都是客户端，各终端之前的资源在中央服务器形成

| 名字 | 大小 | sha256 | uri | 存储地址 |
| ---- | ---- | ------ | --- | -------- |
| name | size | digest | uri | path     |


1. 形成resource table
   - 扫描指定路径，要求递归扫描
   - 计算sha256
2. 比较resource table
3. 请求resource
   请求uri
4. 回传resource
5. 保存resource
   - 边接收边保存


### 断开连接阶段




``` shell
/usr/bin/openssl dgst -sha256 hello.txt

```


20240314 需求
- 一个资源被多个资源服务器拥有，即一个uri存在于多个资源服务器
- 请求资源时，应从多个资源服务器请求资源，每个请求应该只请求部分资源，最后合并成一个资源。

实现
接收到table_Index后，形成sync_table. sync_table的entry为<uri, wanto_sync_dto>

wanto_sync_dto为
- uri
- hash
- size
- server
  - status
  - netaddr
  - cr

若资源大于20MB，则向多个服务器请求资源，否则之请求一个资源


// 要求tcpserver和tcpcli应该共用同一个协议处理逻辑
- tcpserver与tcpcli建立连接后，此连接应该可以执行协议中所有的动作。