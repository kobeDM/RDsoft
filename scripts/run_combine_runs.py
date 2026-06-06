#!/usr/bin/env python3
import os
import subprocess
import argparse


RDSW = os.environ["RDSW"]
ROOT_MACRO_PATH = os.path.join(RDSW, "rootmacros", "CombineRuns.cc")

RUN_LIST_FILE = "run_list.txt"

def arg_parser():
    parser = argparse.ArgumentParser(description="Combine RD analysis results from multiple runs")
    parser.add_argument("periods", nargs="+", help="List of period names to combine (e.g. 20260101 20260102)")
    parser.add_argument("-o", "--output", help="Output directory for combined results", default="combined_results")
    return parser.parse_args()


def make_output_dir(output_dir):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"  Created output directory: {output_dir}")
    else:
        print(f"  Output directory already exists: {output_dir}")


def make_run_list(periods):
    run_list = []
    current_dir = os.getcwd()
    for period in periods:
        run_dir = os.path.join(current_dir, period)
        if not os.path.exists(run_dir):
            print(f"  Warning: Run directory '{run_dir}' does not exist. Skipping.")
            continue
        run_list.append(run_dir)

    with open(RUN_LIST_FILE, "w") as f:
        for run in run_list:
            f.write(run + "\n")

    return RUN_LIST_FILE


def execute_root_macro(run_list, output_dir):
    cmd = f"root -l -b -q '{ROOT_MACRO_PATH}(\"{run_list}\", \"{output_dir}\")'"
    print(f"Executing ROOT macro to combine runs: {cmd}")
    subprocess.run(cmd, shell=True)


def main():
    print("### run_combine_runs.py ###")
    args = arg_parser()
    print(f"Periods to combine: {args.periods}")
    print(f"Output directory  : {args.output}")
    make_output_dir(args.output)
    run_list = make_run_list(args.periods)
    execute_root_macro(run_list, args.output)

if __name__ == "__main__":
    main()