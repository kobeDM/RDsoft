#!/usr//bin/python3

import subprocess, os,sys
#import argparse
from subprocess import PIPE
import datetime
import time
import glob
import getpass
import argparse

def parser():
    argparser=argparse.ArgumentParser()
    argparser.add_argument("Vth",type=float,nargs='?',const=None,help='[threhold (V)]')
    opts=argparser.parse_args()
    return(opts)

print("###runRD-daq.py###")
print("\tinput possword for sudo command.")
passwd=(getpass.getpass()+'\n').encode()

sub_dir = datetime.date.today().strftime('%Y%m%d')

args = parser()
if(args.Vth!=None):
    trigger_level=args.Vth
else:
    trigger_level=0.05 #V        
    

backup_cmd=HOME+'RDsoft/RD-backup.py'
    
def runDAQ():
    HOME=os.environ['HOME']+'/'
    daq_cmd=HOME+'adalm/adalm_daq/bin/daq'
    data_dir = './data/' # with the last slash
    copy_script = HOME+'bin/radon_backup.sh'
    #daq params
    fileheader="sub"
    entries=1000
    frequency=10000000
    sample_num=1024
    dynamic_range=1 #0=25V 1=2.5V
    #    trigger_level=0.05 #V
    trigger_source=0 #0=ch1 1=ch2
    trigger_type=0 #0=rise 1=fall
    mk_subrun(data_dir+sub_dir)
    file_name=data_dir+sub_dir+"/sub"
    #for adalm
    #daq usage ./daq [outfileheader] [sub_entries] [sampling_rate(Hz)] [sampling_number] [dynamic range 0(+/-25V) or 1(+/-2.5V)] [ch1 Vth(V)] [ch2 Vth(V)] [trigger source 0(ch1) or 1(ch2) or 2(or)] [Trig Edge RISE=0 or FALL=1]

    cmd = ['sudo','-S', daq_cmd
    ,file_name
    ,str(entries)
    ,str(frequency)
    ,str(sample_num)
    ,str(dynamic_range)
    ,str(trigger_level)
    ,str(trigger_level)
    ,str(trigger_source)
    ,str(trigger_type)
    ]
#    print_cmd(cmd)
    subprocess.run(cmd,input=passwd)

def print_cmd(cmd):
    for c in cmd:
        print(c)
    print()


def mk_subrun(dirname):
    if  os.path.exists(dirname):
        print(dirname+" exists.")
        files = glob.glob(dirname+'*')
        dirname_bu=dirname+"_bu"+format(len(files))
        print("backup "+dirname+" to "+dirname_bu)
        subprocess.run(['mv', dirname, dirname_bu ])
    print("making directory "+dirname)
    subprocess.run(['mkdir', '-p', dirname ])

def auto_run():
    subprocess.Popen(backup_cmd)
    while(True):
        runDAQ()
        time.sleep(1)


if __name__ == '__main__':
    auto_run()
