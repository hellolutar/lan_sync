---@diagnostic disable: redundant-parameter
-- do not modify this table
local debug_level = {
    DISABLED = 0,
    LEVEL_1  = 1,
    LEVEL_2  = 2
}

local default_settings =
{
    debug_level  = debug_level.LEVEL_1,
    enabled      = true,         -- whether this dissector is enabled or not
    port         = 58081,        -- default TCP port number for LSP
    max_msg_len  = 409600,       -- max length of LSP message
    subdissect   = true,         -- whether to call sub-dissector or not
    subdiss_type = wtap.NETLINK, -- the encap we get the subdissector for
}


local dprint = function() end
local dprint2 = function() end
local function resetDebugLevel()
    if default_settings.debug_level > debug_level.DISABLED then
        dprint = function(...)
            print(table.concat({ "Lua: ", ... }, " "))
        end

        if default_settings.debug_level > debug_level.LEVEL_1 then
            dprint2 = dprint
        end
    else
        dprint = function() end
        dprint2 = dprint
    end
end
-- call it now
resetDebugLevel()


-- this is the size of the LSP message header (8 bytes) and the minimum LSP
-- message size we need to figure out how much the rest of the Netlink message
-- will be
local LSP_MSG_HDR_LEN = 8


-- 协议名称
local lansync_proto = Proto("lan_sync_protocol", "Lan Sync Proto")
-- 协议字段
local versions = {
    [1] = "v0.1"
}
local types = {
    [1] = "Hi",
    [2] = "ReHi",
    [3] = "GetIndex",
    [4] = "ReIndex",
    [5] = "GetRs",
    [6] = "ReRs",
    [7] = "UpdRs",
    [8] = "Close",
}

local hdr_fields =
{
    version = ProtoField.uint8("lan_sync.version", "version", base.DEC, versions),
    type = ProtoField.uint8("lan_sync.type", "type", base.DEC, types),
    header_len = ProtoField.uint16("lan_sync.header_len", "header len", base.DEC),
    total_len = ProtoField.uint32("lan_sync.total_len", "total len", base.DEC),
}

-- register the ProtoFields
lansync_proto.fields = hdr_fields
dprint2("lan sync_proto ProtoFields registered")



--这段代码是目的Packet符合条件时，被Wireshark自动调用的，是p_AB的成员方法
function lansync_proto.dissector(tvbuf, pktinfo, root)
    dprint2("lan_sync_proto.dissector called")

    -- get the length of the packet buffer (Tvb).
    local pktlen = tvbuf:len()

    local bytes_consumed = 0


    -- we do this in a while loop, because there could be multiple LSP messages
    -- inside a single TCP segment, and thus in the same tvbuf - but our
    -- lansync_proto.dissector() will only be called once per TCP segment, so we
    -- need to do this loop to dissect each LSP message in it
    while bytes_consumed < pktlen do
        -- We're going to call our "dissect()" function, which is defined
        -- later in this script file. The dissect() function returns the
        -- length of the LSP message it dissected as a positive number, or if
        -- it's a negative number then it's the number of additional bytes it
        -- needs if the Tvb doesn't have them all. If it returns a 0, it's a
        -- dissection error.
        local result = dissectLsp(tvbuf, pktinfo, root, bytes_consumed)


        if result > 0 then
            -- we successfully processed an LSP message, of 'result' length
            bytes_consumed = bytes_consumed + result
            -- go again on another while loop
        elseif result == 0 then
            -- If the result is 0, then it means we hit an error of some kind,
            -- so return 0. Returning 0 tells Wireshark this packet is not for
            -- us, and it will try heuristic dissectors or the plain "data"
            -- one, which is what should happen in this case.
            return 0
            -- bytes_consumed = bytes_consumed + result
            -- return bytes_consumed
        else
            -- we need more bytes, so set the desegment_offset to what we
            -- already consumed, and the desegment_len to how many more
            -- are needed
            pktinfo.desegment_offset = bytes_consumed

            -- invert the negative result so it's a positive number
            result = -result

            pktinfo.desegment_len = result

            -- even though we need more bytes, this packet is for us, so we
            -- tell wireshark all of its bytes are for us by returning the
            -- number of Tvb bytes we "successfully processed", namely the
            -- length of the Tvb
            return pktlen
        end
    end

    -- In a TCP dissector, you can either return nothing, or return the number of
    -- bytes of the tvbuf that belong to this protocol, which is what we do here.
    -- Do NOT return the number 0, or else Wireshark will interpret that to mean
    -- this packet did not belong to your protocol, and will try to dissect it
    -- with other protocol dissectors (such as heuristic ones)
    return bytes_consumed
