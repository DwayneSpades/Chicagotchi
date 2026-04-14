packets = {}

-- user connect

-- uint32_t id
-- str name
-- uint16_t* spriteData

-- user move
-- uint32_t id
-- int x
-- int y

function recvPacket(packet)
    packets[packet.id](packet)
end

function sendPacket(packet)
    myrtle.sendPacket(packet)
end

function userConnect(packet)
    local id = packet.pop_uint32_t();
    local name = packet.pop_string();
    local spriteData = packet.pop_array_uint16_t();

    --
end

myrtle.sendMessage = sendMessage
