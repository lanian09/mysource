#!/bin/bash

#SM operations:
#--start                 Start the SM server operation
#--restart               Stop and re-start the SM server operation
#--stop                  Stop the SM server operation
#--show                  Display the current SM configuration 

#echo "sm {start|stop|show}"
/usr/local/bin/sudo -u pcube /SM/pcube/sm/server/bin/p3sm --start
