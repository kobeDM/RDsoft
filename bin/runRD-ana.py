#!/usr/bin/python3

import subprocess, os,sys
import argparse
import datetime
from subprocess import PIPE


def parser():
    argparser=argparse.ArgumentParser()
    argparser.add_argument("subdir",type=str,nargs='?',const=None,help='[subdirectory name]')
    argparser.add_argument("fromf",type=int,nargs='?',const=None,help='[file from]')
    argparser.add_argument("tof",type=int,nargs='?',const=None,help='[file to]')
    argparser.add_argument("config",type=str,nargs='?',const=None,help='[config file name]')
    argparser.add_argument("-d","--display", dest='display',help='display only',action="store_true")
    argparser.add_argument("-b","--batch", help="batch mode",dest='batch',action="store_true")
    argparser.add_argument("-c","--chain", help="from chain (skip dat2root) ",dest='chain',action="store_true")
    argparser.add_argument("-v","--verbose", help="verbose",dest='verbose',action="store_true")
    opts=argparser.parse_args()
    return(opts)


HOME=os.environ['HOME']+'/'
CONFIG_SOURCE=HOME+'RDsoft/config/RD-anaconfig.json'
analyzerbin='RDsoft/bin'
rootmacrodir='RDsoft/root_macros'
datadir='../data'
file_from=0
file_to=0
display=0
batch_mode=0
chain_mode=0
verbose=0

RD_mkconfig=HOME+analyzerbin+'/RD-mkconfig.py '
RD_dat2root=HOME+analyzerbin+'/RD_dat2root '
RD_vis=HOME+analyzerbin+'/RD_rnrate '
chainmacro=HOME+rootmacrodir+'/chain.cxx'

#print('usage: run-RDana.py [-d, --display] [dir] [from] [to] [configfile]')

args = parser()
if args.display:
    #switch for diaplay only    
    print("display only")
    display=1

if args.batch:
    #switch for batch mode
    print("batch mode")
    batch_mode=1

if args.chain:
    #switch for from chain
    print("from chain")
    chain_mode=1

if args.verbose:
    #verbose
    print("verbose")
    verbose=1

if(args.subdir):
    subdir=args.subdir
else:
    cmd='ls -ltrd '+ datadir +'/*/ | grep 20 | tail -1' 
    if(verbose):
        print("cmd:",cmd)   
    proc=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
    subdir=proc.stdout.decode().split(' ')[len(proc.stdout.decode().split(' '))-1]
    if(verbose):
        print("subdir:",subdir)   
    #subdir=subdir.replace('/', '').replace('\n','')+'/'
    subdir=subdir.split('/')[len(proc.stdout.decode().split('/'))-2]
subdir=subdir.replace('\n','')+'/'
if(verbose):
    print("subdir:",subdir)   

if verbose:
    RD_dat2root=RD_dat2root+" -v "
    RD_vis=RD_vis+" -v "

#directories
rootdir=subdir+'rootfile'
mondir='rnmon/'+format(datetime.date.today().year)
dirs=[subdir,rootdir,mondir]
for dir in dirs:
    cmd='ls -d '+dir
    ret=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
    if(ret.returncode != 0):
        subprocess.run('mkdir -p '+dir,shell=True); 


if(args.fromf!=None):
    file_from=args.fromf
print("file_from:"+format(file_from))
if(args.tof!=None):
    file_to=args.tof
else:
    cmd='ls -ltr '+ datadir+"/"+subdir +'*.dat | tail -1'
    if(verbose):
        print("cmd:",cmd)
    proc=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
    file_to=(int)((proc.stdout.decode().split(' ')[len(proc.stdout.decode().split(' '))-1]).split('/')[-1].replace('RD_', '').replace('.dat', ''))

print("file_to:"+format(file_to))

file_num=file_to-file_from+1

#detector check        
cmd='pwd'
if (verbose):
    print('executing '+cmd)            
proc=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
detector_id=proc.stdout.decode().split('RD')[len(proc.stdout.decode().split('RD'))-1].split('/')[0]
detector='RD'+detector_id
print("detector: "+detector)

#config files
daq_config_in=datadir+"/"+subdir+"RD.cnf"
json_config_in=CONFIG_SOURCE
config_target=subdir+"RD-anaconfig.json"

if(args.config):
    configfile=args.config
elif (os.path.isfile(config_target)):
    configfile=config_target
else:
    cmd=RD_mkconfig+" "+daq_config_in+" "+json_config_in+" "+config_target
    if(verbose):
        print("cmd:",cmd)
    subprocess.run(cmd,shell=True)
    configfile=config_target
#else:
#    print("config file make failed." )
#    exit(1)
    #cmd='cp '+CONFIG_SOURCE+' '+config_target
    #subprocess.run(cmd,shell=True)
    #configfile=config_target

if(verbose):
    #print("subdir:",subdir)
    print("config file:",configfile)
    print('input file:',file_from,'-',file_to,'(',file_num,' files.)')

#start from dat2root
if(not display and not chain_mode):
    for file_id in range(file_from,file_to+1):
        infile='RD_'+format(file_id)
        infile_full=datadir+'/'+subdir+infile+'.dat '
        if (verbose):
            print(infile_full)
        cmd=RD_dat2root+infile_full+configfile
        if (verbose):
            print('executing '+cmd)
        subprocess.run(cmd,shell=True) 
        rootfile=rootdir+"/"+infile+'.root'
        visfile=infile+'_vis.root'
        cmd='mv out.root '+rootfile
        if (verbose):
            print('executing '+cmd)
        subprocess.run(cmd,shell=True)
        #RD_vis batch mode
        cmd=RD_vis+' -b '+rootfile+' '+configfile+' '+detector_id
        if (verbose):
            print('executing '+cmd)
        subprocess.run(cmd,shell=True)

if (file_num>1):
    # more than two files
    rootfiles=rootdir+"/RD_"+format(file_from)+'-'+format(file_to)+'.root'
    visfiles=rootdir+"/RD_"+format(file_from)+'-'+format(file_to)+'_vis.root'
    if(not display):
        #chain
        cmd='cd '+rootdir+'; root -q \''+chainmacro+'('+format(file_from)+','+format(file_to)+')\''
        subprocess.run(cmd,shell=True)
    if(batch_mode):    
        cmd=RD_vis+' -b '+rootfiles+' '+configfile+' '+detector_id
    else:
        cmd=RD_vis+' '+rootfiles+' '+configfile+' '+detector_id
    
else:
#one file only  
    infile='RD_'+format(file_from)
    rootdir=subdir+'rootfile'
    rootfile=rootdir+"/"+infile+'.root'  
     
    #    if(not display):
    if(batch_mode):    
    #batch_mode:
        cmd=RD_vis+' -b '+rootfile+' '+configfile+' '+detector_id

    else:
        cmd=RD_vis+' '+rootfile+' '+configfile+' '+detector_id

        
if(verbose):
    print('executing '+cmd)
    
subprocess.run(cmd,shell=True)

#cmd='mv rate.dat '+subdir
#subprocess.run(cmd,shell=True)
