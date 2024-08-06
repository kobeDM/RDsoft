#!/usr/bin/env python3

import os
import sys
import subprocess
import argparse
import glob
import re

RDSW = os.environ['RDSW']
ANACONFIG = RDSW + '/config/RD-anaconfig.json'

MKCONFIG = RDSW + '/bin/RD-mkconfig.py'
RNRATE = RDSW + '/bin/RD_rnrate'
CHAIN = RDSW + '/root_macros/chain.cxx'

def parse_args():
    parser = argparse.ArgumentParser(description="Combine directories")
    parser.add_argument("dirs", nargs="+", help="Directroies to combine")
    parser.add_argument("-c", "--config", help="Config file to use")
    return parser.parse_args()

def make_directory(dir_name):
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)
        print(f"Directory {dir_name} created.")
    else:
        print(f"Directory {dir_name} already exists.")

def get_latest_dir_id(dirs):
    latest_dir_id = 0
    for dir in dirs:
        dir_id = int(dir.split('_')[-1])
        if dir_id > latest_dir_id:
            latest_dir_id = dir_id
    return latest_dir_id

def execute(cmd, verbose=True):
    if verbose:
        print("Executing:", cmd)
    ret_code = subprocess.run(cmd, shell=True).returncode
    if ret_code != 0:
        print("Error:", cmd, "failed.")
        sys.exit(1)

def check_detector_id(path):
    if 'RD' not in path:
        print('Error: Current directory is not a RD directory.')
        sys.exit(1)
    detector_id=path.split('RD')[-1].split('/')[0]
    detector='RD'+detector_id
    print('Detector:', detector)
    return detector_id

def main():
    print("### combine.py start ###")
    args = parse_args()
    if len(args.dirs) < 2:
        print("Error: Please provide at least 2 directories to combine.")
        sys.exit(1)

    # check directory exist
    for dir in args.dirs:
        if not os.path.exists(dir):
            print(f"Error: {dir} does not exist.")
            sys.exit(1)
    for dir in args.dirs:
        print('dir name:', dir)

    # make output directory
    combine_dirs = glob.glob('combine_*')
    print('combine_dirs:', combine_dirs)
    if len(combine_dirs) == 0:
        new_dir_name = 'combine_0'
    else:
        latest_combine_dir_id = get_latest_dir_id(combine_dirs)
        new_dir_name = 'combine_' + str(latest_combine_dir_id + 1)
    make_directory(new_dir_name)
    make_directory(new_dir_name + '/rootfile')
    
    # fetch config files
    config_file = new_dir_name + '/RD-anaconfig.json'
    if(args.config):
        config_file = args.config
    elif (os.path.isfile(config_file)):
        print('Using existing config file.')
    else:
        init_dir = sorted(args.dirs)[0]
        daq_config_in = '../data/' + init_dir + '/RD.cnf'
        cmd = MKCONFIG + " " + daq_config_in + " " + ANACONFIG + " " + config_file
        execute(cmd)

    # copy root files
        for dir in args.dirs:
            files = glob.glob(dir + '/rootfile/RD_*.root')
            files = [f for f in files if re.match(r"RD_\d+\.root$", f.split('/')[-1])]
            for file_from in files:
                file_to = new_dir_name + '/rootfile/RD_' + file_from.split('/')[-3] + '_' + file_from.split('_')[-1]
                cmd = 'cp '+file_from+' '+file_to
                execute(cmd, verbose=False)

    # make chain list
    chain_list = new_dir_name + '/rootfile/chainList.txt'
    with open(chain_list, 'w') as chain_list_open:
        root_files = glob.glob(new_dir_name + '/rootfile/RD_*.root')
        for root_file in sorted(root_files):
            chain_list_open.write(root_file + '\n')

    # chain root files
    chain_file = new_dir_name + '/rootfile/RD_chain.root'
    cmd = 'root -b -q -l \'' + CHAIN + '(\"' + chain_list + '\", \"' + chain_file + '\")\''
    execute(cmd)
    
    # make plots
    detector_id = check_detector_id(os.getcwd())
    cmd = RNRATE + ' ' + chain_file + ' ' + config_file + ' ' + detector_id
    execute(cmd)

    print("### combine.py end ###")

if __name__ == '__main__':
    main()