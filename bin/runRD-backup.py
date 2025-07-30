#!/usr/bin/env python3

import os
import subprocess
import time
import json

RDSW = os.environ['RDSW']
cmd  = RDSW + '/' + "bin/RD-backup.py"

with open(RDSW + '/config/RD-bupconfig.json', 'r') as json_open:
    json_load = json.load(json_open)

interval = json_load['interval']

while True:
    subprocess.run(cmd)
    time.sleep(60)
