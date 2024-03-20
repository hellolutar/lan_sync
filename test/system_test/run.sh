#!/usr/bin/bash
function cleanup() {
    #  sudo ip netns del ns0
    #  sudo ip netns del ns1
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
        sudo ip link add name h0 type veth peer name h1
        sudo ip link set h0 netns ns0
        sudo ip link set h1 netns ns1
        sudo ip netns exec ns0 ip link set h0 up
        sudo ip netns exec ns0 ip link set lo up
        sudo ip netns exec ns0 ip addr add 192.168.233.100/24 dev h0
        sudo ip netns exec ns1 ip link set h1 up
        sudo ip netns exec ns1 ip link set lo up
        sudo ip netns exec ns1 ip addr add 192.168.233.101/24 dev h1
        # sudo ip netns exec ns0 ip route
    fi

    sudo ip netns exec ns0 ping -c 1 192.168.233.101 >/dev/null 2>&1
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

    cp cli.properties cli
    cp srv.properties srv
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