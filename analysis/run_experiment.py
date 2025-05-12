import os
import platform
import sys
import argparse
import glob
import time
import re
import multiprocessing
import subprocess
import threading
import queue
import shutil
import json
import random
from progress_bar import print_progress_bar
from datetime import datetime

def get_palloc_path():
    """Get path to palloc based on OS"""
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    system = platform.system()
    if system == "Linux":
        return os.path.join(project_root, "build/palloc-linux/bin/palloc")
    elif system == "Windows":
        return os.path.join(project_root, "build\\palloc-windows\\bin\\palloc.exe")
    elif system == "Darwin":  # macOS
        return os.path.join(project_root, "build/palloc-macos/bin/palloc")
    else:
        print(f"Unsupported operating system: {system}")
        sys.exit(1)

def check_environment(env_path: str):
    """Check if required files exist"""
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    
    palloc_path = get_palloc_path()
    if not os.path.isfile(palloc_path):
        print(f"Error: palloc executable not found at {palloc_path}")
        print("Please compile the project first.")
        sys.exit(1)
    
    env_file = os.path.join(project_root, env_path)
    if not os.path.isfile(env_file):
        print(f"Error: {env_path} not found in project root.")
        print("Please run the setup script to create the environment file.")
        sys.exit(1)
    
    return True
    
def parse_arguments():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(add_help=False)
    
    parser.add_argument("-h", "--help", action="store_true", help="Show help message")
    parser.add_argument("-e", "--environment", help="Path to the environment file")
    parser.add_argument("-t", "--timesteps", default="1440", help="Number of timesteps")
    parser.add_argument("-S", "--start-time", default="08:00", help="Start time in HH:MM format")
    parser.add_argument("-d", "--duration", default="2880", help="Max duration in minutes or range")
    parser.add_argument("-A", "--arrival", default="0", help="Max time till arrival in minutes or range")
    parser.add_argument("-m", "--minimum-parking-time", default="0", help="Minimum parking time in minutes")
    parser.add_argument("-r", "--request-rate", default="4.0", help="Request rate per timestep or range")
    parser.add_argument("-b", "--batch-interval", default="3", help="interval in minutes before processing requests")
    parser.add_argument("-c", "--commit-interval", default="0", help="interval before arriving a request can be committed to a parking spot")
    parser.add_argument("-w", "--weights", action="store_true", help="Use weights for distance to parking")
    parser.add_argument("-g", "--random-generator", default="pcg", help="Random number generator to use (options: pcg, pcg-fast)")
    parser.add_argument("-s", "--seed", default=str(int(time.time() * 1000) % 1000000), help="Random seed for reproducibility")
    parser.add_argument("-T", "--trace", action="store_true", help="Output trace or not")
    parser.add_argument("-a", "--aggregate", default="3", help="Number of runs per configuration")
    parser.add_argument("-j", "--jobs", default=str(multiprocessing.cpu_count()), help="Number of parallel jobs")
    
    args = parser.parse_args()
    
    if args.help:
        sys.exit(0)
    
    if not args.environment:
        sys.exit("Error: Environment file path is required. Use -e or --environment to specify it.")
    
    return args

def parse_range(value_str, is_float=False):
    """Parse a range string e.g. '10-20'"""
    if "-" in value_str:
        start, end = value_str.split("-", 1)
        if is_float:
            return float(start), float(end)
        else:
            return int(start), int(end)
    else:
        if is_float:
            return float(value_str), 0
        else:
            return int(value_str), 0
    
def get_next_experiment_dir():
    """Find the next experiment directory number"""
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    experiments_dir = os.path.join(project_root, "experiments")
    
    os.makedirs(experiments_dir, exist_ok=True)
    
    highest_num = 0
    for dir_path in glob.glob(os.path.join(experiments_dir, "experiment-*")):
        if os.path.isdir(dir_path):
            dir_name = os.path.basename(dir_path)
            match = re.match(r"experiment-(\d+)", dir_name)
            if match:
                num = int(match.group(1))
                highest_num = max(highest_num, num)
    
    next_num = highest_num + 1
    exp_dir = os.path.join(experiments_dir, f"experiment-{next_num}")
    os.makedirs(exp_dir, exist_ok=True)
    
    return exp_dir

