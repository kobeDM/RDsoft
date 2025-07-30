#!/usr/bin/python3

import subprocess, os,sys
import argparse
import datetime
from subprocess import PIPE


def parser():
    argparser=argparse.ArgumentParser()
    argparser.add_argument("subdir",type=str,nargs='?',const=None,help='[subdirectory name YYYYMMDD]')
    argparser.add_argument("fromf",type=int,nargs='?',const=None,help='[file from]')
    argparser.add_argument("tof",type=int,nargs='?',const=None,help='[file to]')
    argparser.add_argument("datadir",type=str,nargs='?',const=None,help='[data directory (default: ../data)]')
    argparser.add_argument("config",type=str,nargs='?',const=None,help='[config file name (default: YYYYMMDD/RD-anaconfig.json)]') 
    argparser.add_argument("-d","--display", dest='display',help='rate analysis and display (skip dat2root and chain)',action="store_true")
    argparser.add_argument("-b","--batch", help="batch mode",dest='batch',action="store_true")
    argparser.add_argument("-c","--chain", help="from chain (skip dat2root) ",dest='chain',action="store_true")
    argparser.add_argument("-v","--verbose", help="verbose",dest='verbose',action="store_true")
    argparser.add_argument("-f","--force", help="auto fitting",dest='fit',action="store_true")
    opts=argparser.parse_args()
    return(opts)


RDSW=os.environ['RDSW']+'/'
CONFIG_SOURCE=RDSW+'config/RD-anaconfig.json'
analyzerbin='/bin'
rootmacrodir='root_macros'
datadir='../data'

file_from=0
file_to=0
display=0
batch_mode=0
chain_mode=0
verbose=0
auto_fitting=0

RD_mkconfig=RDSW+analyzerbin+'/RD-mkconfig.py '
RD_dat2root=RDSW+analyzerbin+'/RD_dat2root '
RD_vis=RDSW+analyzerbin+'/RD_rnrate '
chainmacro=RDSW+rootmacrodir+'/chain.cxx'

print('####################')
print('### run-RDana.py ###')
print('####################')

args = parser()

if args.display:
    # switch for diaplay only    
    print("- Display mode")
    display=1

if args.batch:
    # switch for batch mode
    print("- Batch mode")
    batch_mode=1

if args.chain:
    # analyze from chain file
    print("- Chain mode")
    chain_mode=1

if args.verbose:
    # verbose
    print("- Verbose mode")
    verbose=1

if args.datadir:
    # override
    datadir=args.datadir

if args.fit:
    # auto fitting
    print("- Auto fitting mode")
    auto_fitting=1


if(args.subdir):
    subdir=args.subdir
else:
    # search the latest directory
    cmd='ls -ltrd '+ datadir +'/*/ | grep 20 | tail -1' 
    if(verbose):
        print("> Executing:",cmd)   
    proc=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
    subdir=proc.stdout.decode().split(' ')[len(proc.stdout.decode().split(' '))-1]
    subdir=subdir.split('/')[len(proc.stdout.decode().split('/'))-2]
    print(subdir)

subdir=subdir.replace('\n','')+'/'
if(verbose):
    print("Directory:",subdir.split('/')[0])

if verbose:
    RD_dat2root=RD_dat2root+" -v "
    RD_vis=RD_vis+" -v "
if auto_fitting:
    RD_vis=RD_vis+" -f "

# directories
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
if(args.tof!=None):
    file_to=args.tof
else:
    cmd='ls -ltr '+ datadir+"/"+subdir +'*.dat | tail -1'
    # if(verbose):
    #     print("> Executing:",cmd)
    proc=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
    file_to=(int)((proc.stdout.decode().split(' ')[len(proc.stdout.decode().split(' '))-1]).split('/')[-1].replace('RD_', '').replace('.dat', ''))

file_num=file_to-file_from+1
if(verbose):
    print('File: RD_'+str(file_from)+'.dat - RD_'+str(file_to)+'.dat ('+str(file_num)+' files.)')

# detector check        
cmd='pwd'
proc=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
detector_id=proc.stdout.decode().split('RD')[len(proc.stdout.decode().split('RD'))-1].split('/')[0]
detector='RD'+detector_id

if (verbose):
    print("Detector:", detector)

# config files
daq_config_in=datadir+"/"+subdir+"RD.cnf"
json_config_in=CONFIG_SOURCE
config_target=subdir+"RD-anaconfig.json"

if(args.config):
    configfile=args.config
elif (os.path.isfile(config_target)):
    configfile=config_target
else:
    cmd=RD_mkconfig+" "+daq_config_in+" "+json_config_in+" "+config_target
    subprocess.run(cmd,shell=True)
    configfile=config_target

if(verbose):
    print('DAQ config file:',daq_config_in)
    print('JSON config file:',json_config_in)

print('===\n')

# start from dat2root
if(not display and not chain_mode):
    for file_id in range(file_from,file_to+1):
        infile='RD_'+format(file_id)
        infile_full=datadir+'/'+subdir+infile+'.dat '

        # RD_dat2root
        cmd=RD_dat2root+infile_full+configfile
        if (verbose):
            print('Converting '+infile_full+'to root file ...')
            print('Executing:',cmd)
        subprocess.run(cmd,shell=True) 
        rootfile=rootdir+"/"+infile+'.root'
        visfile=infile+'_vis.root'

        # rename
        cmd='mv out.root '+rootfile
        print('---')
        if (verbose):
            print('Renaming out.root to '+rootfile)
            print('Executing:',cmd)
        subprocess.run(cmd,shell=True)
        print('---')

        # RD_rnrate
        cmd=RD_vis+' -b '+rootfile+' '+configfile+' '+detector_id
        if (verbose):
            print('Drawing vis file ...')
            print('Executing:',cmd)
        subprocess.run(cmd,shell=True)
        print('===\n')

if (file_num>1):
    rootfiles=rootdir+"/RD_"+format(file_from)+'-'+format(file_to)+'.root'
    visfiles=rootdir+"/RD_"+format(file_from)+'-'+format(file_to)+'_vis.root'
    if(not display):
        cmd='cd '+rootdir+'; root -q \''+chainmacro+'('+format(file_from)+','+format(file_to)+')\''
        subprocess.run(cmd,shell=True)
    if(batch_mode):    
        cmd=RD_vis+' -b '+rootfiles+' '+configfile+' '+detector_id
    else:
        cmd=RD_vis+' '+rootfiles+' '+configfile+' '+detector_id
else:
    infile='RD_'+format(file_from)
    rootdir=subdir+'rootfile'
    rootfile=rootdir+"/"+infile+'.root'       
    if(batch_mode):    
        cmd=RD_vis+' -b '+rootfile+' '+configfile+' '+detector_id
    else:
        cmd=RD_vis+' '+rootfile+' '+configfile+' '+detector_id

        
if(verbose):
    print('executing '+cmd)
    
subprocess.run(cmd,shell=True)
