#!/usr/bin/python3

import subprocess, os,sys
import argparse
from subprocess import PIPE

def parser():
    argparser=argparse.ArgumentParser()
    argparser.add_argument("subdir",type=str,nargs='?',const=None,help='[subdirectory name]')
    argparser.add_argument("fromf",type=int,nargs='?',const=None,help='[file from]')
    argparser.add_argument("tof",type=int,nargs='?',const=None,help='[file to]')
    argparser.add_argument("config",type=str,nargs='?',const=None,help='[config file name]')
    argparser.add_argument("-d","--display", dest='display',help='display only',action="store_true")
    argparser.add_argument("-b","--batch", help="batch mode",dest='batch',action="store_true")
    argparser.add_argument("-c","--chain", help="from chain (not process the raw2root) ",dest='chain',action="store_true")
    argparser.add_argument("-v","--verbose", help="verbose",dest='verbose',action="store_true")
    opts=argparser.parse_args()
    return(opts)


HOME=os.environ['HOME']+'/'
#CONFIG_DEFAULT=HOME+'RD2/analyzer/json/config.json'
CONFIG_DEFAULT=HOME+'RDsoft/analyzer/json/RD-anaconfig.json'
analyzerbin='RDsoft/bin/'
datadir='data/'
rootdir='rootfile/'
analdir='anal/'
file_from=0
file_to=0
display=0
batch_mode=0
chain_mode=0
verbose=0

#ad2_dat2root=HOME+analyzerbin+'ad2_dat2root '
#ad2_vis=HOME+analyzerbin+'ad2_rnrate '
adalm_dat2root=HOME+analyzerbin+'adalm_dat2root '
adalm_vis=HOME+analyzerbin+'adalm_rnrate '
chainmacro=HOME+analyzerbin+'chain.cxx'

print('usage: run_RD2anal.py [-d, --display] [subdir] [from] [to] [configfile]')

#args = sys.argv
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
    cmd='ls -ltr '+ datadir +' | tail -1' 
    if(verbose):
        print("cmd:",cmd)   
    proc=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
    if(verbose):
        print(proc.stdout)
        print(proc.stdout.decode().encode('utf-8'))
    #    proc=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE,text=True)
    subdir=proc.stdout.decode().encode('utf-8').split(' ')[len(proc.stdout.decode().encode('utf-8').split(' '))-1]
    if(verbose):
        print("subdir:",subdir)   
    subdir=subdir.replace('/', '').replace('\n','')+'/'
    if(verbose):
        print("subdir:",subdir)   
    #    subdir=subdir.replace('/', '').replace('\n','')

if(args.fromf!=None):
    file_from=args.fromf

if(args.tof!=None):
    file_to=args.tof
else:
    cmd='ls -ltr '+ datadir+subdir +'*.dat | tail -1'
    if(verbose):
        print("cmd:",cmd)
    proc=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE,text=True)
    file_to=(int)((proc.stdout.split(' ')[len(proc.stdout.split(' '))-1]).replace('sub', '').split('.')[-3].split('/')[2])



subdir=subdir.replace('/', '').replace('\n','')+'/'
file_num=file_to-file_from+1

dirs=[datadir,rootdir,analdir]
for dir in dirs:
    cmd='ls -d '+dir
    ret=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
    if(ret.returncode != 0):
        subprocess.run('mkdir '+dir,shell=True); 
    outdir=dir+subdir
    cmd='ls -d '+outdir
    ret=subprocess.run(cmd,shell=True, stdout=PIPE, stderr=PIPE)
    if(ret.returncode != 0):
        subprocess.run('mkdir '+outdir,shell=True); 
    if(verbose):    
        print('output directory:'+outdir)

config_target=analdir+subdir+"config.json"

if(args.config):
    configfile=args.config
elif (os.path.isfile(config_target)):
     configfile=config_target
else: 
    cmd='cp '+CONFIG_DEFAULT+' '+config_target
    subprocess.run(cmd,shell=True)
    configfile=config_target

if(verbose):
    print("subdir:",subdir)
    print("config file:",configfile)
    print('input file:',file_from,'-',file_to,'(',file_num,' files.)')


#if(display!=1 and not chain_mode):
if(not display and not chain_mode):

    for file_id in range(file_from,file_to+1):
        infile='sub'+format(file_id).zfill(4)+'.0000'
        infile_full=datadir+'/'+subdir+infile+'.dat '
        cmd=ad2_dat2root+infile_full+configfile
        if (verbose):
            print('executing '+cmd)
        subprocess.run(cmd,shell=True) 
        rootfile=infile+'.root'
        visfile=infile+'_vis.root'
        cmd='mv ' +rootfile+' '+rootdir+subdir
        if (verbose):
            print('executing '+cmd)
        subprocess.run(cmd,shell=True)
        #ad2_vis batch mode
        cmd=ad2_vis+' -b '+rootdir+subdir+'/'+rootfile+' '+configfile
        if (verbose):
            print('executing '+cmd)
        subprocess.run(cmd,shell=True)
#        cmd='mv ' +visfile+' '+analdir+subdir
#        subprocess.run(cmd,shell=True)

    
if (file_num>1):
    #rootfiles
    infiles='sub_chain_'+format(file_from)+'-'+format(file_to)
    rootfiles=infiles+'.root'
    visfiles=infiles+'_vis.root'
    if(not display):
        #chain
        cmd='cd '+rootdir+subdir+'; root -q \''+chainmacro+'('+format(file_from)+','+format(file_to)+')\''
        if (verbose):
            print('executing '+cmd)      
        subprocess.run(cmd,shell=True)
        cmd='pwd'
        if (verbose):
            print('executing '+cmd)            
        subprocess.run(cmd,shell=True)
        cmd=ad2_vis+' -b '+rootdir+subdir+'/'+rootfiles+' '+configfile
        if (verbose):
            print('executing '+cmd)
        subprocess.run(cmd,shell=True)
#        cmd='mv ' +visfiles+' '+analdir+subdir
#        subprocess.run(cmd,shell=True)
        

#display
if file_num>1:
    if batch_mode:
        cmd=ad2_vis+' -b '+rootdir+subdir+'/'+rootfiles+' '+configfile

    else:
        cmd=ad2_vis+' '+rootdir+subdir+'/'+rootfiles+' '+configfile

#    subprocess.run(cmd,shell=True)
else:
    if batch_mode:
        cmd=ad2_vis+' -b '+rootdir+subdir+'/'+rootfile+' '+configfile

    else:
        cmd=ad2_vis+' '+rootdir+subdir+'/'+rootfile+' '+configfile

        
if(verbose):
    print('executing '+cmd)
#if(not chain_mode):
#if(verbose):
print("cmd: ",cmd)
subprocess.run(cmd,shell=True)