def create_summary_file(exp_dir, args, duration_range, arrival_range, minimum_parking_time_range, rate_range, commit_range):
    """Create a summary file with experiment parameters"""
    summary_path = os.path.join(exp_dir, "summary.txt")
    
    duration_step = 15
    arrival_step = 5
    minimum_parking_step = 5
    rate_step = 0.5
    commit_step = 5
    
    with open(summary_path, "w") as f:
        f.write("Experiment Summary\n")
        f.write(f"Date: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}\n\n")
        f.write(f"Environment file: {args.environment}\n")
        f.write(f"Generate traces: {args.trace}\n")
        f.write(f"Timesteps: {args.timesteps}\n")
        f.write(f"Start time: {args.start_time}\n")
    
        duration_start, duration_end = duration_range
        arrival_start, arrival_end = arrival_range
        minimum_parking_time_start, minimum_parking_time_end = minimum_parking_time_range
        rate_start, rate_end = rate_range
        commit_start, commit_end = commit_range
        
        duration_count = 1
        if duration_end > 0:
            duration_count = 0
            current_dur = duration_start
            while current_dur <= duration_end:
                duration_count += 1
                current_dur += duration_step
            f.write(f"Duration range: {duration_start}-{duration_end} (step: {duration_step})\n")
        else:
            f.write(f"Duration: {duration_start}\n")
        
        arrival_count = 1
        if arrival_end > 0:
            arrival_count = 0
            current_arv = arrival_start
            while current_arv <= arrival_end:
                arrival_count += 1
                current_arv += arrival_step
            f.write(f"Arrival range: {arrival_start}-{arrival_end} (step: {arrival_step})\n")
        else:
            f.write(f"Arrival: {arrival_start}\n")
        
        minimum_parking_time_count = 1
        if minimum_parking_time_end > 0:
            minimum_parking_time_count = 0
            current_min_parking_time = minimum_parking_time_start
            while current_min_parking_time <= minimum_parking_time_end:
                minimum_parking_time_count += 1
                current_min_parking_time += minimum_parking_step
            f.write(f"Minimum parking time range: {minimum_parking_time_start}-{minimum_parking_time_end} (step: {minimum_parking_step})\n")
        else:
            f.write(f"Minimum parking time: {minimum_parking_time_start}\n")
        
        rate_count = 1
        if rate_end > 0:
            rate_count = 0
            current_rate = rate_start
            eps = 1e-6
            while current_rate <= rate_end + eps:
                rate_count += 1
                current_rate += rate_step
            f.write(f"Request rate range: {rate_start}-{rate_end} (step: {rate_step})\n")
        else:
            f.write(f"Request rate: {rate_start}\n")

        f.write(f"Batch interval: {args.batch_interval}\n")

        commit_count = 1
        if commit_end > 0:
            commit_count = 0
            current_commit = commit_start
            while current_commit <= commit_end:
                commit_count += 1
                current_commit += commit_step
            f.write(f"Commit interval range: {rate_start}-{rate_end} (step: {rate_step})\n")
        else:
            f.write(f"Commit interval: {rate_start}\n")
        
        total_configs = duration_count * arrival_count * minimum_parking_time_count * rate_count * commit_count
            
        f.write(f"Total configurations: {total_configs}\n")
        f.write(f"Random generator: {args.random_generator}\n")
        f.write(f"Seed: {args.seed}\n")
        f.write(f"Number of runs per configuration: {args.aggregate}\n")
        f.write(f"Parallel jobs: {args.jobs}\n")
        f.write("----------------------------------------\n")
    
    return total_configs, (duration_step, arrival_step, minimum_parking_step, rate_step, commit_step)
        
def run_job(job_tuple, args, progress_queue):
    """Run a single simulation job"""
    duration, arrival, min_parking_time, rate, commit, output_file = job_tuple
    
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    palloc_path = get_palloc_path()
    
    cmd = [
        palloc_path,
        "-e", os.path.join(project_root, args.environment),
        "-t", args.timesteps,
        "-S", args.start_time,
        "-d", duration,
        "-A", arrival,
        "-m", min_parking_time,
        "-r", rate,
        "-b", args.batch_interval,
        "-c", commit,
        "-g", args.random_generator,
        "-s", args.seed,
        "-o", output_file,
        "-a", args.aggregate,
    ]
    
    if args.weights:
        cmd.append("-w")
        
    if args.trace:
        cmd.append("-T")
    
    try:
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)    
        progress_queue.put(1)
        
    except subprocess.CalledProcessError as e:
        print(f"\nError running job: {e}")
        print(f"Command: {" ".join(cmd)}")
        progress_queue.put(0)
    
