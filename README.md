## 泛在无线网络团队SDN项目AP侧项目

---
#### 目标
1. 为SDN架构的WLAN网络中的AP提供控制与管理功能，功能包括对AP运行的无线参数进行配置，以及关联于AP的STA进行控制。
2. 对SDN架构的WLAN网络中的AP运行状况进行监测，统计数据进行收集。
3. 为了应对未来无线网络参数的增加和变更，采用了通用的封装和传输机制以获得良好的可扩展性。

#### 内容

AP端主要实现与控制器onos之间的ACAMP协议通信，使用C语言做实现，项目wiki(搭建中)

#### 分支
master分支为主分支，用于可应用的版本的发布  
2.0develop分支为开发者分支，不能保证运行，请团队成员向该分支提交修改的代码  
develop分支为旧版本向协议2.0版本过渡的分支，现已不再使用  
  
团队成员修改请提交到2.0develop分支  
感谢团队成员贡献的代码  

#### 运行
1. 先运行控制器端ACAMP代理。
2. make
3. ./ACAMPAccessPoint

#### github相关
1. 安装github
2. 配置好global user.name & email 
3. 在该项目主页上fork该项目
4. git clone -b 2.0develop git@github.com:yourusername/ACAMPAccessPoint.git 
4. git remote add origin git@github.com:yourusername/ACAMPAccessPoint.git
6. git checkout 2.0develop //切换分支
7. git branch --set-upstream-to=2.0develop //关联远端的分支
8. git push --set-upstream 2.0develop 2.0develop// 上传远端仓库的路径 格式：[remote] [local]  

推荐使用sublime vim等编辑器进行编写，如果使用ide请务必ignore其ide产生的相关工程文件
