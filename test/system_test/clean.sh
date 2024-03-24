#!/usr/bin/bash
function clean_local(){
    sudo rm -rf veth0
    sudo rm -rf veth1
    sudo rm -rf /home/lutar/code/c/lan_sync/src/static/cli/*
}

function clean_net() {
    sudo ip netns del ns0
    sudo ip netns del ns1
    sudo ip link set dev vbridge down
    sudo brctl delbr vbridge
    sudo ip link delete veth-0-br
    sudo ip link delete veth-1-br
}
function clean() {
    clean_local
    clean_net
}

clean