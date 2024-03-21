#!/usr/bin/bash
function cleanup() {
    # sudo ip netns del ns0
    # sudo ip netns del ns1
    # sudo ip link set dev vbridge down
    # sudo brctl delbr vbridge
    #  rm -rf cli
    #  rm -rf srv
    echo ""
}

function net_env_normal() {
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

        sudo bridge link

        sudo ip netns exec ns0 ip addr add 192.168.233.101/24 dev veth-0
        sudo ip netns exec ns1 ip addr add 192.168.233.102/24 dev veth-1

        sudo ip netns exec ns0 ip link set veth-0 up
        sudo ip link set dev veth-0-br up
        sudo ip netns exec ns1 ip link set veth-1 up
        sudo ip link set dev veth-1-br up

        sudo ip addr add 192.168.233.1/24 dev vbridge

        sudo ip netns exec ns0 ip route add default via 192.168.233.1
        sudo ip netns exec ns1 ip route add default via 192.168.233.1

        sudo sudo ip netns exec ns0 ip route
    fi

    ping -c 1 192.168.233.101 >/dev/null 2>&1
    if [ "$?" -ne 0 ]
    then
        echo "FAIL: netowork environment"
    else
        echo "SUCCESS: netowork environment"
    fi
}

function env_app(){
    cd ../../src
    # make clean >/dev/null 2>&1
    make >/dev/null 2>&1

    if [ "$?" -ne 0 ]
    then
        echo "FAIL: env_app"
    else
        echo "SUCCESS: env_app"
    fi
    cd - >/dev/null 2>&1
}

function env_dir(){
    rm -rf cli/*
    rm -rf srv/*
    # mkdir cli
    # mkdir srv
    env_app
    cp ../../src/main.bin cli/
    cp ../../src/main.bin srv/

    cp cli.properties cli/properties.properties
    cp srv.properties srv/properties.properties
}

function run_app() {
    echo "run application"
    # sudo ip netns exec net0 ./main.bin -c ./cli.properties
    # sudo ip netns exec net1 ./main.bin -c ./srv.properties
}

function prepare_env(){
    net_env_normal
    env_dir
    run_app

    cleanup
}

prepare_env