do
	local ACAMP_UDP_PORT = 1080
	local ACAMP_PREAMBLE = 0x00000001

	local proto_acamp = Proto("ACAMP", "AP Control And Management Protocol")

	local msg_type = {
		[0x0011] = "Register Request",
		[0x0012] = "Register Response",
		[0x0013] = "Disconnect Request",
		[0x0014] = "Disconnect Response",
		[0x0021] = "Configuration Request",
		[0x0022] = "Configuration_Response",
		[0x0023] = "Configuration Reset Request",
		[0x0024] = "Configuration Reset Response",
		[0x0031] = "Statistic Stat Response",
		[0x0032] = "Statistic Stat Query",
		[0x0033] = "Statistic Stat Reply",
		[0x0041] = "Stat Request",
		[0x0042] = "Stat Response"
	}

	local msg_ele_type = {
		[0x0001] = "Result Code", 
		[0x0002] = "Reason Code", 
		[0x0003] = "Assigned APID", 
		[0x0101] = "AP Mac Addr", 
		[0x0102] = "AP Inet Addr", 
		[0x0103] = "AP Name", 
		[0x0104] = "AP Description", 
		[0x0105] = "AP Location", 
		[0x0106] = "AP Board Data", 
		[0x0201] = "AC Mac Addr", 
		[0x0202] = "AC Inet Addr", 
		[0x0203] = "Time Stamp", 
		[0x0301] = "Wlan Info", 
		[0x0302] = "AP Radio Info", 
		[0x0303] = "Antenna", 
		[0x0304] = "Tx Power", 
		[0x0305] = "Multi-Domain Capability", 
		[0x0306] = "Supported Rates", 
		[0x0401] = "Add Mac ACL Entry", 
		[0x0402] = "Del Mac ACL Entry", 
		[0x0403] = "Add Station", 
		[0x0404] = "Del Station", 
		[0x0501] = "Station Event"
	}

	local f_acamp_preamble = ProtoField.uint32("ACAMP.Preamble", "Preamble", base.HEX)
	local f_acamp_version = ProtoField.uint8("ACAMP.Version", "Version", base.DEC)
	local f_acamp_type = ProtoField.uint8("ACAMP.Type", "Type", base.HEX, {[0x00]="Control Message"})
	local f_acamp_apid = ProtoField.uint16("ACAMP.APID", "APID", base.DEC)
	local f_acamp_seqnum = ProtoField.uint32("ACAMP.SeqNum", "Seq Num", base.DEC)
	local f_acamp_msg_type = ProtoField.uint16("ACAMP.MsgType", "Msg Type", base.HEX, msg_type)
	local f_acamp_msg_len = ProtoField.uint16("ACAMP.MsgLen", "Msg Len", base.DEC)

	local f_element_type = ProtoField.uint16("ACAMP.ELE_TYPE", "Element Type", base.HEX, msg_ele_type)
	local f_element_len = ProtoField.uint16("ACAMP.ELE_TYPE", "Element Len", base.DEC)
	local f_element_data = ProtoField.string("ACAMP.ELE_DATA", "Element Data")

	proto_acamp.fields = {f_acamp_preamble, f_acamp_version, f_acamp_type, f_acamp_apid, f_acamp_seqnum, f_acamp_msg_type, f_acamp_msg_len, 
								f_element_type, f_element_len, f_element_data}
	local data_dis = Dissector.get("data")

	local function proto_acamp_dissector(buffer, pinfo, tree)
		local v_buffer_len = buffer:len()
		if v_buffer_len < 16 then return false end
		local v_preamble = buffer(0, 4)
		if (v_preamble:uint() ~= ACAMP_PREAMBLE) then return false end

		pinfo.cols.protocol:set("ACAMP")
		local v_header = buffer(0, 16)
		local v_element = buffer(16, v_buffer_len - 16)

		local v_apid = buffer(6, 2)
		local t = tree:add(proto_acamp, buffer(0, v_buffer_len), "AP Control And Management Protocol, APID: " .. v_apid:uint())
		local th = t.add(t, v_header, "ACAMP Header")
		local tes = t.add(t, v_element, "ACAMP Elements")

		th:add(f_acamp_preamble, v_header(0, 4))
		th:add(f_acamp_version, v_header(4, 1))
		th:add(f_acamp_type, v_header(5, 1))
		th:add(f_acamp_apid, v_header(6, 2))
		th:add(f_acamp_seqnum, v_header(8, 4))
		th:add(f_acamp_msg_type, v_header(12, 2))
		th:add(f_acamp_msg_len, v_header(14, 2))
		
		local p_ele = 0
		while p_ele < v_element:len() do
			local v_element_type = v_element(p_ele, 2)
			local v_element_len = v_element(p_ele + 2, 2)
			local element_len = v_element_len:uint()
			local v_element_data = v_element(p_ele + 4, element_len)
			local element_totlen = element_len + 4
			local te = tes.add(tes, v_element(p_ele, element_totlen), msg_ele_type[v_element_type:uint()])
		
			te:add(f_element_type, v_element(p_ele, 2))
			te:add(f_element_len, v_element(p_ele + 2, 2))
			te:add(f_element_data, v_element(p_ele + 4, element_len))
		
			p_ele = (p_ele + element_totlen)
		end

		return true
	end


	function proto_acamp.dissector(buffer, pinfo, tree)
		if(proto_acamp_dissector(buffer, pinfo, tree)) then
		else data_dis:call(buffer, pinfo, tree) 
		end
	end
			

	local udp_port_table = DissectorTable.get("udp.port")
	udp_port_table:add(ACAMP_UDP_PORT, proto_acamp)
end
