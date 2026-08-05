#!/usr/bin/env python3

import os
import argparse
from pathlib import Path
import subprocess
import re
import shutil
import json

RDSW = os.environ["RDSW"]
DEFAULT_CONFIG = os.path.join(RDSW, "config", "RD-anaconfig.json")

DAT2ROOT = os.path.join(RDSW, "bin", "Dat2Root")
CALC_RNRATE   = os.path.join(RDSW, "bin", "CalcRnRate")

CHAIN_ROOT_FILE = "RD_chain.root"
ROOT_FILE_DIR = "rootfile"
MONITOR_DIR = "rnmon"


def arg_parser():
    parser = argparse.ArgumentParser(description="Run RD analysis")
    parser.add_argument("period", nargs="?", help="Period name (e.g. 20260101)", default=None)
    parser.add_argument("-c", "--config", help="Path to configuration file", default=DEFAULT_CONFIG)
    parser.add_argument("-v", "--verbose", action="store_true", help="Enable verbose output")
    args = parser.parse_args()
    return args


def make_analysis_dir(period=None):
    # check data directory exists
    current_dir = os.getcwd()
    data_dir = current_dir.replace("ana", "data")
    if not os.path.exists(data_dir):
        print(f"  Error: Data directory '{data_dir}' does not exist.")
        exit(1)
    

    # get target run path
    print("Searching for run directories in data directory...")
    target_run = None
    if period is None: # latest run
        dirs = [
            d.name for d in Path(data_dir).iterdir()
            if d.is_dir() and '20' in d.name
        ]
        target_run = max(dirs)
        print(f"  Latest run path: {target_run}")
    else: # specified run
        target_run = period
        print(f"  Using specified period: {target_run}")

    # make analysis directory
    analysis_dir = os.path.join(current_dir, target_run)
    if not os.path.exists(analysis_dir):
        os.makedirs(analysis_dir)
        print(f"  Created analysis directory: {analysis_dir}")
    else:
        print(f"  Analysis directory already exists: {analysis_dir}")

    if not os.path.exists(os.path.join(analysis_dir, ROOT_FILE_DIR)):
        os.makedirs(os.path.join(analysis_dir, ROOT_FILE_DIR))
        print(f"  Created rootfile directory: {os.path.join(analysis_dir, ROOT_FILE_DIR)}")
    else:
        print(f"  Rootfile directory already exists: {os.path.join(analysis_dir, ROOT_FILE_DIR)}")
    print()

    run_dir = os.path.join(data_dir, target_run)

    # return analysis_dir (/path/to/ana/YYYYMMDD) and run_dir (/path/to/data/YYYYMMDD)
    return analysis_dir, run_dir


def run_command(cmd, verbose=False):
    if verbose:
        print(f"Running command: {cmd}")
    subprocess.run(cmd, shell=True)



def get_data_file_list(data_dir_path):
    files = [
        str(f) for f in Path(data_dir_path).glob("RD_*.dat")
        if re.fullmatch(r"^RD_\d+\.dat$", f.name)
    ]
    return files


def make_root_file_path(input_file, analysis_dir):
    input_filename = os.path.basename(input_file)
    output_filename = input_filename.replace(".dat", ".root")
    output_file_path = os.path.join(analysis_dir, ROOT_FILE_DIR, output_filename)
    return output_file_path


def make_config_files(config_path, analysis_dir, run_dir):
    if not os.path.exists(config_path):
        print(f"Error: Config file '{config_path}' does not exist.")
        exit(1)
    
    # copy config file to analysis directory if not exists
    target_config_path = os.path.join(analysis_dir, "RD-anaconfig.json")
    if not os.path.exists(target_config_path):
        print(f"Copying config file to analysis directory: {target_config_path}")
        shutil.copy(config_path, target_config_path)
    else:
        print(f"Config file already exists in analysis directory: {target_config_path}")

    # parse DAQ config file and update default config
    daq_config_path = os.path.join(run_dir, "RD.cnf")
    if not os.path.exists(daq_config_path):
        print(f"Error: DAQ config file '{daq_config_path}' does not exist.")
        exit(1)
    with open(daq_config_path, "r") as f:
        lines = f.readlines()
    
    config_dict = {}
    for line in lines:
        if ":" not in line:
            continue
        key, value = line.split(":", 1)
        config_dict[key.strip()] = value.strip()

    with open(target_config_path, "r") as f:
        default_config = json.load(f)
    default_config["DAQ"]["SAMPLING RATE"]         = config_dict["SAMPLING RATE"].split()[0]
    default_config["DAQ"]["SAMPLING NUMBER"]       = config_dict["SAMPLING NUMBER"].split()[0]
    dynamic_range_type = int(config_dict["DYNAMIC RANGE"].split()[0])
    if dynamic_range_type == 0:
        default_config["DAQ"]["DYNAMIC RANGE"] = 25
    else:
        default_config["DAQ"]["DYNAMIC RANGE"] = 2.5
    default_config["DAQ"]["TRIGGER THRESHOLD CH1"] = config_dict["TRIGGER THRESHOLD CH1"].split()[0]
    default_config["DAQ"]["RUN START"]             = config_dict["RUN START"].split()[1]

    # update config file in analysis directory
    with open(target_config_path, "w") as f:
        json.dump(default_config, f, indent=4)

    return target_config_path