end

----------------------------------------
-- 以下是一个本地函数，用于使用 desegment_offset/desegment_len 方法剖析 TCP 段内的 LSP 消息。
-- 这是一个单独的函数，因为我们通过 TCP 运行，因此可能需要解析单个segment/packet中的多个消息。
-- 因此，我们调用此函数仅解析一个 LSP 消息，并在 Proto 的主解析器函数的 while 循环中调用它。
-- 此函数从 Proto 的解析器函数传入原始 Tvb、Pinfo 和 TreeItem，以及此函数应开始解析的 Tvb 中的偏移量。
--
-- 该函数返回
-- 正数 表示解析的 LSP 消息的长度
-- 负数 表示它需要的额外的字节数（如果 Tvb 没有全部），
-- 0 表示错误。
--
dissectLsp = function(tvbuf, pktinfo, root, offset)
    dprint2("LSP dissect function called")

    local total_len, length_tvbr = checkLspLength(tvbuf, offset)

    if total_len <= 0 then
        return total_len
    end

    dprint("checkLspLength ret: total_len:" .. total_len)

    -- 如果我们到达这里，那么我们在 Tvb 缓冲区中就有了完整的消息，所以让我们完成对其的剖析......

    -- We start by adding our protocol to the dissection display tree.
    local tree         = root:add(lansync_proto, tvbuf:range(offset, total_len))

    local version_tvbr = tvbuf:range(offset, 1)
    tree:add(hdr_fields.version, version_tvbr:uint())

    if version_tvbr:uint() < 1 or version_tvbr:uint() > 1 then
        dprint("is not lsp protocol, version(hex):" .. version_tvbr:uint())
        return 0
    end


    local msgtype_tvbr = tvbuf:range(offset + 1, 1)
    tree:add(hdr_fields.type, msgtype_tvbr:uint())

    local header_len = tvbuf:range(offset + 2, 2):uint() or 0
    tree:add(hdr_fields.header_len, header_len)
    tree:add(hdr_fields.total_len, total_len)

    local xheader_len = header_len - LSP_MSG_HDR_LEN;
    dprint("xheader_len(hex):" .. xheader_len)

    handleXheader(xheader_len, offset, tvbuf, tree, total_len)

    handleData(total_len, header_len, xheader_len, tvbuf, pktinfo, root, offset)

    handlePktInfoColsInfo(pktinfo, msgtype_tvbr, xheader_len, total_len, header_len)

    return total_len
end

function handleXheader(xheader_len, offset, tvbuf, tree, total_len)
    if xheader_len > 0 then
        local start = offset + LSP_MSG_HDR_LEN

        local xheader = tvbuf:range(start, xheader_len)
        local xheader_str = xheader:raw()

        local sub_start = start;
        for i = 0, xheader_len, 1 do
            local ch = string.byte(xheader_str, i)
            if ch == 0 then
                local sub_len = start + xheader_len - sub_start;
                print("(" .. sub_start .. "," .. sub_len .. ") xheader_len:" .. xheader_len .. " total_len:" .. total_len)
                local xheader_entry = tvbuf:range(sub_start, sub_len):raw()
                tree:add(xheader_entry)
                sub_start = start + i
            end
        end
    end
end

function handleData(total_len, header_len, xheader_len, tvbuf, pktinfo, root, offset)
    local data_len = total_len - header_len
    if data_len > 0 then
        local header_offset = LSP_MSG_HDR_LEN;
        if xheader_len > 0 then
            header_offset = header_offset + xheader_len
        end
        local data_dis = Dissector.get("data");
        local data_tvb = tvbuf:range(offset + header_offset, data_len):tvb();
        local data_tvb_len = data_tvb:len()
        dprint2("may handle data pkt, total_len(hex):" ..
            total_len ..
            ", hdr_len(hex):" .. header_len .. ", data_len(hex):" .. data_len .. ", data_tvb_len(hex):" .. data_tvb_len)
        data_dis:call(data_tvb, pktinfo, root)
    end
end

