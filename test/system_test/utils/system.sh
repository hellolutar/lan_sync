#!/usr/bin/bash

function checkSudo(){
    if [ $(id -u) != 0 ]; then
        echo "FAIL: please run with the sudo permission"
        exit -1
    else
        echo "SUCCESS: permission"
    fi
}