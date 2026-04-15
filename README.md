# Gateway_Control_System

# 安装和运行

## software

* vscode(cmake\git\c++\gdb\Remote Development等插件)

* git

* docker

## docker

* 安装HYPER-V、WSL2、Docker Desktop

* 配置镜像源
  
  ```
    "registry-mirrors": [
        "https://registry.docker-cn.com",
        "https://docker.mirrors.ustc.edu.cn"
    ]
  ```

* 安装镜像
  
  * ubuntu:24.04

* 构建镜像
  
  ```
  进入项目根目录运行：
  root& docker build -f docker/Dockerfile -t ubuntu_cpp:v2 .
  ```

* 启动容器
  
```
  root& docker run -it -P -d --network bridge -v [${PROJECT_DIR}]:/home/Gateway_Control_System --name control-env ubuntu_cpp:v2

  for example(宿主机和docker网络为host模式)：sudo docker run -itd \
  --name gateway-env \
  --net=host \
  --privileged \
  -v /opt/gateway/Gateway_Control_System:/home/Gateway_Control_System \
  -v /opt/gateway/Gateway_Management_System:/home/Gateway_Management_System \
  -e TZ=Asia/Shanghai \
  -e NTP_SERVERS=cn.pool.ntp.org \
  gateway-env:backup

  for example1(windows): root& docker run -it -P -d --network bridge -v Y:\Studyplace_Web_Development\Gateway_Control_System:/home/Gateway_Control_System -v  Y:\Studyplace_Web_Development\Gateway_Management_System:/home/Gateway_Management_System --name gateway-env --privileged --env "TZ=Asia/Shanghai" --env "NTP_SERVERS=cn.pool.ntp.org" ubuntu_cpp:v2
 for example2(linux,固定端口版本，arm64架构要考虑用host网络模式):root&  docker run -it -d --network bridge -p 40022:22 -p 40080:80 -p 41234:1234 -p 45672:5672 -p 46379:6379 -p 47100:7100 -p 47101:7101 -p 47205:7205 -p 47206:7206 -p 48080:8080 -p 48384:8384 -p 48800:8800 -p 48801:8801 -p 45672:15672 -p 49999:19999 -v /home/orangepi/Gateway_Control_System/:/home/Gateway_Control_System -v /home/orangepi/Gateway_Management_System/:/home/Gateway_Management_System --name gateway-env --privileged --env "TZ=Asia/Shanghai" --env  "NTP_SERVERS=cn.pool.ntp.org" gateway-env
```

* 配置完成，通过vscode附加到运行的容器环境

* 在doker中通过systen_config文件夹内容进一步配置

# 工程目录

```
Gateway_Control_System
├─ 📁.vscode
├─ 📁bin
├─ 📁build
├─ 📁config
│  └─ 📄defconfig.ini
├─ 📁docker
│  └─ 📄Dockerfile
├─ 📁driver
│  ├─ 📁ethernet
│  ├─ 📁opticalfiber
│  ├─ 📁radiodigital
│  ├─ 📁satellite
│  ├─ 📁serial
│  ├─ 📁underwateracoustic
│  └─ 📄功能说明.md
├─ 📁lib
├─ 📁logs
├─ 📁reactor
│  └─ 📄功能需求.md
├─ 📁test
├─ 📁utility
│  ├─ 📁configparser
│  ├─ 📁logger
│  ├─ 📁mysqlpool
│  ├─ 📁rabbitmqclient
│  ├─ 📁singleton
│  ├─ 📁systime
│  ├─ 📁threadpool
│  └─ 📄功能需求.md
├─ 📄.gitignore
├─ 📄CMakeLists.txt
└─ 📄README.md
```