function handlePktInfoColsInfo(pktinfo, msgtype_tvbr, xheader_len, total_len, header_len)
    -- set the protocol column to show our protocol name
    local data_len = total_len - header_len
    pktinfo.cols.protocol:set("LSP")
    if string.find(tostring(pktinfo.cols.info), "^LSP") == nil then
        local info_cell = "[" .. types[msgtype_tvbr:uint()] .. "]"
        if xheader_len > 0 then
            info_cell = info_cell .. " xhdlen=" .. xheader_len
        end
        info_cell = info_cell .. " dataLen=" .. data_len
        pktinfo.cols.info:set(info_cell)
    end
end

----------------------------------------
-- The function to check the length field.
--
-- 这会返回两个内容：
-- (1) 长度，
-- (2) TvbRange 对象
-- 如果 length <= 0，则该对象可能为零。
checkLspLength = function(tvbuf, offset)
    -- “msglen”是 Tvb 缓冲区中剩余的字节数，我们可以在本次运行中分析它
    local msglen = tvbuf:len() - offset

    -- check if capture was only capturing partial packet size
    if msglen ~= tvbuf:reported_length_remaining(offset) then
        -- 捕获的数据包正在被sliced/cut-off，所以不要试图 desegment/reassemble
        dprint2("Captured packet was shorter than original, can't reassemble")
        return 0
    end

    if msglen < LSP_MSG_HDR_LEN then
        -- 我们需要更多字节，所以告诉 the main dissector function我们没有解析任何东西，并且我们需要未知数量的更多字节（这就是“DESEGMENT_ONE_MORE_SEGMENT”的用途）
        dprint2("Need more bytes to figure out LSP length field")
        -- return as a negative number
        return -DESEGMENT_ONE_MORE_SEGMENT
    end

    -- 如果我们到达这里，那么我们知道 Tvb 缓冲区中有足够的字节，至少可以算出该 LSP 消息的完整长度（长度是第三个和第四个字节中的 16 位整数）

    -- 以网络顺序（大端）获取无符号整数的长度
    -- local length_val = length_tvbr:uint()
    local length_tvbr = tvbuf:range(offset + 4, 4)
    local total_len = length_tvbr:uint() or 0;

    dprint2("offset:" .. offset .. "\tmsg_len:" .. msglen .. '\ttotal_len:' .. total_len)

    if msglen < total_len then
        -- we need more bytes to get the whole LSP message
        dprint2("Need more bytes to desegment full LSP")
        return -(total_len - msglen)
    end

    return total_len, length_tvbr
end



--------------------------------------------------------------------------------
-- We want to have our protocol dissection invoked for a specific TCP port,
-- so get the TCP dissector table and add our protocol to it.
local function enableDissector()
    -- using DissectorTable:set() removes existing dissector(s), whereas the
    -- DissectorTable:add() one adds ours before any existing ones, but
    -- leaves the other ones alone, which is better
    DissectorTable.get("tcp.port"):add(58081, lansync_proto)
    DissectorTable.get("udp.port"):add(58080, lansync_proto)
end
-- call it now, because we're enabled by default
enableDissector()

local function disableDissector()
    DissectorTable.get("tcp.port"):remove(58081, lansync_proto)
    DissectorTable.get("udp.port"):remove(58080, lansync_proto)
end


--------------------------------------------------------------------------------
-- preferences handling stuff
--------------------------------------------------------------------------------

local debug_pref_enum          = {
    { 1, "Disabled", debug_level.DISABLED },
    { 2, "Level 1",  debug_level.LEVEL_1 },
    { 3, "Level 2",  debug_level.LEVEL_2 },
}

----------------------------------------
-- register our preferences
lansync_proto.prefs.enabled    = Pref.bool("Dissector enabled", default_settings.enabled,
    "Whether the LSP dissector is enabled or not")

lansync_proto.prefs.subdissect = Pref.bool("Enable sub-dissectors", default_settings.subdissect,
    "Whether the LSP packet's content" ..
    " should be dissected or not")

lansync_proto.prefs.debug      = Pref.enum("Debug", default_settings.debug_level,
    "The debug printing level", debug_pref_enum)

----------------------------------------
-- the function for handling preferences being changed
function lansync_proto.prefs_changed()
    dprint2("prefs_changed called")

    default_settings.subdissect  = lansync_proto.prefs.subdissect

    default_settings.debug_level = lansync_proto.prefs.debug
    resetDebugLevel()

    if default_settings.enabled ~= lansync_proto.prefs.enabled then
        default_settings.enabled = lansync_proto.prefs.enabled
        if default_settings.enabled then
            enableDissector()
        else
            disableDissector()
        end
        -- have to reload the capture file for this type of change
        reload()
    end
end

dprint2("pcapfile Prefs registered")
