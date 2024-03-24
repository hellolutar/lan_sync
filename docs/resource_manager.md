``` 
sync_rs{
    size,
    hash,
    uri,
    rs_block[],
    rs_owner_ip[],         // 拥有此资源的ip。要求size、hash相同。
                           // 若size不同，以大的为准进行更新。
}
syncing_range{
    *rs,
    ip,
    syncing_range,
    syncing_timeout,
}

rs_sync_mamanger{
    <uri, rs>
    <uri, syncing_range>  // 拥有此uri的ip

    table all_local_rs(); // 本地所有资源
    bool  save_rs_to_local(offset,data,len); // 保存本地资源
    bool  reg_sync_rs(ip, uri, size, hash);   // 添加rs。
    reqvo upd_sync_rs(ip, uri, size, hash);   // 更新uri信息。
    reqvo reg_req_sync_rs_auto(ip, uri); //根据ip，返回一个resource请求。 
    reqvo reg_req_sync_rs_cplt(ip,uri);
    reqvo reg_req_sync_rs_rang(ip,uri,offset,size);
}
```

- reg_net_rs: 将此ip的资源注册到resource中


场景1： 常规
- rs对应有两个文件块。
- ip1请求[0,500), ip2请求[500,1000)
- ip1成功。 删除<ip,syncing_range>,
- ip1成功。 删除<ip,syncing_range>,
- 此时已经同步资源：len(rs.rs_block[]) == 0  && len(<ip,syncing_range>) == 0,
- 同步资源完成后，需要验证hash。
- hash验证通过后，删除<uri, rs>。

场景2：正在同步，却发现更优的资源。
- rs对应有两个文件块。
- 已知ip1有此资源，ip1请求[0,500)。
- ip1的rs正在同步中，此时收到rs2的tb idx, 发现rs2由于rs1。
    - 情况1： ip1还没有回复，此时清空 uri对应的<ip, syncing_range>，删除已经本地已经同步的内容，更新<uri, rs>，
    - 情况2： ip1已经回复，正在写。 等待写完成，然后，清空 uri对应的<ip, syncing_range>，删除本地已经不同的内容，更新<uri, rs>。
- 当接收到ip1的rs回复时，先查询<ip, syncing_range>，若不存在，则表示当前回复已不合法，则不进行处理。

场景3：正在同步，其中一个peer连接异常。
- rs对应有两个文件块
- rs.size 大。 所以ip1有2个请求，ip2有一个请求。
- ip1请求[0,500), ip2请求[500,1000), ip1请求[1000,1500)。
- ip2异常（断开连接、超时），[500,1000)请求失败。 此时[500,1000)应该回到rs.rs_block[]中，重新分配。

场景4：正在同步，所有peer连接异常
- rs对应有两个文件块
- rs.size 大。 所以ip1有2个请求，ip2有一个请求。
- ip1和ip2同时断开连接，此时删除<ip, syncing_range>，<uri, rs>
