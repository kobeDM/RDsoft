#!/usr/bin/env python3
import os
from pathlib import Path
import time
import json
import argparse
import shutil
import subprocess
import tempfile
from glob import glob
from datetime import datetime
from influxdb import InfluxDBClient

from run_rd_ana import run_rd_ana

RDSW = os.environ["RDSW"]
DEFAULT_CONFIG = RDSW + "/config/RD-monconfig.json"
ANA_CONFIG = RDSW + "/config/RD-anaconfig.json"
MONITOR_DIR = "rnmon"


def arg_parser():
    parser = argparse.ArgumentParser(description='Run RD-mon with specified configuration.')
    parser.add_argument('--config', type=str, default=DEFAULT_CONFIG, help='Path to the configuration file (default: %(default)s)')
    return parser.parse_args()


def load_config(config_path):
    with open(config_path, 'r') as f:
        config = json.load(f)
    return config


def get_rate_file_paths(ana_dir):
    rate_files = glob(os.path.join(ana_dir, MONITOR_DIR, "*/*.dat"))
    if not rate_files:
        print(f"  Error: No .dat files found in {os.path.join(ana_dir, MONITOR_DIR)}")
        exit(1)
    return rate_files


def get_rate_data_list(rate_file_paths, detector_config):
    rate_file_data = []
    rate_data_list = []
    for file_path in rate_file_paths:            
        with open(file_path, 'r') as f:
            lines = f.readlines()
            date_srt, rate_str, rate_err_str = lines[0].split()
            isotope = os.path.basename(file_path).split('.')[0].split('_')[-1]
            if isotope == "po214":
                isotope = detector_config.get("rate_Po214", "Po214")
            if isotope == "po218":
                isotope = detector_config.get("rate_Po218", "Po218")
            if isotope == "po212":
                isotope = detector_config.get("rate_Po212", "Po212")
            rate_file_data.append((date_srt, rate_str, rate_err_str, isotope))
    return rate_file_data


def check_influxdb_connection(client):
    try:
        client.ping()
        print("Successfully connected to InfluxDB.")
    except Exception as e:
        print(f"Error connecting to InfluxDB: {e}")
        exit(1)


def make_write_points(detector, measurement, data_list):
    data_points = []
    for date_str, rate_str, rate_err_str, isotope in data_list:
        dt = datetime.strptime(date_str, "%Y/%m/%d/%H:%M:%S")
        np_ut = int(dt.timestamp() * 1e9)  

        data_point = {
            "measurement": measurement,
            "tags": {
                "detector": detector,
                "isotope": isotope
            },
            "time": np_ut,
            "fields": {
                "value": float(rate_str),
                "error": float(rate_err_str)
            }
        }
        data_points.append(data_point)

    return data_points


def write_to_influxdb(client, data_points, batch_size=1000):
    for i in range(0, len(data_points), batch_size):
        batch = data_points[i:i + batch_size]
        client.write_points(batch)


def copy_result_plots(ana_dir, grafana_config, detector_name=""):
    if not grafana_config.get("activate", False):
        return

    host = grafana_config.get("host", "")
    hostuser = grafana_config.get("hostuser", "")
    img_dir = grafana_config.get("img_dir", "")
    if not host or not hostuser or not img_dir:
        print("Error: grafana configuration requires host, hostuser, and img_dir.")
        exit(1)

    # get latest run name
    dirs = [
        d.name for d in Path(ana_dir).iterdir()
        if d.is_dir() and '20' in d.name
    ]
    if not dirs:
        print(f"Warning: No run directories found in {ana_dir}")
        return

    target_run = max(dirs)
    plot_files = glob(os.path.join(ana_dir, target_run, "*.png"))
    if not plot_files:
        print(f"Warning: No plot files found in {os.path.join(ana_dir, target_run)}")
        return

    remote_target = f"{hostuser}@{host}:{img_dir.rstrip('/')}/"

    # Stage renamed files locally, then rsync to remote host.
    with tempfile.TemporaryDirectory(prefix="rdmon_plots_") as tmp_dir:
        for plot_file in plot_files:
            dest_file = os.path.join(tmp_dir, f"{os.path.basename(plot_file).split('.')[0]}_{detector_name}.png")
            shutil.copy(plot_file, dest_file)

        rsync_cmd = [
            "rsync",
            "-rv",
            f"{tmp_dir}/",
            remote_target,
        ]

        try:
            result = subprocess.run(rsync_cmd, check=True, capture_output=True, text=True)
            if result.stdout.strip():
                print(result.stdout.strip())
            print(f"Synced plots to {remote_target}")
        except FileNotFoundError:
            print("Error: rsync command not found.")
            exit(1)
        except subprocess.CalledProcessError as e:
            print(f"Error: rsync failed: {e.stderr.strip()}")
            exit(1)


def main():
    args = arg_parser()
    config_path = args.config
    config = load_config(config_path)
    print(f"Loaded configuration from {config_path}")

    influx_config = config.get("influxdb", {})
    host = influx_config.get("host", "localhost")
    port = influx_config.get("port", 8086)
    user = influx_config.get("user", "root")
    passwd = influx_config.get("passwd", "root")
    database = influx_config.get("database", "none")
    batch_size = influx_config.get("batch_size", 1000)

    client = InfluxDBClient(host=host, port=port, username=user, password=passwd, database=database)
    print(f"Connected to InfluxDB at {host}:{port}, database: {database}")
    check_influxdb_connection(client)


    detector_config = config.get("detectors", {})

    # monitoring loop
    while True:
        for detector_id in detector_config:
            if not detector_config[detector_id].get("activate", False):
                continue

            # run analyzer
            ana_dir = detector_config[detector_id].get("ana_dir", "")
            os.chdir(ana_dir)
            run_rd_ana(ANA_CONFIG)
            copy_result_plots(ana_dir, config.get("grafana", {}), detector_config[detector_id].get("detector", ""))

            # send data to InfluxDB
            rate_file_paths = get_rate_file_paths(ana_dir)
            rate_data_list = get_rate_data_list(rate_file_paths, detector_config[detector_id])
            data_points = make_write_points(detector_config[detector_id].get("detector", ""), detector_config[detector_id].get("measurement", ""), rate_data_list)
            write_to_influxdb(client, data_points, batch_size)
            print(f"Written {len(data_points)} data points for detector {detector_id} to InfluxDB.")
        
        print(f"Sleeping for {config.get('interval', 300)} seconds...")
        time.sleep(config.get("interval", 300))

if __name__ == "__main__":
    main()