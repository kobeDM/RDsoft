#!/usr/bin/python3

import subprocess, os,sys
import argparse
import json
from subprocess import PIPE

def parser():
    argparser=argparse.ArgumentParser()
    argparser.add_argument("daq_config_in",type=str,nargs='?',const=None,help='[daq config file name]',default="RD.cnf")   
    argparser.add_argument("json_config_in",type=str,nargs='?',const=None,help='[json config file name]',default=CONFIG_DEFAULT)   
    argparser.add_argument("config_out",type=str,nargs='?',const=None,help='[outpu config file]',default="RD-anaconfig.json")
    argparser.add_argument("-v","--verbose", help="verbose",dest='verbose',action="store_true")
    opts=argparser.parse_args()
    return(opts)

RDSW=os.environ['RDSW']+'/'
CONFIG_DEFAULT=RDSW+'config/RD-anaconfig.json'

keys_DAQ=["SAMPLING RATE","SAMPLING NUMBER","DYNAMIC RANGE","TRIGGER THRESHOLD CH1","RUN START","RUN END"] # find in the DAQ config
json_ana_out={}
json_view_out={}
json_daq_out={}
json_out={}
json_out["ana"]=json_ana_out
json_out["view"]=json_view_out
json_out["DAQ"]=json_daq_out
verbose=0
args = parser()
if args.verbose:
    #print("verbose")
    verbose=1


print("##RD-mkconfig.py##")
print("prepare config file")


if(args.json_config_in):
    json_config_in=args.json_config_in
if(args.daq_config_in):
    daq_config_in=args.daq_config_in
if(args.config_out):
    config_out=args.config_out
if (verbose):
    print("JSON config file: "+json_config_in)
json_open =open(CONFIG_DEFAULT, 'r')
json_load = json.load(json_open)

#for dic in ["ana","view"]:
#    print (dic)
#    for key in json_load[dic]:
#        print(key)
#        json_ana_out[key]=json_load[dic][key]

for key in json_load["ana"]:
    json_ana_out[key]=json_load["ana"][key]
        
for key in json_load["view"]:
    json_view_out[key]=json_load["view"][key]
        
print("DAQ config file: "+daq_config_in)
with open(daq_config_in,"r") as f:
    for s_line in f:
        for key in keys_DAQ:
            if key in s_line:
                if key=="RUN START" or  key=="RUN END":
                    value=s_line.split(":")[-1].split(" ")[2].replace("\n","")
                else:
                    value=s_line.split(":")[-1].split(" ")[1].replace("\n","")
                #print(s_line)
                #print(key+value)
                if key=="DYNAMIC RANGE":
                    #print("hit"+value)
                    if value=="1":
                        value=2.5
                    elif value==0:
                        value=25
                json_daq_out[key]=value


print("output config file: "+config_out)

#print(json_out)                
json_file = open(config_out, 'w')
json.dump(json_out, json_file,indent=2)
