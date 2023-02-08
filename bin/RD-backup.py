#!/usr//bin/python3

import subprocess, os,sys

dirs_from=['/home/msgc/RD?/']
dirs_to=['/nadb/nadb41/msgc','/nadb/nadb57/msgc']

for dir_from in dirs_from:
    for dir_to in dirs_to:
        #bup_cmd=['rsync -avz',dir_from,dir_to]
        bup_cmd='rsync -avz '+dir_from+" "+dir_to
        print(bup_cmd)
        subprocess.run(bup_cmd,shell="yes")
    

