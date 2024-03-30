#!/usr/bin/bash

function check_hash(){
    f1=$1
    f2=$2

    for entry in {$f1,$f2}  
    do  
        if ! test -e "${entry}"; then
            echo "FAIL: not found : ${entry}"
            return
        fi
    done

    f1_hash=$(/usr/bin/openssl dgst -sha256 ${f1} | awk '{print $2}')
    f2_hash=$(/usr/bin/openssl dgst -sha256 ${f2} | awk '{print $2}')
    
    if [ "${f1_hash}" == "${f2_hash}" ]
    then
        echo "SUCCESS: HASH | URI"
        echo "         ${f1_hash} | ${f1}"
        echo "         ${f2_hash} | ${f2}"
    else
        echo "FAIL:    HASH | URI"
        echo "         ${f1_hash} | ${f1}"
        echo "         ${f2_hash} | ${f2}"
    fi
}
