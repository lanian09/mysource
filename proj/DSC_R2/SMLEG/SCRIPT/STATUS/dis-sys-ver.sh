#!/usr/bin/bash
#dis-sys-ver infomation
/usr/bin/uname -a | awk ' { printf "%s \n", $3}'
/usr/bin/mysql -V | awk ' { printf "%s \n", $5 }'| cut -d ',' -f1
/SM/pcube/lib/tt/TimesTen/pcubesm22/bin/ttversion | grep "Release" | awk '{printf "%s \n", $3}'
