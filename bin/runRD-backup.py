#!/usr/bin/python3

import subprocess,os
import time

RDSW=os.environ['RDSW']+'/'
cmd=RDSW+"bin/RD-backup.py"
interval=60
while 1:
    #    print(exe)
    subprocess.run(cmd)
    time.sleep(60)
