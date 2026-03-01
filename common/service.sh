#!/system/bin/sh
MODDIR=${0%/*}

# 等待系统开机完成
until [ "$(getprop sys.boot_completed)" = "1" ]; do
    sleep 2
done

chmod +x $MODDIR/omnisched_daemon

nohup $MODDIR/omnisched_daemon > /dev/null 2>&1 &
