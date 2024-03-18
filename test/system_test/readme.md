
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

``` shell
# Run command inside network namespace
as_ns () {
    NAME=$1
    NETNS=faucet-${NAME}
    shift
    sudo ip netns exec ${NETNS} $@
}

# Create network namespace
create_ns () {
    NAME=$1
    IP=$2
    NETNS=faucet-${NAME}
    sudo ip netns add ${NETNS}
    sudo ip link add dev veth-${NAME} type veth peer name veth0 netns ${NETNS}
    sudo ip link set dev veth-${NAME} up
    as_ns ${NAME} ip link set dev lo up
    [ -n "${IP}" ] && as_ns ${NAME} ip addr add dev veth0 ${IP}
    as_ns ${NAME} ip link set dev veth0 up
}

create_ns host1 192.168.0.1/24
create_ns host2 192.168.0.2/24

sudo /usr/local/share/openvswitch/scripts/ovs-ctl start --system-id=random

as_ns host1 ping 192.168.0.2


```