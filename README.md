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
  
  for example1(windows): root& docker run -it -P -d --network bridge -v Y:\Studyplace_Web_Development\Gateway_Control_System:/home/Gateway_Control_System -v Y:\Studyplace_Web_Development\Gateway_Management_System:/home/Gateway_Management_System --name gateway-env --privileged --env "TZ=Asia/Shanghai" --env "NTP_SERVERS=cn.pool.ntp.org" ubuntu_cpp:v2

 for example2(linux,固定端口版本):root&  docker run -it  -d --network bridge  -p 22:22 \
  -p 80:80 \
  -p 1234:1234 \
  -p 5672:5672 \
  -p 6379:6379 \
  -p 7100:7100 \
  -p 7101:7101 \
  -p 7205:7205 \
  -p 7206:7206 \
  -p 8080:8080 \
  -p 8800:8800 \
  -p 8801:8801 \
  -p 15672:15672 \
  -p 19999:19999 -v /home/orangepi/Gateway_Control_System/:/home/Gateway_Control_System -v /home/orangepi/Gateway_Management_System/:/home/Gateway_Management_System --name gateway-env --privileged --env "TZ=Asia/Shanghai" --env "NTP_SERVERS=cn.pool.ntp.org" ubuntu_cpp:v2

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
