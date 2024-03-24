
install openvswitch

``` shell
mkdir ~/libs
wget https://www.openvswitch.org/releases/openvswitch-2.17.9.tar.gz -O ~/libs/openvswitch-2.17.9.tar.gz
cd ~/libs

tar -xf openvswitch-2.17.9.tar.gz
cd openvswitch-2.17.9
./configure
make
sudo make install
```



veth pair

https://blog.csdn.net/cuiwenjun/article/details/120402132
``` shell
sudo ip -all netns exec ip a

sudo ip netns add ns0
sudo ip netns add ns1
sudo ip link add name h0 type veth peer name h1
sudo ip link set h0 netns ns0
sudo ip link set h1 netns ns1
sudo ip netns exec ns0 ip link set h0 up
sudo ip netns exec ns0 ip link set lo up
sudo ip netns exec ns0 ip addr add 192.168.100.100/24 dev h0
sudo ip netns exec ns0 ip route
sudo ip netns exec ns1 ip link set h1 up
sudo ip netns exec ns1 ip link set lo up
sudo ip netns exec ns1 ip addr add 192.168.100.200/24 dev h1
sudo ip netns exec ns0 ping -c 3 192.168.100.200
```

br

https://zhuanlan.zhihu.com/p/658040916

``` shell
ip netns add ns1
ip netns add ns2
ip netns add ns3

ip link add vbridge type bridge 
ip link set dev vbridge up

ip link add veth-1 type veth peer name veth-1-br
ip link add veth-2 type veth peer name veth-2-br
ip link add veth-3 type veth peer name veth-3-br

ip link set veth-1 netns ns1
ip link set veth-1-br master vbridge
ip link set veth-2 netns ns2
ip link set veth-2-br master vbridge
ip link set veth-3 netns ns3
ip link set veth-3-br master vbridge

bridge link

ip netns exec ns1 ip addr add 192.168.233.101/24 dev veth-1
ip netns exec ns2 ip addr add 192.168.233.102/24 dev veth-2
ip netns exec ns3 ip addr add 192.168.233.103/24 dev veth-3

ip netns exec ns1 ip link set veth-1 up
ip link set dev veth-1-br up
ip netns exec ns2 ip link set veth-2 up
ip link set dev veth-2-br up
ip netns exec ns3 ip link set veth-3 up
ip link set dev veth-3-br up

ip netns exec ns1 ping 192.168.233.102

ip addr add 192.168.233.105/24 dev vbridge

ping 192.168.233.101

ip netns exec ns1 route -n

ip netns exec ns1 ip route add default via 192.168.233.105
ip netns exec ns2 ip route add default via 192.168.233.105
ip netns exec ns3 ip route add default via 192.168.233.105

ip netns exec ns1 ping 127.27.211.138


# 此时数据包是可以通过网卡发送出去的，但是由于数据包的源地址是一个内网地址，因此无法回数据包。
ip netns exec ns1 ping 8.8.8.8

# 这里首先需要开机主机的数据包转发功能。可以使用sysctl修改ip_forward的内核参数使得主机可以支持数据包转发。
sysctl net.ipv4.ip_forward
net.ipv4.ip_forward = 0

sysctl -w net.ipv4.ip_forward=1
net.ipv4.ip_forward = 1

# 通过iptables增加SNAT规则，将源ip为192.168.233.0/24内的数据包的源ip修改为ens33的ip
iptables -t nat -A POSTROUTING -s 192.168.233.0/24 -j MASQUERADE

ip netns exec ns1 ping 8.8.8.8
```


集成测试环境：

常规测试：
- 1. UDPSERVER/TCPSERVER 提供一个退出程序的接口
- 2. 脚本利用ip netns 搭建网络环境。 搭建resourcehome环境（1kb、300MB、多级目录）。
- 3. 脚本启动程序，并在60s后调用退出程序接口。
- 4. 脚本验证文件数量、一致性。
