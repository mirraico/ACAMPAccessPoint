--ACAMP Wireshark Dissector

do
	local ACAMP_UDP_PORT = 6606

	local proto_acamp = Proto("ACAMP", "AP Control And Management Protocol")

	local msg_type = {
		[0x0001] = "Keep Alive Request",
		[0x0002] = "Keep Alive Response",
		[0x0003] = "Discovery Request",
		[0x0004] = "Discovery Response",
		[0x0101] = "Register Request",
		[0x0102] = "Register Reponse",
		[0x0103] = "Unregister Request",
		[0x0104] = "Unregister Response",
		[0x0201] = "Configuration Update Request",
		[0x0202] = "Configuration Upadte Response",
		[0x0203] = "Configuration Request",
		[0x0204] = "Configuration Report",
		[0x0301] = "Station Configuration Update Request",
		[0x0302] = "Station Configuration Upadte Response",
		[0x0303] = "Station Configuration Request",
		[0x0304] = "Station Configuration Response",	
		[0x0401] = "Statistic Request",
		[0x0402] = "Statistic Report"
	}

	local msg_elem_type = {
		[0x0001] = "Result Code",
		[0x0002] = "Reason Code",
		[0x0003] = "Assigned APID",
		[0x0004] = "Discovery Type",
		[0x0005] = "Registered Service",
		[0x0006] = "Controller Name",
		[0x0007] = "Controller Descriptor",
		[0x0008] = "Controlelr IP Address",
		[0x0009] = "Controller Mac Address",
		[0x000a] = "AP Name",
		[0x000b] = "AP Descriptor",
		[0x000c] = "AP IP Address",
		[0x000d] = "AP Mac Address",
		[0x000e] = "Returned Message Element",
		[0x0101] = "SSID",
		[0x0102] = "Channel",
		[0x0103] = "Hardware Mode",
		[0x0104] = "Suppress SSID",
		[0x0105] = "Security Setting",
		[0x0201] = "WPA Version",
		[0x0202] = "WPA Passphrase",
		[0x0203] = "WPA Key Management",
		[0x0204] = "WPA Pairwise",
		[0x0205] = "WPA Group Rekey",
		[0x0301] = "WEP Default Key",
		[0x0302] = "WEP Key",
		[0x0501] = "MAC ACL Mode",
		[0x0502] = "MAC Accept List",
		[0x0503] = "MAC Deny List",
	}

	local f_acamp_version = ProtoField.uint8("ACAMP.Version", "Version", base.DEC)
	local f_acamp_type = ProtoField.uint8("ACAMP.Type", "Type", base.HEX, {[0x00]="Control Message"})
	local f_acamp_apid = ProtoField.uint16("ACAMP.APID", "APID", base.DEC)
	local f_acamp_seqnum = ProtoField.uint32("ACAMP.SeqNum", "Seq Num", base.DEC)
	local f_acamp_msg_type = ProtoField.uint16("ACAMP.MsgType", "Msg Type", base.HEX, msg_type)
	local f_acamp_msg_len = ProtoField.uint16("ACAMP.MsgLen", "Msg Len", base.DEC)

	local f_element_type = ProtoField.uint16("ACAMP.ElemType", "Element Type", base.HEX, msg_elem_type)
	local f_element_len = ProtoField.uint16("ACAMP.ElemType", "Element Len", base.DEC)
	local f_element_data = ProtoField.string("ACAMP.ElemData", "Element Data")

	proto_acamp.fields = {f_acamp_version, f_acamp_type, f_acamp_apid, f_acamp_seqnum, f_acamp_msg_type, f_acamp_msg_len, 
								f_element_type, f_element_len, f_element_data}
	local data_dis = Dissector.get("data")

	local function proto_acamp_dissector(buffer, pinfo, tree)
		local v_buffer_len = buffer:len()
		if v_buffer_len < 16 then return false end

		pinfo.cols.protocol:set("ACAMP")
		local v_header = buffer(0, 16)
		local v_element = buffer(16, v_buffer_len - 16)

		local v_apid = buffer(2, 2)
		local t = tree:add(proto_acamp, buffer(0, v_buffer_len), "AP Control And Management Protocol, APID: " .. v_apid:uint())
		local th = t.add(t, v_header, "ACAMP Header")
		local tes
		if v_buffer_len > 16 then
			tes = t.add(t, v_element, "ACAMP Elements")
		end

		th:add(f_acamp_version, v_header(0, 1))
		th:add(f_acamp_type, v_header(1, 1))
		th:add(f_acamp_apid, v_header(2, 2))
		th:add(f_acamp_seqnum, v_header(4, 4))
		th:add(f_acamp_msg_type, v_header(8, 2))
		th:add(f_acamp_msg_len, v_header(10, 2))
		
		local v_msg_type = v_header(8, 2)
		pinfo.cols.info:set(msg_type[v_msg_type:uint()])

		local p_elem = 0
		while p_elem < v_element:len() do
			local v_element_type = v_element(p_elem, 2)
			local v_element_len = v_element(p_elem + 2, 2)
			local element_len = v_element_len:uint()
			if element_len ~= 0 then 
				local v_element_data = v_element(p_elem + 4, element_len)
			end
			local element_totlen = element_len + 4
			local te = tes.add(tes, v_element(p_elem, element_totlen), msg_elem_type[v_element_type:uint()])
		
			te:add(f_element_type, v_element(p_elem, 2))
			te:add(f_element_len, v_element(p_elem + 2, 2))
			if element_len ~= 0 then 
				te:add(f_element_data, v_element(p_elem + 4, element_len))
			end
			p_elem = (p_elem + element_totlen)
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
