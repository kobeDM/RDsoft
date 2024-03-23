#!/usr/bin/python3

import subprocess, os,sys
import argparse
import time,datetime
import json
import numpy as np
from subprocess import PIPE
from influxdb import InfluxDBClient

def parser():
    argparser=argparse.ArgumentParser()
    argparser.add_argument("config",type=str,nargs='?',const=None,help='[config file name (default: RD-monconfig.json)]') 
    argparser.add_argument("-b","--batch", help="batch mode",dest='batch',action="store_true")
    argparser.add_argument("-v","--verbose", help="verbose",dest='verbose',action="store_true")
    argparser.add_argument("-f","--force", help="auto fitting",dest='fit',action="store_true")
    opts=argparser.parse_args()
    return(opts)

RDSW=os.environ['RDSW']+'/'
configfile='RD-monconfig.json'
CONFIG_SOURCE=RDSW+'config/RD-monconfig.json'
analyzerbin='bin/'
rootmacrodir='root_macros'
runRD_ana=RDSW+analyzerbin+'runRD-ana.py -b -f'

batch_mode=0
verbose=0
auto_fitting=0

print('### runRD-mon.py ###')
args = parser()
if args.batch:
    print(" batch mode")
    batch_mode=1    
if args.verbose:
    print(" verbose")
    verbose=1
if args.fit:
    print(" auto fitting")
    auto_fitting=1
    
if (os.path.isfile(configfile)):
    if verbose:
        print(configfile," exists.")
else:
    cmd="cp "+CONFIG_SOURCE+" ./"+configfile
    if(verbose):
        print(cmd)
    subprocess.run(cmd,shell=True)

detinfo=[]
name_list=[]
data_dir_list=[]
ana_dir_list=[]
mon_dir_list=[]
rate_file_list=[]

#read config file
print(" reading configfile ",configfile)
json_open=open(configfile,"r")
json_load=json.load(json_open)

interval=json_load['interval']

numofdet=len(json_load['detectors'])
for detID in range (numofdet):
    det=list(json_load['detectors'].items())[detID][0]
    detinfo.append(list(json_load['detectors'][det].items()))
    for item in detinfo[detID]:
        if(item[0]=="detector"):
            name_list.append(item[1])
        elif(item[0]=="data_dir"):
            data_dir_list.append(item[1])
        elif(item[0]=="ana_dir"):
            ana_dir_list.append(item[1])  
        elif(item[0]=="mon_dir"):
            mon_dir_list.append(item[1])        

# output the directories
for detID in range (numofdet):
    data_dir_list[detID]=data_dir_list[detID]
    ana_dir_list[detID]=ana_dir_list[detID]
    print("  detector",detID,end=":")
    print(" name=",name_list[detID],end=",")
    print(" data_dir=",data_dir_list[detID],end=",")
    print(" ana_dir=",ana_dir_list[detID])

# influxdb setting
client = InfluxDBClient( host     = json_load['influxdb']['host'],
                         port     = json_load['influxdb']['port'],
                         username = json_load['influxdb']['user'],
                         password = json_load['influxdb']['passwd'],
                         database = json_load['influxdb']['database'])    
    
# runRD_ana
while(True):
    for detID in range (numofdet):
        cmd='cd '+ ana_dir_list[detID]+'; '+runRD_ana
        if(verbose):
            print("cmd:",cmd)
        proc=subprocess.run(cmd,shell=True)
        cmd='rsync -dq '+ ana_dir_list[detID]+'/rnmon/*/* '+mon_dir_list[detID]
        if(verbose):
            print("cmd:",cmd)
        proc=subprocess.run(cmd,shell=True)

    print("## monitor update ##")
    for detID in range (numofdet):
        # make the rate file list
        cmd='ls '+ ana_dir_list[detID]+'/rnmon/*/* '
        if(verbose):
            print("cmd:",cmd)
        proc=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
        rate_file_list.insert(detID,(proc.stdout.decode().split('\n')))
        numofratefiles=len(rate_file_list[detID])-1
        print("number of ratefiles: ",numofratefiles)
        for ratefileID in range (numofratefiles):
            infile=rate_file_list[detID][ratefileID]
            with open(infile) as f:
                l=f.readlines()
                for lineID in range(len(l)):
                    detector=name_list[detID]
                    det=list(json_load['detectors'].items())[detID][0]
                    measurement=json_load['detectors'][det]['measurement']
                    Po212=json_load['detectors'][det]['rate_Po212']
                    Po214=json_load['detectors'][det]['rate_Po214']
                    Po218=json_load['detectors'][det]['rate_Po218']

                    rate_file_name_short=rate_file_list[detID][ratefileID].split("/")[-1]
                    evtime=l[lineID].split(" ")[0]
                    evtime_ut=datetime.datetime.strptime(evtime,"%Y/%m/%d/%H:%M:%S").timestamp()
                    np_ut=np.int64(datetime.datetime.strptime(evtime,"%Y/%m/%d/%H:%M:%S").timestamp()*1e9)
                    rate=float(l[lineID].split(" ")[1])
                    rate_error=float(l[lineID].split(" ")[2])           
                    
                    if(verbose):
                        print(detector," ",ratefileID," ",lineID," ",rate_file_name_short," ",evtime," ",np_ut," ",rate," ",rate_error)
                    if 'po214' in infile:
                        data=[{'measurement':measurement,'fields':{'value':rate,'error':rate_error},'time':np_ut,'tags':{'detector':detector,'isotope':Po214}}]
                    elif  'po218' in infile:
                        data=[{'measurement':measurement,'fields':{'value':rate,'error':rate_error},'time':np_ut,'tags':{'detector':detector,'isotope':Po218}}]
                    elif  'po212' in infile:
                        data=[{'measurement':measurement,'fields':{'value':rate,'error':rate_error},'time':np_ut,'tags':{'detector':detector,'isotope':Po212}}]
                   
                    else:
                        data=[{'measurement':measurement,'fields':{'value':rate,'error':rate_error},'time':np_ut,'tags':{'detector':detector,'isotope':'others'}}]
                    client.write_points(data)                                            

        # write to the influxDB
        det=list(json_load['detectors'].items())[detID][0]
        detector=json_load['detectors'][det]['detector']
        data=[{'measurement':'test','fields':{'value':8.0},'time':1687412683324915288,'tags':{'detector':detector,'isotope':Po218}}]
        client.write_points(data)

        
    print("sleeping for ",interval," seconds.")
    time.sleep(interval)

