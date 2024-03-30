#!/usr/bin/bash

function io_write_append() {
    fname=$1
    data=$2
    
    # -n 不换行； -e 支持\n等符号
    echo -n -e ${data} >> $fname
}

function io_write_create() {
    fname=$1
    data=$2

    if [ -e ${fname} ]; then
        rm -rf ${fname}
    fi

    touch ${fname}

    if [ ! -n $data ] 
    then
        return
    fi

    io_write_append ${fname} ${data}
}


function mockFile() {
    fname=$1
    fsize=$2

    echo -e "\t[DEBUG] mockFile: ${fname} ${fsize}"

    line_data="abcdefghijklmnopqrstuvwxyz1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
    data_1KB=${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}${line_data}
    data_64KB=${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}${data_1KB}
    data_2048KB=${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}${data_64KB}

    io_write_create $fname

    while true; do
        size_now=$(ls -la ${fname} | awk '{print $5}')
        # echo -e "\t[DEBUG] mockFile: ${size_now} / ${fsize}"
        if [ ${size_now} -gt ${fsize} ]
        then
            break
        fi
        io_write_append $fname ${data_2048KB}
    done
}