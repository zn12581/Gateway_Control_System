## 虚拟串口

调试时使用socat虚拟串口：
    串口-串口    ：  socat -d -d pty,b115200,raw,echo=0 pty,b115200,raw,echo=0
    串口-TCP服务器:  socat -d -d pty,b115200,raw,nonblock,ignoreeof,cr,echo=0 TCP4-LISTEN:2234,reuseaddr
    串口-TCP客户端： socat -d -d pty,b115200,raw,nonblock,ignoreeof,cr,echo=0  TCP:localhost:1234

    CMD_FILEINFO = 0x01,     // 文件信息    : echo -e -n "\x69\x96\x01test.txt-31-3\x00\x00\x0D\x0A" > /dev/pts/2
    CMD_FILEID   = 0x02,     // 文件ID      : echo -e -n "\x69\x96\x02test.txt-01\x00\x00\x00\x00\x0D\x0A" > /dev/pts/2
    CMD_TUNKDATA = 0x03,     // 块数据      : echo -e -n "\x69\x96\x030000-1234567890\x0D\x0A" > /dev/pts/2
                                              echo -e -n "\x69\x96\x030001-1234567890\x0D\x0A" > /dev/pts/2
                                              echo -e -n "\x69\x96\x030002-1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0D\x0A" > /dev/pts/2
    CMD_ACK      = 0x04,     // ACK信息     : echo -e -n "\x69\x96\x0401\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0D\x0A" > /dev/pts/2
    CMD_RETRANS  = 0x05      // 重传块数据  : echo -e -n "\x69\x96\x0501-00-01\x00\x00\x00\x00\x00\x00\x00\x0D\x0A" > /dev/pts/2

## 启动rabbitmq-server服务

    root@ service rabbitmq-server restart
    注意：如果device_control出现RabbitMQ建立连接时,发生未知的Response Server错误
    请重新设置一下用户权限：
        systemctl enable rabbitmq-server                                              \
        rabbitmqctl add_user user 123456                                              \
        rabbitmqctl set_user_tags user administrator                                  \
        rabbitmqctl set_permissions -p / user ".*" ".*" ".*"                          \
        service rabbitmq-server restart                                               \
## 重启apache服务

    root@ service apache2 restart

## 配置apache服务器

* 配置/etc/apache2/apache2.conf文件
    ServerName localhost:80

* 配置/etc/apache2/sites-available/000-default.conf文件，绑定wsgi接口
    <VirtualHost *:80>
  
        ServerName localhost
      
        DocumentRoot /home/Gateway_Management_System/web
        WSGIScriptAlias / /home/Gateway_Management_System/web/webapp.wsgi
      
        ErrorLog /home/Gateway_Management_System/logs/error.log
        CustomLog /home/Gateway_Management_System/logs/access.log combined
        
        # 指定虚拟环境路径
        WSGIDaemonProcess your_app_name python-home=/opt/venv/
        WSGIProcessGroup your_app_name

        <Directory /home/Gateway_Management_System/web>
            WSGIApplicationGroup %{GLOBAL}
            Require all granted

            
        </Directory>
  
    </VirtualHost>

* 使能配置 root@ a2ensite 000-default.conf 
* 重启apache服务 root@ service apache2 restart

## 配置Syncthing文件同步功能
* 1. doker中已经完成Syncthing下载，在云服务器中，根据dokerfile命令安装Syncthing。根据在终端指令记录默认配置文件config.xml​​的地址。ubuntu下syncthingv1.29.7默认路径是/root/.local/state/state/syncthingconfig.xml​,可能会变
* 2. 在云服务器和doker中输入命令：syncthing 得到一个默认设置的文件系统，ctrl^c结束程序。
---------由于实测发现华为云远程登录访问他的web-gui界面非常不方便，通过设置开放8483端口给外部访问。---------
* 3. 在云服务器中（华为云为例）设置防火墙：华为云在控制台-实例-设置实例安全规则，设置TCP规则将8384暴露给外部![alt text](image/sync1.png)
* 4. 华为云服务器的Ubuntu中暴露sync的端口给外部
*    vim /root/.local/state/state/syncthingconfig.xml
*    修改配置文件:找到
*    <gui enabled="true" tls="false">
*    <address>127.0.0.1:8483</address>
*    </gui>
*    将127.0.0.1改为0.0.0.0
*    至此可以通过http://server1_public_IP:8483访问华为云syncthing的gui界面
---------配置结束---------
* 5. 配置syncthing,在doker和华为云中输入syncthing启动服务
    通过http://server1_public_IP:8483访问华为云公网gui（当前公网ip为60.204.187.121）
    通过http://127.0.0.1:8483访问doker下的本地gui
    配置文件夹、配置远程设备，根据官方文档https://docs.syncthing.net/intro/getting-started.html
    ![alt text](image/sync2.png)
    ![alt text](image/sync3.png)
* 6. 在gui右上角设置选项中设置gui账号密码为admin和88888888，文件夹选型中设置同步时间
![alt text](image/sync4.png)
![alt text](image/sync5.png)
## 配置Syncthing完成

## 十六进制查看文件

    root@ hexdump -C 2


## 部署到服务器启动自动启动配置（选配）
1. 给现有容器配置开机自启
docker update --restart=always gateway-env

2. 进入容器，配置启动脚本（确保容器启动时自动执行exec_management）
docker start gateway-env
docker exec -it gateway-env bash

3. 在容器内编辑~/.bashrc（或/etc/profile），追加启动命令
echo "cd /home/Gateway_Control_System/bin &&  sleep 1 &&./exec_management &" >> ~/.bashrc

4. 退出容器，重启容器生效
exit
docker restart gateway-env

5. 查看 .bashrc 末尾是否已追加命令
cat ~/.bashrc | tail -n 2

5. 检查 exec_management 是否后台运行
ps aux | grep exec_management


## 配置服务器的静态IP方便连接无线电等硬件设备（选配）
* 1. 编辑rc.local文件（开机rk3588板会自动执行这个脚本）这部分根据服务器平台配置各有不同，enP4p65s0为网卡型号需要替换
sudo tee /etc/rc.local > /dev/null << EOF
#!/bin/bash
# 配置enP4p65s0静态IP
ifconfig enP4p65s0 down
ifconfig enP4p65s0 192.168.1.100 netmask 255.255.255.0
route add default gw 192.168.1.1 enP4p65s0
ifconfig enP4p65s0 up
exit 0
EOF

* 2. 添加可执行权限（关键！否则开机不执行）
sudo chmod +x /etc/rc.local

* 3. 验证脚本（手动执行一次，确保无报错）
sudo /etc/rc.local
