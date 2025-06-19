
#!/bin/bash
# Syncthing 强制重启脚本（带进程清理）

# 检查 Syncthing 是否在运行
if pgrep -x "syncthing" > /dev/null; then
    echo "🛑 发现正在运行的 Syncthing 进程（PID: $(pgrep -x "syncthing")），正在停止..."
    pkill -x "syncthing" && echo "✅ 已停止旧进程"
fi
echo "🚀 启动 Syncthing..."
syncthing -no-browser > /dev/null 2>&1 &
echo "🔄 Syncthing 已重启（新PID: $(pgrep -x "syncthing")）"
#syncthing -no-browser启动 Syncthing 并禁止自动打开浏览器页面
#> /dev/null将​​标准输出​​重定向到空设备（即丢弃所有正常输出）
#2>&1将​​标准错误输出​​也重定向到标准输出（即错误信息也会被丢弃）
#&让命令在​​后台运行​​（不阻塞当前终端）