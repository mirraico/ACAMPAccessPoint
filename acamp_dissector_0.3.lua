-- ACAMP Wireshark Dissector

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
		[0x0201] = "Configuration Request",
		[0x0202] = "Configuration Response",
		[0x0203] = "Configuration Update Request",
		[0x0204] = "Configuration Upadte Response",
		-- [0x0301] = "Scan Request",
		-- [0x0302] = "Scan Response",
		-- [0x0303] = "Station Request",
		-- [0x0304] = "Station Response",
		-- [0x0305] = "Statistic Request",
		-- [0x0306] = "Statistic Response",
		[0x0307] = "System Request",
		[0x0308] = "System Response",
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
		[0x0009] = "Controller MAC Address",
		[0x000a] = "AP Name",
		[0x000b] = "AP Descriptor",
		[0x000c] = "AP IP Address",
		[0x000d] = "AP MAC Address",
		-- [0x000e] = "Returned Message Element",
		[0x0010] = "Controller Next Seq Num",
		[0x0011] = "Desired Configuration List",
		[0x0101] = "SSID",
		[0x0102] = "Channel",
		[0x0103] = "Hardware Mode",
		[0x0104] = "Suppress SSID",
		[0x0105] = "Security Option",
		[0x0106] = "MAC Filter Mode",
		[0x0107] = "MAC Filter List",
		[0x0108] = "Tx Power",
		[0x0202] = "WPA Password",
		[0x0501] = "Add MAC Filter List",
		[0x0502] = "Delete MAC Filter List",
		[0x0503] = "Clear MAC Filter List",
		[0x0401] = "System Command",
		-- [0x0402] = "Scanned WLAN Info",
		-- [0x0403] = "Station Info",
		-- [0x0404] = "Statistics",

	}

	local f_acamp_version = ProtoField.uint8("ACAMP.Version", "Version", base.DEC)
	local f_acamp_type = ProtoField.uint8("ACAMP.Type", "Type", base.HEX, {[0x00]="Control Message"})
	local f_acamp_apid = ProtoField.uint16("ACAMP.APID", "APID", base.DEC)
	local f_acamp_seqnum = ProtoField.uint32("ACAMP.SeqNum", "Seq Num", base.DEC)
	local f_acamp_msg_type = ProtoField.uint16("ACAMP.MsgType", "Msg Type", base.HEX, msg_type)
	local f_acamp_msg_len = ProtoField.int16("ACAMP.MsgLen", "Msg Len", base.DEC)

	local f_element_type = ProtoField.uint16("ACAMP.ElemType", "Element Type", base.HEX, msg_elem_type)
	local f_element_len = ProtoField.int16("ACAMP.ElemType", "Element Len", base.DEC)
	-- local f_element_data = ProtoField.string("ACAMP.ElemData", "Element Data")

	local f_element_result_code = ProtoField.uint16("ACAMP.ResultCode", "Result Code", base.HEX, {[0x0000]="Success", [0x0001]="Failure", --[[[0x0002]="Unrecognized Element"--]]})
	local f_element_reason_code = ProtoField.uint16("ACAMP.ReasonCode", "Reason Code", base.HEX, {[0x0101]="Invalid Version", [0x0102]="Insufficient Resource"})
	local f_element_assigned_apid = ProtoField.uint16("ACAMP.AssignedAPID", "Assigned APID", base.DEC)
	local f_element_discovery_type = ProtoField.uint8("ACAMP.DiscoveryType", "Discovery Type", base.DEC, {[0]="Discovery", [1]="Static", [2]="Default Gateway"})
	local f_element_registered_service = ProtoField.uint8("ACAMP.RegisteredService", "Registered Service", base.HEX, {[0x00]="Configuration and station services"})
	local f_element_controller_name = ProtoField.string("ACAMP.ControllerName", "Controller Name")
	local f_element_controller_descriptor = ProtoField.string("ACAMP.ControllerDescriptor", "Controller Descriptor")
	local f_element_controller_ip_addr = ProtoField.ipv4("ACAMP.ControllerIPAddr", "Controller IP Address")
	local f_element_controller_mac_addr = ProtoField.ether("ACAMP.ControllerMACAddr", "Controller MAC Address")
	local f_element_ap_name = ProtoField.string("ACAMP.APName", "AP Name")
	local f_element_ap_descriptor = ProtoField.string("ACAMP.APDescriptor", "AP Descriptor")
	local f_element_ap_ip_addr = ProtoField.ipv4("ACAMP.APIPAddr", "AP IP Address")
	local f_element_ap_mac_addr = ProtoField.ether("ACAMP.APMACAddr", "AP MAC Address")
	-- local f_element_returned_elem = 
	local f_element_controller_seq = ProtoField.uint32("ACAMP.ControllerSeqNum", "Controller Next Seq Num", base.DEC)
	local f_list_desired_conf = ProtoField.uint16("ACAMP.DesiredConf", "Desired Configuration", base.HEX, msg_elem_type)
	local f_element_ssid = ProtoField.string("ACAMP.SSID", "SSID")
	local f_element_channel = ProtoField.uint8("ACAMP.Channel", "Channel", base.DEC)
	local f_element_hw_mode = ProtoField.uint8("ACAMP.HWMode", "Hardware Mode", base.DEC, {[0]="802.11a", [1]="802.11b", [2]="802.11g", [3]="802.11ad"})
	local f_element_suppress_ssid = ProtoField.uint8("ACAMP.SuppressSSID", "Suppress SSID", base.DEC, {[0]="Disabled", [1]="Enabled"})
	local f_element_security_option = ProtoField.uint8("ACAMP.SecurityOption", "Security Option", base.DEC, {[0]="Open", [2]="WPA/WPA2 Mixed", [3]="WPA", [4]="WPA2"})
	local f_element_filter_mode = ProtoField.uint8("ACAMP.MACFilterMode", "MAC Filter Mode", base.DEC, {[0]="None", [1]="Accept List Only", [2]="Deny List"})
	local f_filter_mac_addr = ProtoField.ether("ACAMP.MACFilter", "MAC Address")
	local f_element_txpower = ProtoField.uint8("ACAMP.TxPower", "Tx Power")
	-- local f_wep_default_key = ProtoField.uint8("ACAMP.DefaultKey", "Default Key", base.DEC)
	-- local f_wep_key_count = ProtoField.uint8("ACAMP.WEPKeyCount", "Key Count", base.DEC)
	-- local f_wep_key_num = ProtoField.uint8("ACAMP.WEPKeyNum", "Key Num", base.DEC)
	-- local f_wep_key_type = ProtoField.uint8("ACAMP.WEPKeyType", "Key Type", base.DEC, {[1]="Char 5", [2]="Char 13", [3]="Char 16", [4]="Hex 10", [5]="Hex 26", [6]="Hex 32"})
	-- local f_wep_key = ProtoField.string("ACAMP.WEPKey", "Key")
	-- local f_wpa_version = ProtoField.uint8("ACAMP.WPAVersion", "WPA Version", base.DEC, {[1]="WPA", [2]="IEEE 802.11i/RSN (WPA2)"})
	local f_element_wpa_password = ProtoField.string("ACAMP.WPAPassword", "WPA Password")
	-- local f_wpa_pairwire_cipher = ProtoField.uint8("ACAMP.WPAPairwireCipher", "WPA Pairwire Cipher", base.DEC, {[0]="TKIP", [1]="CCMP", [2]="TKIP CCMP"})
	-- local f_wpa_key_len = ProtoField.uint16("ACAMP.WPAKeyLen", "Key Len", base.DEC)
	-- local f_wpa_group_rekey = ProtoField.uint32("ACAMP.WPAGroupRekey", "Group Rekey", base.DEC)
	local f_element_system_command = ProtoField.uint8("ACAMP.SystemCommand", "System Command", base.DEC, {[0]="WLAN Down", [1]="WLAN Up", [2]="WLAN Restart", [3]="Network Restart", [4]="System Restart"})


	local element_data_switch = {
		[0x0001] = function(v_element, te, p_elem, element_len)
			te:add(f_element_result_code, v_element(p_elem + 4, element_len))
		end,
		[0x0002] = function(v_element, te, p_elem, element_len)
			te:add(f_element_reason_code, v_element(p_elem + 4, element_len))
		end,
		[0x0003] = function(v_element, te, p_elem, element_len)
			te:add(f_element_assigned_apid, v_element(p_elem + 4, element_len))
		end,
		[0x0004] = function(v_element, te, p_elem, element_len)
			te:add(f_element_discovery_type, v_element(p_elem + 4, element_len))
		end,
		[0x0005] = function(v_element, te, p_elem, element_len)
			te:add(f_element_registered_service, v_element(p_elem + 4, element_len))
		end,
		[0x0006] = function(v_element, te, p_elem, element_len)
			te:add(f_element_controller_name, v_element(p_elem + 4, element_len))
		end,
		[0x0007] = function(v_element, te, p_elem, element_len)
			te:add(f_element_controller_descriptor, v_element(p_elem + 4, element_len))
		end,
		[0x0008] = function(v_element, te, p_elem, element_len)
			te:add(f_element_controller_ip_addr, v_element(p_elem + 4, element_len))
		end,
		[0x0009] = function(v_element, te, p_elem, element_len)
			te:add(f_element_controller_mac_addr, v_element(p_elem + 4, element_len))
		end,
		[0x000a] = function(v_element, te, p_elem, element_len)
			te:add(f_element_ap_name, v_element(p_elem + 4, element_len))
		end,
		[0x000b] = function(v_element, te, p_elem, element_len)
			te:add(f_element_ap_descriptor, v_element(p_elem + 4, element_len))
		end,
		[0x000c] = function(v_element, te, p_elem, element_len)
			te:add(f_element_ap_ip_addr, v_element(p_elem + 4, element_len))
		end,
		[0x000d] = function(v_element, te, p_elem, element_len)
			te:add(f_element_ap_mac_addr, v_element(p_elem + 4, element_len))
		end,
		[0x0010] = function(v_element, te, p_elem, element_len)
			te:add(f_element_controller_seq, v_element(p_elem + 4, element_len))
		end,
		[0x0011] = function(v_element, te, p_elem, element_len)
			local p_conf_list = 0
			while p_conf_list < element_len do
				te:add(f_list_desired_conf, v_element(p_elem + 4 + p_conf_list, 2))
				p_conf_list = p_conf_list + 2
			end
		end,
		[0x0101] = function(v_element, te, p_elem, element_len)
			te:add(f_element_ssid, v_element(p_elem + 4, element_len))
		end,
		[0x0102] = function(v_element, te, p_elem, element_len)
			te:add(f_element_channel, v_element(p_elem + 4, element_len))
		end,
		[0x0103] = function(v_element, te, p_elem, element_len)
			te:add(f_element_hw_mode, v_element(p_elem + 4, element_len))
		end,
		[0x0104] = function(v_element, te, p_elem, element_len)
			te:add(f_element_suppress_ssid, v_element(p_elem + 4, element_len))
		end,
		[0x0105] = function(v_element, te, p_elem, element_len)
			te:add(f_element_security_option, v_element(p_elem + 4, element_len))
		end,
		[0x0106] = function(v_element, te, p_elem, element_len)
			te:add(f_element_filter_mode, v_element(p_elem + 4, element_len))
		end,
		[0x0107] = function(v_element, te, p_elem, element_len)
			local p_mac_list = 0
			while p_mac_list < element_len do
				te:add(f_filter_mac_addr, v_element(p_elem + 4 + p_mac_list, 6))
				p_mac_list = p_mac_list + 6
			end
		end,
		[0x0108] = function(v_element, te, p_elem, element_len)
			te:add(f_element_txpower, v_element(p_elem + 4, element_len))
		end,
		--[[
		[0x0201] = function(v_element, te, p_elem, element_len)
			local type_len = {[1]=5, [2]=13, [3]=16, [4]=10, [5]=26, [6]=32}
			
			local p_wep = 4
			te:add(f_wep_default_key, v_element(p_elem + p_wep, 1))
			te:add(f_wep_key_count, v_element(p_elem + p_wep + 1, 1))
			local wep_key_count = v_element(p_elem + p_wep + 1, 1):uint()
			p_wep = p_wep + 2
			local i = 0
			while i < wep_key_count do
				local wep_key_totlen = 2 -- num & type
				local wep_key_num = v_element(p_elem + p_wep, 1):uint()
				local wep_key_type = v_element(p_elem + p_wep + 1, 1):uint()
				wep_key_totlen = wep_key_totlen + type_len[wep_key_type]
				local v_wep = v_element(p_elem + p_wep, wep_key_totlen)

				local twep = te.add(te, v_wep, "WEP Key " .. wep_key_num)
				twep:add(f_wep_key_num, v_wep(0, 1))
				twep:add(f_wep_key_type, v_wep(1, 1))
				twep:add(f_wep_key, v_wep(2, type_len[wep_key_type]))

				p_wep = p_wep + wep_key_totlen
				i = i + 1
			end
		end,
		--]]
		[0x0202] = function(v_element, te, p_elem, element_len)
			te:add(f_element_wpa_password, v_element(p_elem + 4, element_len))
		end,
		[0x0501] = function(v_element, te, p_elem, element_len)
			local p_mac_list = 0
			while p_mac_list < element_len do
				te:add(f_filter_mac_addr, v_element(p_elem + 4 + p_mac_list, 6))
				p_mac_list = p_mac_list + 6
			end
		end,
		[0x0502] = function(v_element, te, p_elem, element_len)
			local p_mac_list = 0
			while p_mac_list < element_len do
				te:add(f_filter_mac_addr, v_element(p_elem + 4 + p_mac_list, 6))
				p_mac_list = p_mac_list + 6
			end
		end,
		-- [0x0503] = empty
		[0x0401] = function(v_element, te, p_elem, element_len)
			te:add(f_element_system_command, v_element(p_elem + 4, element_len))
		end,
	}

	-- proto_acamp.fields = {f_acamp_version, f_acamp_type, f_acamp_apid, f_acamp_seqnum, f_acamp_msg_type, f_acamp_msg_len, 
	-- 							f_element_type, f_element_len, f_element_data}
	proto_acamp.fields = {f_acamp_version, f_acamp_type, f_acamp_apid, f_acamp_seqnum, f_acamp_msg_type, f_acamp_msg_len, f_element_type, f_element_len,
								f_element_result_code, f_element_reason_code, f_element_assigned_apid, f_element_discovery_type, f_element_registered_service,
								f_element_controller_name, f_element_controller_descriptor, f_element_controller_ip_addr, f_element_controller_mac_addr,
								f_element_ap_name, f_element_ap_descriptor, f_element_ap_ip_addr, f_element_ap_mac_addr,f_element_controller_seq,f_list_desired_conf,
								f_element_ssid, f_element_channel, f_element_hw_mode, f_element_suppress_ssid, f_element_security_option, f_element_filter_mode, f_filter_mac_addr,
								f_element_txpower, f_element_wpa_password, f_element_system_command}

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
				-- te:add(f_element_data, v_element(p_elem + 4, element_len))
				local func = element_data_switch[v_element_type:uint()]
				func(v_element, te, p_elem, element_len)
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
