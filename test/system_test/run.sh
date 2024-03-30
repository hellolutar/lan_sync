#!/usr/bin/bash


WORKSPACE=$(pwd)
source ${WORKSPACE}/utils/io.sh
source ${WORKSPACE}/utils/system.sh
source ${WORKSPACE}/utils/hash.sh

HOME_CLI=${WORKSPACE}/veth0
RESOUCE_HOME_CLI=${WORKSPACE}/veth0/rs
HOME_SRV=${WORKSPACE}/veth1
RESOUCE_HOME_SRV=${WORKSPACE}/veth1/rs
SIZE_1MB=1024000
SIZE_10MB=10240000
SIZE_50MB=51200000
SIZE_100MB=102400000

function setup_net_env() {
    echo -e "\t[DEBUG] setup net env"

    sudo ip netns add ns0
    if [ "$?" -ne 0 ]
    then
        echo "FAIL: sudo ip netns add ns0"
    else
        echo "SUCCESS: add netns ns0"
        
        sudo ip netns add ns1
        sudo ip link add vbridge type bridge 
        sudo ip link set dev vbridge up

        sudo ip link add veth-0 type veth peer name veth-0-br
        sudo ip link add veth-1 type veth peer name veth-1-br

        sudo ip link set veth-0 netns ns0
        sudo ip link set veth-0-br master vbridge
        sudo ip link set veth-1 netns ns1
        sudo ip link set veth-1-br master vbridge

        # sudo bridge link

        sudo ip netns exec ns0 ip addr add 192.168.233.101/24 dev veth-0
        sudo ip netns exec ns1 ip addr add 192.168.233.102/24 dev veth-1

        sudo ip netns exec ns0 ip link set veth-0 up
        sudo ip link set dev veth-0-br up
        sudo ip netns exec ns1 ip link set veth-1 up
        sudo ip link set dev veth-1-br up

        sudo ip addr add 192.168.233.1/24 dev vbridge

        sudo ip netns exec ns0 ip route add default via 192.168.233.1
        sudo ip netns exec ns1 ip route add default via 192.168.233.1

        # sudo sudo ip netns exec ns0 ip route
    fi

    ping -c 1 192.168.233.101 >/dev/null 2>&1
    if [ "$?" -ne 0 ]
    then
        echo "FAIL: netowork environment"
        exit -1
    else
        echo "SUCCESS: netowork environment"
    fi
}

function build_app(){
    echo -e "\t[DEBUG] build app"

    cd ${WORKSPACE}/../../src
    # make clean >/dev/null 2>&1
    make >/dev/null 2>&1

    if [ "$?" -ne 0 ]
    then
        echo "FAIL: env_app"
    else
        echo "SUCCESS: env_app"
    fi
    cp main.bin ${WORKSPACE}/

    cd ${WORKSPACE}
}

function setup_runtime_workspace(){
    echo -e "\t[DEBUG] setup runtime workspace"

    if [ -d ${HOME_CLI} ] ;then
        rm -rf ${HOME_CLI}   
    fi

    if [ -d ${HOME_SRV} ] ;then
        rm -rf ${HOME_SRV}   
    fi

    mkdir -p ${RESOUCE_HOME_SRV}
    mkdir -p ${RESOUCE_HOME_CLI}

    if [ ! -e ${WORKSPACE}/main.bin ]  
        then
        echo "FAIL: main.bin is not found!"
        exit -1
    fi

    cp ${WORKSPACE}/main.bin ${HOME_CLI}
    cp ${WORKSPACE}/main.bin ${HOME_SRV}

    cp ${WORKSPACE}/veth0.properties ${HOME_CLI}/properties.properties
    cp ${WORKSPACE}/veth1.properties ${HOME_SRV}/properties.properties

    echo "resource.home=${RESOUCE_HOME_CLI}" >> ${HOME_CLI}/properties.properties
    echo "resource.home=${RESOUCE_HOME_SRV}" >> ${HOME_SRV}/properties.properties
}


# echo -n -e "\x01\xff\x00\x08\x00\x00\x00\x08" | nc 127.0.0.1 58080
# prepare_env

# genFile "abc.txt" 100

# echo ${WORKSPACE}


function clean_local(){
    echo -e "\t[DEBUG] clean runtime workspace"
    sudo rm -rf ${HOME_CLI}
    sudo rm -rf ${HOME_SRV}
}

function clean_net() {
    echo -e "\t[DEBUG] clean net env"

    sudo ip netns del ns0 >/dev/null 2>&1
    sudo ip netns del ns1 >/dev/null 2>&1
    sudo ip link set dev vbridge down >/dev/null 2>&1
    sudo brctl delbr vbridge >/dev/null 2>&1
    sudo ip link delete veth-0-br >/dev/null 2>&1
    sudo ip link delete veth-1-br >/dev/null 2>&1
}

function clean() {
    clean_local
    clean_net
}

setup(){
    checkSudo
    clean
    build_app
    setup_net_env
    setup_runtime_workspace
}

function prepareFile() {
    mockFile ${RESOUCE_HOME_SRV}/srv_big.txt ${SIZE_100MB}
    mockFile ${RESOUCE_HOME_SRV}/srv_small.txt ${SIZE_10MB}

    mockFile ${RESOUCE_HOME_CLI}/cli_big.txt ${SIZE_100MB}
    mockFile ${RESOUCE_HOME_CLI}/cli_small.txt ${SIZE_10MB}
}

function valid_sync_result() {
    check_hash ${RESOUCE_HOME_CLI}/srv_big.txt      ${RESOUCE_HOME_SRV}/srv_big.txt 
    check_hash ${RESOUCE_HOME_CLI}/srv_small.txt    ${RESOUCE_HOME_SRV}/srv_small.txt 
    check_hash ${RESOUCE_HOME_CLI}/cli_big.txt      ${RESOUCE_HOME_SRV}/cli_big.txt 
    check_hash ${RESOUCE_HOME_CLI}/cli_small.txt    ${RESOUCE_HOME_SRV}/cli_small.txt 

}

function runApp(){
    where_before=$1
    where_want=$2
    netns_name=$3
    echo -e "\t[DEBUG] run [${netns_name}] [${where_want}]"

    cd ${where_want}
    ip netns exec ${netns_name} ./main.bin > stdout.log 2>&1 &
    cd ${where_before}
}

run(){
    echo -e "\t[DEBUG] run application"
    prepareFile
    # 后台运行程序
    runApp ${WORKSPACE} ${HOME_CLI} ns0
    runApp ${WORKSPACE} ${HOME_SRV} ns1
    sleep 60s

    echo -e "\t[DEBUG] app exit"
    echo -n -e "\x01\xff\x00\x08\x00\x00\x00\x08" | nc 192.168.233.101 58081
    echo -n -e "\x01\xff\x00\x08\x00\x00\x00\x08" | nc 192.168.233.102 58081

    echo -e "\t[DEBUG] valid sync result"
    valid_sync_result
    echo -e "\t[DEBUG] done!"
}


main() {
   setup
   run
}

main
