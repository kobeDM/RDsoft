#!/usr/bin/python3

import subprocess
import time
cmd="/home/msgc/RDsoft/bin/RD-backup.py"
interval=60
while 1:
    #    print(exe)
    subprocess.run(cmd)
    time.sleep(60)
