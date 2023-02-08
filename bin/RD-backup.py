#!/usr//bin/python3

import subprocess, os,sys


#rsync -avz -e "ssh -p 22 -i ~/.ssh/id_rsa" /home/pi/adalm_data pi@03cDAQ.local:/mnt/lbghdd/radon_backup

bup_cmd=['rsync -avz /home/msgc/RD* /nadb/nadb41/msgc','rsync -avz /home/msgc/RD* /nadb/nadb57/msgc']
#print(bup_cmd)
subprocess.run(bup_cmd,shell="yes")
