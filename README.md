## 泛在无线网络团队SDN项目AP侧项目
SDN（软件定义网络）是一种核心思想将控制与数据分离的新型网络架构，该项目将SDN架构扩展至无线领域，使SDN控制器可同时对有线的SDN交换机以及无线的接入点AP进行配置管理、控制

#### 目标  
1. 为SDN架构的WLAN网络中的AP提供控制与管理功能，功能包括对AP运行的无线参数进行配置，以及关联于AP的STA进行控制  
2. 对SDN架构的WLAN网络中的AP运行状况进行监测，统计数据进行收集  

#### 内容  
AP端主要实现与控制器onos之间的ACAMP协议通信，使用c/uci做实现  

####环境  
1. 下载openwrtSDK交叉编译环境  
https://downloads.openwrt.org/chaos_calmer/15.05/ar71xx/generic/OpenWrt-SDK-15.05-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-x86_64.tar.bz2  
2. 在路由器中刷入openwrt固件  
https://downloads.openwrt.org/chaos_calmer/15.05/ar71xx/generic/openwrt-15.05-ar71xx-generic-el-mini-squashfs-factory.bin  

#### 运行  
1. 在openwrtSDK中交叉编译得到ipk  
2. 拷贝至路由器中使用opkg install安装  
3. 运行acamp，注意执行目录需要存在setting.conf  

