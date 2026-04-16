packets = {}

-- PIDs must be 0-65535
PID_HELLO = 1

-- packet functions have signature:
--  function(srcAddr: string, packet: table)
-- where srcAddr is the MAC address of the sender,
-- and the packet is a big thing o' data
packets[PID_HELLO] = function(srcAddr, packet) 
    myrtle.println("Hello from "..srcAddr.." :)")
end

function myrtle_on_peer_discovery(peerAddr)
	myrtle.println("discovered a peer!: ")
	myrtle.println("addr: "..tostring(peerAddr))
	-- now do streetpass stuff
end

function myrtle_on_peer_lost(peerAddr)
	myrtle.println("Lost a peer!: ")
	myrtle.println("addr: "..tostring(peerAddr))
end

function myrtle_on_packetrecv(peerAddr, packetId, packet)
	myrtle.println("received packet! "..tostring(packet))
	recvData = "{\n"
	for k,v in pairs(packet) do
		recvData = recvData.."\t"..tostring(k)..": "..tostring(v).."\n"
	end
	recvData = recvData.."}\n"
	myrtle.println("packet data: "..recvData)

    if (packets[packetId] ~= nil) then
        packets[packetId](peerAddr, packet)
    else
        myrtle.println("No corresponding packet function for "..packetId)
    end

end
