##泛在无线网络团队SDN项目AP侧项目
##主要实现与控制器之间的通信
##master分支为主要分支，此分支受保护，不允许强制修改和删除
##团队成员修改请提交到community分支
##感谢团队成员贡献的代码！

1. RUN:
  1. 先运行控制器端程序。
  2. make
  3. ./ACAMPAccessPoint

2. INSTALL:
  1. 安装Qt
  2. 创建C++工程
  3. 删去main.c文件
  4. 配置github. PS:随时用git status查看状态。
     1. 添加文件.gitignore。去掉pro,pro.user,.gitignore
     2. git init
     2. git clone git@github.com:AuyeungCarl/ACAMPAccessPoint.git //将ACAMPAccessPoint里面文件考至C++工程内。若用sudo 创建，则用chown，chgrp更改文件所属。
     3. git remote add origin git@github.com:AuyeungCarl/ACAMPAccessPoint.git
     4. git branch develop //新建本地仓库，删除用-d
     5. git checkout develop,然后删除master本地仓库
     6. git branch --set-upstream-to=develop //关联远端的分支
     7. git push --set-upstream develop develop// 上传远端仓库的路径 格式：[remote] [local]
     8.修改：
	1. git add/rm 文件 //添加或者删除文件
        2. git commit -m "注释" //做注释，用“ADD/DEL/MOD ： FILE”
	3. git push
 








