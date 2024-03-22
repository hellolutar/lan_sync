#!/usr/bin/bash
function clean_local(){
    sudo rm -rf veth0
    sudo rm -rf veth1
}

function clean_net() {
    sudo ip netns del ns0
    sudo ip netns del ns1
    sudo ip link set dev vbridge down
    sudo brctl delbr vbridge
}
function clean() {
    clean_local
    clean_net
}

clean_local