def run_job_wrapper(args_tuple):
    job_tuple, args, _, progress_queue = args_tuple
    run_job(job_tuple, args, progress_queue)

def display_progress(progress_dict, total):
    """Thread function to display progress"""
    start_time = time.time()
    
    while progress_dict["running"]:
        completed = progress_dict["completed"]
        print_progress_bar(completed, total, start_time)
        time.sleep(0.5)

def run_jobs(jobs, args, exp_dir):
    print(f"Running simulations in parallel with {args.jobs} concurrent jobs...")
    
    total_jobs = len(jobs)
    
    manager = multiprocessing.Manager()
    progress_queue = manager.Queue()
    
    progress_dict = {"completed": 0, "running": True}
    display_thread = threading.Thread(
        target=display_progress, 
        args=(progress_dict, total_jobs)
    )
    
    display_thread.daemon = True
    display_thread.start()
    
    job_args = [(job, args, exp_dir, progress_queue) for job in jobs]
    with multiprocessing.Pool(processes=int(args.jobs)) as pool:
        result = pool.map_async(run_job_wrapper, job_args)
        while not result.ready():
            try:
                progress_queue.get(timeout=0.1)
                progress_dict["completed"] += 1
            except queue.Empty:
                pass
    
    try:
        while not progress_queue.empty():
            progress_queue.get_nowait()
            progress_dict["completed"] += 1
    except queue.Empty:
        pass
    
    progress_dict["running"] = False
    display_thread.join(timeout=1)
    
    print_progress_bar(progress_dict["completed"], total_jobs, final=True)
    
    return total_jobs

def copy_missing_commit_files(exp_dir, commit_range, commit_step):
    existing_files = glob.glob(os.path.join(exp_dir, "*.json"))
    existing_configs = {}
    for file_path in existing_files:
        file_name = os.path.basename(file_path)
        match = re.match(r"d(\d+)-A(\d+)-m(\d+)-r([\d\.]+)-c(\d+)\.json", file_name)
        if match:
            duration, arrival, min_parking, rate, commit = match.groups()
            key = (duration, arrival, min_parking, rate)
            if key not in existing_configs:
                existing_configs[key] = {}
            existing_configs[key][commit] = file_path
    
    commit_copies = 0
    for key, commits in existing_configs.items():
        duration, arrival, min_parking, rate = key
        arrival_int = int(arrival)
        commit_start, commit_end = commit_range

        current_commit = commit_start
        while current_commit <= commit_end or commit_end == 0:
            if current_commit > arrival_int:
                commit_str = str(current_commit)
                if commit_str not in commits:
                    arrival_str = str(arrival_int)
                    if arrival_str in commits:
                        source_file = commits[arrival_str]
                        target_name = f"d{duration}-A{arrival}-m{min_parking}-r{rate}-c{commit_str}.json"
                        target_file = os.path.join(exp_dir, target_name)
                        shutil.copy2(source_file, target_file)
                        
                        try:
                            with open(target_file, 'r') as f:
                                data = json.load(f)
                            
                            data["settings"]["commit_interval"] = current_commit
                            
                            with open(target_file, 'w') as f:
                                json.dump(data, f)
                        
                            commit_copies += 1
                        except Exception as e:
                            print(f"Error updating commit interval in {target_name}: {e}")
                            os.remove(target_file)
            if commit_end == 0:
                break
            current_commit += commit_step

    print(f"Created {commit_copies} additional result files for commit")

