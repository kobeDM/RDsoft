#!/usr//bin/env python3

import os
import sys
import subprocess
import json

RDSW = os.environ['RDSW']

with open(RDSW + '/config/RD-bupconfig.json', 'r') as json_open:
    json_load = json.load(json_open)

dirs_from = json_load['dirs_from']
dirs_to = json_load['dirs_to']

print(dirs_from)
print(dirs_to)