def make_monitor_dir(run_dir):
    current_dir = os.getcwd()
    monitor_dir = os.path.join(current_dir, MONITOR_DIR)
    if not os.path.exists(monitor_dir):
        os.makedirs(monitor_dir)
        print(f"  Created monitor directory: {monitor_dir}")
    else:
        print(f"  Monitor directory already exists: {monitor_dir}")
    year = run_dir.split("/")[-1][:4] # get year from run_dir name (assuming format YYYYMMDD)
    monitor_subdir = os.path.join(monitor_dir, year)
    if not os.path.exists(monitor_subdir):
        os.makedirs(monitor_subdir)
        print(f"  Created monitor subdirectory: {monitor_subdir}")
    else:
        print(f"  Monitor subdirectory already exists: {monitor_subdir}")

    return monitor_subdir


def hadd_root_files(root_dir, verbose=False):
    print(f"Hadding root files in {root_dir}...")
    output_file = os.path.join(root_dir, CHAIN_ROOT_FILE)
    if os.path.exists(output_file):
        print(f"  Removing existing {CHAIN_ROOT_FILE} in {root_dir}")
        os.remove(output_file)

    root_files = list(Path(root_dir).glob("*.root"))
    if not root_files:
        print(f"  No root files found in {root_dir}")
        return

    # sort root files
    root_files = sorted(
        Path(root_dir).glob("RD_*.root"),
        key=lambda p: int(p.stem.split("_")[1])
    )

    hadd_cmd = f"hadd -f {output_file} " + " ".join(str(f) for f in root_files)
    run_command(hadd_cmd, verbose=verbose)
    print()


def run_rd_ana(config_path, verbose_flag=False, period=None):
    print("### runRD-ana.py ###")

    print(f"Using config file: {config_path}")
    print(f"Verbose mode     : {'ON' if verbose_flag else 'OFF'}")
    print()

    analysis_dir, run_dir = make_analysis_dir(period)
    input_file_list = get_data_file_list(run_dir)

    target_config_path = make_config_files(config_path, analysis_dir, run_dir)

    # Run Dat2Root
    for input_file in input_file_list:
        output_file = make_root_file_path(input_file, analysis_dir)
        dat2root_cmd = f"{DAT2ROOT} -i {input_file} -o {output_file} -c {target_config_path}"
        if verbose_flag:
            dat2root_cmd += " -v"
        run_command(dat2root_cmd, verbose=verbose_flag)
    print()

    # Hadd root files
    hadd_root_files(os.path.join(analysis_dir, ROOT_FILE_DIR), verbose=verbose_flag)

    # Run CalcRnRate
    monitor_dir = make_monitor_dir(run_dir)

    target_detector = next( # get detector name
        (part for part in Path(analysis_dir).parts if re.fullmatch(r'RD\d+', part)),
        None
    )
    if target_detector is None:
        print("Error: Could not find RD directory in analysis path.")
        exit(1)


    print(f"Running CalcRnRate for detector {target_detector}...")
    calc_rnrate_cmd = f"{CALC_RNRATE} -i {os.path.join(analysis_dir, ROOT_FILE_DIR, CHAIN_ROOT_FILE)} -o {analysis_dir} -c {target_config_path} -d {target_detector} -m {monitor_dir}"
    if verbose_flag:
        calc_rnrate_cmd += " -v"
    run_command(calc_rnrate_cmd, verbose=verbose_flag)


def main():
    args = arg_parser()
    config_path = args.config
    verbose_flag = args.verbose
    target_period = args.period

    run_rd_ana(config_path, verbose_flag, target_period)

if __name__ == "__main__":
    main()