def main():
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(project_root)
    
    args = parse_arguments()
    check_environment(args.environment)
    
    duration_range = parse_range(args.duration)
    arrival_range = parse_range(args.arrival)
    minimum_parking_time_range = parse_range(args.minimum_parking_time)
    rate_range = parse_range(args.request_rate, is_float=True)
    commit_range = parse_range(args.commit_interval)
    
    # If commit larger then its the same as if it was equal to arrival
    if commit_range[0] > arrival_range[1] and arrival_range[1] > 0:
        commit_range = (arrival_range[1], 0)  
    elif commit_range[1] > arrival_range[1]:
        is_same = commit_range[0] == arrival_range[1]
        if is_same:
            commit_range = (arrival_range[1], 0)
        elif arrival_range[1] > 0:
            commit_range = (commit_range[0], arrival_range[1])

    exp_dir = get_next_experiment_dir()
    
    total_configs, steps = create_summary_file(exp_dir, args, duration_range, arrival_range, minimum_parking_time_range, rate_range, commit_range)
    duration_step, arrival_step, minimum_parking_step, rate_step, commit_step = steps
    
    print(f"Running {total_configs} simulations with {args.jobs} parallel jobs...")
    print("Parameters:")
    print(f"  - Environment file: {args.environment}")
    print(f"  - Generate traces: {args.trace}")
    print(f"  - Timesteps: {args.timesteps}")
    print(f"  - Start time: {args.start_time}")
    duration_start, duration_end = duration_range
    if duration_end > 0:
        print(f"  - Duration range: {duration_start}-{duration_end} (step: {duration_step})")
    else:
        print(f"  - Duration: {duration_start}")
    
    arrival_start, arrival_end = arrival_range
    if arrival_end > 0:
        print(f"  - Arrival range: {arrival_start}-{arrival_end} (step: {arrival_step})")
    else:
        print(f"  - Arrival: {arrival_start}")
    
    minimum_parking_time_start, minimum_parking_time_end = minimum_parking_time_range
    if minimum_parking_time_end > 0:
        print(f"  - Minimum parking time range: {minimum_parking_time_start}-{minimum_parking_time_end} (step: {minimum_parking_step})")
    else:
        print(f"  - Minimum parking time: {minimum_parking_time_start}")
    
    rate_start, rate_end = rate_range
    if rate_end > 0:
        print(f"  - Request rate range: {rate_start}-{rate_end} (step: {rate_step})")
    else:
        print(f"  - Request rate: {rate_start}")

    print(f"  - Batch interval: {args.batch_interval}")
    
    commit_start, commit_end = commit_range
    if commit_end > 0:
        print(f"  - Commit interval range: {commit_start}-{commit_end} (step: {commit_step})")
    else:
        print(f"  - Commit interval: {commit_start}")
    
    print(f"  - Weighted parking: {args.weights}")
    print(f"  - Random generator: {args.random_generator}")
    print(f"  - Seed: {args.seed}")
    print(f"  - Output directory: {exp_dir}")
    print(f"  - Number of runs per configuration: {args.aggregate}")
    print(f"  - Parallel jobs: {args.jobs}")
    print("----------------------------------------")
    
    jobs = []
    current_duration = duration_start
    while current_duration <= duration_end or duration_end == 0:
        current_arrival = arrival_start
        while current_arrival <= arrival_end or arrival_end == 0:
            current_min_parking_time = minimum_parking_time_start
            while current_min_parking_time <= minimum_parking_time_end or minimum_parking_time_end == 0:
                current_rate = rate_start
                while current_rate <= rate_end or rate_end == 0:
                    current_commit = commit_start
                    while current_commit <= commit_end or commit_end == 0: 
                        if current_commit > current_arrival:
                            current_commit += commit_step
                            continue
                                           
                        config_name = f"d{current_duration}-A{current_arrival}-m{current_min_parking_time}-r{current_rate}-c{current_commit}"
                        output_file = os.path.join(exp_dir, f"{config_name}.json")
                             
                        jobs.append((str(current_duration), str(current_arrival), str(current_min_parking_time), str(current_rate), str(current_commit), output_file))

                        if commit_end == 0:
                            break

                        current_commit += commit_step
                        
                    if rate_end == 0:
                        break
                    
                    current_rate += rate_step
                    current_rate = round(current_rate * 100) / 100

                if minimum_parking_time_end == 0:
                    break
                
                current_min_parking_time += minimum_parking_step
            
            if arrival_end == 0:
                break
            
            current_arrival += arrival_step
        
        if duration_end == 0:
            break
        
        current_duration += duration_step
    
    # Shuffle to make progress bar estimate better
    random.shuffle(jobs)
    
    run_jobs(jobs, args, exp_dir)
    
    print("\nCreating copies for commit larger than arrival...")
    copy_missing_commit_files(exp_dir, commit_range, commit_step)
        
    print("\nSimulations completed!")
    print(f"Created experiment directory: {exp_dir}")
    
    print("Generating reports...")
    
    report_script = os.path.join(project_root, "analysis", "generate_report.py")
    env_file = os.path.join(project_root, args.environment) 
    experiments_dir = os.path.join(project_root, "experiments")
    
    subprocess.run(["python", report_script, env_file, experiments_dir, "--experiments", exp_dir])

if __name__ == "__main__":
    main()