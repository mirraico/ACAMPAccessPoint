--Acamp Wireshark Dissector

do
	local ACAMP_UDP_PORT = 6606

	local proto_acamp = Proto("ACAMP", "AP Control And Management Protocol")

	local msg_type = {
		[0x0001] = "Keep Alive Request",
		[0x0002] = "Keep Alive Response",
		[0x0003] = "Discover Request",
		[0x0004] = "Discover Response",
		[0x0011] = "Register Request",
		[0x0012] = "Register Reponse",
		[0x0013] = "Unregister Request",
		[0x0014] = "Unregister Response",
		[0x0021] = "Configuration Request",
		[0x0022] = "Configuration Deliver",
		[0x0023] = "Configuration Report",
		[0x0024] = "Station Configuration Deliver",
		[0x0031] = "Statistic Request",
		[0x0032] = "Statistic Report",
		[0x0041] = "State Request",
		[0x0042] = "State Report"
	}

	local msg_ele_type = {
		[0x0102] = "AP Board Data", 
		[0x0103] = "AP Description", 
		[0x0104] = "AP Name", 
		[0x030A] = "AP Radio Information"
	}

	local f_acamp_version = ProtoField.uint8("ACAMP.Version", "Version", base.DEC)
	local f_acamp_type = ProtoField.uint8("ACAMP.Type", "Type", base.HEX, {[0x00]="Control Message"})
	local f_acamp_apid = ProtoField.uint16("ACAMP.APID", "APID", base.DEC)
	local f_acamp_seqnum = ProtoField.uint32("ACAMP.SeqNum", "Seq Num", base.DEC)
	local f_acamp_msg_type = ProtoField.uint16("ACAMP.MsgType", "Msg Type", base.HEX, msg_type)
	local f_acamp_msg_len = ProtoField.uint16("ACAMP.MsgLen", "Msg Len", base.DEC)

	local f_element_type = ProtoField.uint16("ACAMP.ELE_TYPE", "Element Type", base.HEX, msg_ele_type)
	local f_element_len = ProtoField.uint16("ACAMP.ELE_TYPE", "Element Len", base.DEC)
	local f_element_data = ProtoField.string("ACAMP.ELE_DATA", "Element Data")

	proto_acamp.fields = {f_acamp_version, f_acamp_type, f_acamp_apid, f_acamp_seqnum, f_acamp_msg_type, f_acamp_msg_len, 
								f_element_type, f_element_len, f_element_data}
	local data_dis = Dissector.get("data")

	local function proto_acamp_dissector(buffer, pinfo, tree)
		local v_buffer_len = buffer:len()
		if v_buffer_len < 12 then return false end

		pinfo.cols.protocol:set("ACAMP")
		local v_header = buffer(0, 12)
		local v_element = buffer(12, v_buffer_len - 12)

		local v_apid = buffer(6, 2)
		local t = tree:add(proto_acamp, buffer(0, v_buffer_len), "AP Control And Management Protocol, APID: " .. v_apid:uint())
		local th = t.add(t, v_header, "ACAMP Header")
		if v_buffer_len > 12 then
			local tes = t.add(t, v_element, "ACAMP Elements")
		end

		th:add(f_acamp_version, v_header(0, 1))
		th:add(f_acamp_type, v_header(1, 1))
		th:add(f_acamp_apid, v_header(2, 2))
		th:add(f_acamp_seqnum, v_header(4, 4))
		th:add(f_acamp_msg_type, v_header(8, 2))
		th:add(f_acamp_msg_len, v_header(10, 2))
		
		local v_msg_type = v_header(8, 2)
		pinfo.cols.info:set(msg_type[v_msg_type:uint()])

		local p_ele = 0
		while p_ele < v_element:len() do
			local v_element_type = v_element(p_ele, 2)
			local v_element_len = v_element(p_ele + 2, 2)
			local element_len = v_element_len:uint()
			if element_len ~= 0 then 
				local v_element_data = v_element(p_ele + 4, element_len)
			end
			local element_totlen = element_len + 4
			local te = tes.add(tes, v_element(p_ele, element_totlen), msg_ele_type[v_element_type:uint()])
		
			te:add(f_element_type, v_element(p_ele, 2))
			te:add(f_element_len, v_element(p_ele + 2, 2))
			if element_len ~= 0 then 
				te:add(f_element_data, v_element(p_ele + 4, element_len))
			end
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
