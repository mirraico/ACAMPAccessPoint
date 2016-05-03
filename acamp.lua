do
	local ACAMP_UDP_PORT = 1080
	local ACAMP_PREAMBLE = 0x00000001

	local proto_acamp = Proto("ACAMP", "AP Control And Management Protocol")

	local msg_type = {
		[0x0101] = "Register Request",
		[0x0102] = "Register Response",
		[0x0103] = "Disconnect Request",
		[0x0104] = "Disconnect Response",
		[0x0201] = "Configuration Request",
		[0x0202] = "Configuration_Response",
		[0x0203] = "Configuration Reset Request",
		[0x0204] = "Configuration Reset Response",
		[0x0301] = "Statistic Stat Response",
		[0x0302] = "Statistic Stat Query",
		[0x0303] = "Statistic Stat Reply",
		[0x0401] = "Stat Request",
		[0x0402] = "Stat Response"
	}

<<<<<<< HEAD
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
=======

	local msg_ele_type = {
		[0x0001] = "Result Code",
		[0x0002] = "Reason Code",
		[0x0003] = "Assigned APID",
		[0x0101] = "AP MAC Addr",
		[0x0102] = "AP Inet Addr",
		[0x0103] = "AP Name",
		[0x0104] = "AP Descriptor",
		[0x0105] = "AP Location",
		[0x0106] = "AP Board Data",
		[0x0201] = "AC MAC Addr",
		[0x0202] = "AC Inet Addr",
		[0x0203] = "Timestamp",
		[0x0301] = "Wlan Information",
		[0x0302] = "AP Radio Information",
		[0x0303] = "Antenna",
		[0x0304] = "Tx Power",
		[0x0305] = "Multi-Domain Capability",
		[0x0306] = "Supported Rates",
		[0x0401] = "Add MAC ACL Entry",
		[0x0402] = "Delete MAC ACL Entry",
		[0x0403] = "Add Station",
		[0x0404] = "Delete Station",
>>>>>>> 956dd49e05033b1d54fb68da17f02dafce693f8e
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
<<<<<<< HEAD
	local f_element_len = ProtoField.uint16("ACAMP.ELE_TYPE", "Element Len", base.DEC)
	local f_element_data = ProtoField.string("ACAMP.ELE_DATA", "Element Data")

	proto_acamp.fields = {f_acamp_preamble, f_acamp_version, f_acamp_type, f_acamp_apid, f_acamp_seqnum, f_acamp_msg_type, f_acamp_msg_len, 
								f_element_type, f_element_len, f_element_data}
	local data_dis = Dissector.get("data")

=======
	local f_element_len = ProtoField.uint16("ACAMP.ELE_LEN", "Element Len", base.DEC)

	local f_element_data = ProtoField.string("ACAMP.ELE_DATA", "Element Data")
	local f_eledat_ResultCode = ProtoField.uint8("ACAMP.ELE_DATA", "Element Data",base.HEX,  {[0x00]="Request Failure", [0x01]="Request Sucess"})
	local f_eledat_ReasonCode = ProtoField.uint32("ACAMP.ELE_DATA", "Element Data",base.HEX,  {[0x2010] = "No Reason", [0x0102] = "AC No Response", [0x1020] = "AP No Response"})
	local f_eledat_AssignedAPID = ProtoField.uint8("ACAMP.ELE_DATA", "Element Data", base.DEC)
	local f_eledat_apMacAddr =  ProtoField.string("ACAMP.ELE_DATA", "Element Data") --TODO
	local f_eledat_apInetAddr = ProtoField.string("ACAMP.ELE_DATA", "Element Data") --TODO
	local f_eledat_apName = ProtoField.string("ACAMP.ELE_DATA", "Element Data")
	local f_eledat_apDescriptor = ProtoField.string("ACAMP.ELE_DATA", "Element Data") 
	local f_eledat_apLocation = ProtoField.string("ACAMP.ELE_DATA", "Element Data")
	local f_eledat_apBoardData = ProtoField.string("ACAMP.ELE_DATA", "Element Data")
	local f_eledat_acMacAddr = ProtoField.string("ACAMP.ELE_DATA", "Element Data") --TODO
	local f_eledat_acInetAddr = ProtoField.string("ACAMP.ELE_DATA", "Element Data") --TODO
	local f_eledat_TimeStamp = ProtoField.uint32("ACAMP.SeqNum", "Element Data", base.DEC)
	local f_eledat_WlanInfo_RadioInfo = ProtoField.uint8("ACAMP.ELE_DATA", "Radio Infomation", base.DEC)
	local f_eledat_WlanInfo_WlanId = ProtoField.uint8("ACAMP.ELE_DATA", "Wlan ID", base.DEC)
	local f_eledat_WlanInfo_Capability = ProtoField.string("ACAMP.ELE_DATA", "Capability")  
											--TODO
	local f_eledat_WlanInfo_KeyIndex = ProtoField.uint8("ACAMP.ELE_DATA", "Key Index", base.DEC)
	local f_eledat_WlanInfo_KeyStatus = ProtoField.uint8("ACAMP.ELE_DATA", "Key Status", base.HEX, {[0x00]="WLAN Uses Per-station Keys", [0x01]="WLAN Uses Share Wired Equivalent Privacy", [0x02]="WLAN Starts Updating STA's GTK of BSS",[0x03]="WLAN  Updated STA's GTK of BSS"})
	local f_eledat_WlanInfo_KeyLength = ProtoField.uint16("ACAMP.ELE_DATA", "Key Length", base.DEC)
	local f_eledat_WlanInfo_Key = ProtoField.string("ACAMP.ELE_DATA", "Key")  
	local f_eledat_WlanInfo_GroupTSC = ProtoField.string("ACAMP.ELE_DATA", "Group TSC")
	local f_eledat_WlanInfo_QoS = ProtoField.uint8("ACAMP.ELE_DATA", "QoS", base.HEX, {[0x00]="Best Effort", [0x01]="Video", [0x02]="Voice",[0x03]="Backgroud"})
	local f_eledat_WlanInfo_AuthType = ProtoField.uint8("ACAMP.ELE_DATA", "Auth Type", base.HEX, {[0x00]="Open System", [0x01]="WEP Shared Key"})
	local f_eledat_WlanInfo_SuppressSSID = ProtoField.uint8("ACAMP.ELE_DATA", "Suppress SSID", base.HEX, {[0x00]="NO SSID", [0x01]="Including SSID"})
	local f_eledat_WlanInfo_SSID = ProtoField.string("ACAMP.ELE_DATA", "SSID")  

	local f_eledat_apRadiInfo_RadioID =ProtoField.uint8("ACAMP.ELE_DATA", "Radio ID")
local f_eledat_apRadiInfo_ShortPreamble = ProtoField.uint8("ACAMP.ELE_DATA", "Short Preamble", base.HEX, {[0x00]="Supporting Preamble", [0x01]="Not Supporting Preamble"})
local f_eledat_apRadiInfo_NumofBSSIDs = ProtoField.uint8("ACAMP.ELE_DATA", "Num of BSSIDs")
local f_eledat_apRadiInfo_DTIMPeriod = ProtoField.uint16("ACAMP.ELE_DATA", "DTIM Period",base.DEX)
local f_eledat_apRadiInfo_BSSID = ProtoField.string("ACAMP.ELE_DATA", "BSSID")  
local f_eledat_apRadiInfo_BeaconPeriod = ProtoField.uint16("ACAMP.ELE_DATA", "Beacon Period",base.DEX)
local f_eledat_apRadiInfo_CountryString = ProtoField.string("ACAMP.ELE_DATA", "Country String") 

local f_eledat_Antenna_RadioID= ProtoField.uint8("ACAMP.ELE_DATA", "Radio ID",base.DEX)
local f_eledat_Antenna_Diversity = ProtoField.uint8("ACAMP.ELE_DATA", "Diversity", base.HEX, {[0x00]="Enable Diversity", [0x01]="unable Diversity"}) 
local f_eledat_Antenna_Combiner = ProtoField.uint8("ACAMP.ELE_DATA", "Combiner", base.HEX, {[0x01]="Sectorized (Left)", [0x02]="Sectorized (Right)",  [0x03]="Omni",  [0x04]="Multiple Input/Multiple Output (MIMO) Diversity"}) 
local f_eledat_Antenna_AntennaCnt = ProtoField.uint8("ACAMP.ELE_DATA", "Antenna Count",base.DEX)
local f_eledat_Antenna_Selection = ProtoField.uint32("ACAMP.ELE_DATA", "Antenna Selection", base.HEX, {[0x01]="Internal Antenna", [0x02]="External Antenna"})
	local f_eledat_TxPower_RadioID = ProtoField.uint8("ACAMP.ELE_DATA", "Radio ID",base.DEX)
	local f_eledat_TxPower_NumLevel =ProtoField.uint8("ACAMP.ELE_DATA", "Num Level",base.DEX)
	local f_eledat_TxPower_PowerLevel = ProtoField.uint8("ACAMP.ELE_DATA", "Power Level[n]",base.DEX)

	local f_eledat_MultiDomainCapa_RadioID = ProtoField.uint8("ACAMP.ELE_DATA", "Radio ID",base.DEX)
	local f_eledat_MultiDomainCapa_Reserved = ProtoField.uint8("ACAMP.ELE_DATA", "Element Data", base.HEX, {[0x00]="Reserved"}) 
	local f_eledat_MultiDomainCapa_FirstChannel = ProtoField.uint8("ACAMP.ELE_DATA", "First Channel",base.DEX)
	local f_eledat_MultiDomainCapa_NumberofChannels = ProtoField.uint16("ACAMP.ELE_DATA", "Number of Channels",base.DEX)
	local f_eledat_MultiDomainCapa_MaxTxPowerLevel = ProtoField.uint16("ACAMP.ELE_DATA", "Max Tx Power Level",base.DEX)

	local f_eledat_SupportedRates_RadioID = ProtoField.uint8("ACAMP.ELE_DATA", "Radio ID",base.DEX)
local f_eledat_SupportedRates_SupportedRates = ProtoField.string("ACAMP.ELE_DATA", "Supported Rates")   --TODO

	local f_eledat_AddMacACLEntry_NumofEntries = ProtoField.uint8("ACAMP.ELE_DATA", "Num of Entries", base.DEX)
local f_eledat_AddMacACLEntry_MacAddress= ProtoField.string("ACAMP.ELE_DATA", "Mac Address")   --TODO	
	local f_eledat_DelMacACLEntry_NumofEntries = ProtoField.uint8("ACAMP.ELE_DATA", "Num of Entries",base.DEX)
	local f_eledat_DelMacACLEntry_MacAddress = ProtoField.string("ACAMP.ELE_DATA", "Mac Address")   --TODO
	
local f_eledat_AddStation_STAMacAddr = ProtoField.string("ACAMP.ELE_DATA", "STA MacAddr")   --TODO
	local f_eledat_DelStation_STAMacAddr = ProtoField.string("ACAMP.ELE_DATA", "STA Mac Addr")   --TODO
	local f_eledat_StationEvent_STAMacAddr = ProtoField.string("ACAMP.ELE_DATA", "STA Mac Addr")   --TODO
	

	proto_acamp.fields = {f_acamp_preamble, f_acamp_version, f_acamp_type, f_acamp_apid, f_acamp_seqnum, f_acamp_msg_type, f_acamp_msg_len, f_element_type, f_element_len, f_eledat_ResultCode, f_eledat_ReasonCode, f_eledat_AssignedAPID, f_eledat_apMacAddr,
f_eledat_apInetAddr, f_eledat_apName, f_eledat_apDescriptor,f_eledat_apLocation, f_eledat_apBoardData, f_eledat_acMacAddr, f_eledat_acInetAddr, f_eledat_TimeStamp, f_eledat_WlanInfo_RadioInfo, f_eledat_WlanInfo_WlanId,f_eledat_WlanInfo_Capability, f_eledat_WlanInfo_KeyIndex, f_eledat_WlanInfo_KeyStatus, f_eledat_WlanInfo_Key, f_eledat_WlanInfo_KeyLength, f_eledat_WlanInfo_GroupTSC, f_eledat_WlanInfo_QoS, f_eledat_WlanInfo_AuthType, f_eledat_WlanInfo_SuppressSSID, f_eledat_WlanInfo_SSID, f_eledat_apRadiInfo_RadioID, f_eledat_apRadiInfo_ShortPreamble, f_eledat_apRadiInfo_NumofBSSIDs, f_eledat_apRadiInfo_DTIMPeriod, f_eledat_apRadiInfo_BSSID, f_eledat_apRadiInfo_BeaconPeriod, f_eledat_apRadiInfo_CountryString, f_eledat_Antenna_RadioID,f_eledat_Antenna_Diversity, f_eledat_Antenna_Combiner, f_eledat_Antenna_AntennaCnt, f_eledat_Antenna_Selection, f_eledat_TxPower_RadioID, f_eledat_TxPower_NumLevel, f_eledat_TxPower_PowerLevel, f_eledat_MultiDomainCapa_RadioID, f_eledat_MultiDomainCapa_Reserved, f_eledat_MultiDomainCapa_FirstChannel, f_eledat_MultiDomainCapa_NumberofChannels, f_eledat_MultiDomainCapa_MaxTxPowerLevel, f_eledat_SupportedRates_RadioID, f_eledat_SupportedRates_SupportedRates, f_eledat_AddMacACLEntry_NumofEntries, f_eledat_AddMacACLEntry_MacAddress, f_eledat_DelMacACLEntry_NumofEntries, f_eledat_DelMacACLEntry_MacAddress, f_eledat_AddStation_STAMacAddr, f_eledat_DelStation_STAMacAddr, f_eledat_StationEvent_STAMacAddr
,f_element_data}
	local data_dis = Dissector.get("data")
	
>>>>>>> 956dd49e05033b1d54fb68da17f02dafce693f8e
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
<<<<<<< HEAD
			local te = tes.add(tes, v_element(p_ele, element_totlen), msg_ele_type[v_element_type:uint()])
		
			te:add(f_element_type, v_element(p_ele, 2))
			te:add(f_element_len, v_element(p_ele + 2, 2))
			te:add(f_element_data, v_element(p_ele + 4, element_len))
		
			p_ele = (p_ele + element_totlen)
		end

		return true
	end


=======
			local te = tes.add(tes, v_element(p_ele, element_totlen), 						msg_ele_type[v_element_type:uint()])

			te:add(f_element_type, v_element(p_ele, 2))
			te:add(f_element_len, v_element(p_ele + 2, 2))

			if (v_element_type:uint() == 0x0001) then
				te:add(f_eledat_ResultCode, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0002) then
				te:add(f_eledat_ReasonCode, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0003) then
				te:add(f_eledat_AssignedAPID, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0101) then
				te:add(f_eledat_apMacAddr, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0102) then
				te:add(f_eledat_apInetAddr, v_element(p_ele + 4, element_len))	
			elseif(v_element_type:uint() == 0x0103) then
				te:add(f_eledat_apName, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0104) then
				te:add(f_eledat_apDescriptor, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0105) then
				te:add(f_eledat_apLocation, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0106) then
				te:add(f_eledat_apBoardData, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0201) then
				te:add(f_eledat_acMacAddr, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0202) then
				te:add(f_eledat_acInetAddr, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0203) then
				te:add(f_eledat_TimeStamp, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0301) then
				te:add(f_eledat_WlanInfo_RadioInfo, v_element(p_ele + 4, 1))
				te:add(f_eledat_WlanInfo_WlanId, v_element(p_ele + 5, 1))
				te:add(f_eledat_WlanInfo_Capability, v_element(p_ele + 6, 2))
				te:add(f_eledat_WlanInfo_KeyIndex, v_element(p_ele + 8, 1))
				te:add(f_eledat_WlanInfo_Key, v_element(p_ele + 9, 1))
				te:add(f_eledat_WlanInfo_KeyLength, v_element(p_ele + 10, 2))
				te:add(f_eledat_WlanInfo_Key, v_element(p_ele + 12, 4))
				te:add(f_eledat_WlanInfo_GroupTSC, v_element(p_ele + 16, 6))
				te:add(f_eledat_WlanInfo_QoS, v_element(p_ele + 22, 1))
				te:add(f_eledat_WlanInfo_AuthType, v_element(p_ele + 23, 1))
				te:add(f_eledat_WlanInfo_SuppressSSID, v_element(p_ele + 24, 1))
				te:add(f_eledat_WlanInfo_SSID, v_element(p_ele + 25, 3))
			elseif (v_element_type:uint() == 0x0302) then
				te:add(f_eledat_apRadiInfo_RadioID, v_element(p_ele + 4, 1))
				te:add(f_eledat_apRadiInfo_ShortPreamble, v_element(p_ele + 5, 1))
				te:add(f_eledat_apRadiInfo_NumofBSSIDs, v_element(p_ele + 6, 1))
				te:add(f_eledat_apRadiInfo_DTIMPeriod, v_element(p_ele + 7, 1))
				te:add(f_eledat_apRadiInfo_BSSID, v_element(p_ele + 8, 6))
				te:add(f_eledat_apRadiInfo_BeaconPeriod, v_element(p_ele + 14, 2))
				te:add(f_eledat_apRadiInfo_CountryString, v_element(p_ele + 16, 4))
			elseif (v_element_type:uint() == 0x0303) then
				te:add(f_eledat_Antenna_RadioID, v_element(p_ele + 4, 1))
				te:add(f_eledat_Antenna_Diversity, v_element(p_ele + 5, 1))
				te:add(f_eledat_Antenna_Combiner, v_element(p_ele + 6, 1))
				te:add(f_eledat_Antenna_AntennaCnt, v_element(p_ele + 7, 1))
				te:add(f_eledat_Antenna_Selection, v_element(p_ele + 8, 4))
			elseif (v_element_type:uint() == 0x0304) then
				te:add(f_eledat_TxPower_RadioID, v_element(p_ele + 4, 1))
				te:add(f_eledat_TxPower_NumLevel, v_element(p_ele + 5, 1))
				te:add(f_eledat_TxPower_PowerLevel, v_element(p_ele + 8, 2))
			elseif (v_element_type:uint() == 0x0305) then
				te:add(f_eledat_MultiDomainCapa_RadioID, v_element(p_ele + 4, 1))
				te:add(f_eledat_MultiDomainCapa_Reserved, v_element(p_ele + 5, 1))
				te:add(f_eledat_MultiDomainCapa_FirstChannel, v_element(p_ele + 6, 2))
				te:add(f_eledat_MultiDomainCapa_NumberofChannels, v_element(p_ele + 8, 3))
				te:add(f_eledat_MultiDomainCapa_MaxTxPowerLevel, v_element(p_ele + 10, 2))
			elseif (v_element_type:uint() == 0x0306) then
				te:add(f_eledat_SupportedRates_RadioID, v_element(p_ele + 4, 1))
				te:add(f_eledat_SupportedRates_SupportedRates, v_element(p_ele + 5, 3))			
			elseif (v_element_type:uint() == 0x0401) then
				te:add(f_eledat_AddMacACLEntry_NumofEntries, v_element(p_ele + 4, 1))
				te:add(f_eledat_AddMacACLEntry_MacAddress, v_element(p_ele + 4, 3))
			elseif (v_element_type:uint() == 0x0402) then
				te:add(f_eledat_DelMacACLEntry_NumofEntries, v_element(p_ele + 4, 1))
				te:add(f_eledat_DelMacACLEntry_MacAddress, v_element(p_ele + 4, 3))
			elseif (v_element_type:uint() == 0x0403) then
				te:add(f_eledat_AddStation_STAMacAddr, v_element(p_ele + 4, element_len))
			elseif (v_element_type:uint() == 0x0404) then
				te:add(f_eledat_DelStation_STAMacAddr, v_element(p_ele + 4, element_len))

			elseif (v_element_type:uint() == 0x0501) then
				te:add(f_eledat_StationEvent_STAMacAddr, v_element(p_ele + 4, element_len))
			else 
				te:add(f_element_data, v_element(p_ele + 4, element_len))
			end
			p_ele = (p_ele + element_totlen)
		end
		return true
	end
	
>>>>>>> 956dd49e05033b1d54fb68da17f02dafce693f8e
	function proto_acamp.dissector(buffer, pinfo, tree)
		if(proto_acamp_dissector(buffer, pinfo, tree)) then
		else data_dis:call(buffer, pinfo, tree) 
		end
	end
			

	local udp_port_table = DissectorTable.get("udp.port")
	udp_port_table:add(ACAMP_UDP_PORT, proto_acamp)
end
