#!/usr/bin/env python3

import os
import argparse
import json
import subprocess
from datetime import datetime

RDSW = os.environ["RDSW"]
DEFAULT_CONFIG_PATH = os.path.join(RDSW, "config", "RD-anaconfig.json")
DAQ_EXE = os.path.join(RDSW, "bin", "RD-daq")

def arg_parser():
    parser = argparse.ArgumentParser(description="Run RD-DAQ")
    parser.add_argument("--config", type=str, help="Path to configuration file", default=DEFAULT_CONFIG_PATH)
    parser.add_argument("-v", "--voltage", type=float, help="Threshold voltage (V)", default=0.1)
    args = parser.parse_args()
    return args

def load_config(config_path):
    with open(config_path, 'r') as f:
        config = json.load(f)
    return config


def get_dir_name(base_name):
    counter = 1
    while True:
        dir_name = f"{base_name}_{counter}"
        if not os.path.exists(dir_name):
            return dir_name
        counter += 1

def make_run_dir():
    dir_name = datetime.now().strftime("%Y%m%d")
    if os.path.exists(dir_name):
        dir_name = get_dir_name(dir_name)
    os.makedirs(dir_name)
    print(f"Created directory: {dir_name}")
    return dir_name

def run_command(cmd):
    print("Running command:", " ".join(cmd))
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if result.returncode != 0:
        print("Error running command:", result.stderr)
    else:
        print("Command output:", result.stdout)

def run_daq(th_voltage):
    subentries     = 1000     # events
    sampling_num   = 1024     # clocks
    sampling_rate  = 10000000 # Hz
    dynamic_range  = 1        # 0: 25V, 1: 2.5V
    trigger_source = 0        # ch0
    trigger_type   = 0        # rising edge

    dir_name = make_run_dir()

    file_name = f"{dir_name}/RD"

    cmd = [
        DAQ_EXE,
        file_name,
        str(subentries),
        str(sampling_rate),
        str(sampling_num),
        str(dynamic_range),
        str(th_voltage),
        str(th_voltage),
        str(trigger_source),
        str(trigger_type),
	"comment"
    ]
    run_command(cmd)

def main():
    print("### runRD-daq.py ###")
    args = arg_parser()
    config = load_config(args.config)
    run_daq(args.voltage)

if __name__ == "__main__":
    main()
