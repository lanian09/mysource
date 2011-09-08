#!/usr/bin/sh
if [ $1 = "0" ]; then
	/usr/bin/rsh -l root SCMB /opt/VRTSvcs/bin/hagrp -switch SCM_SG -to SCMA
else
	/usr/bin/rsh -l root SCMA /opt/VRTSvcs/bin/hagrp -switch SCM_SG -to SCMB
fi
