import plotly.express as px
import pandas as pd
import json
import subprocess
import sys
import os
import argparse
from pathlib import Path

def run_simulation(env_file, output_file, timesteps=None, duration=None, 
                  requests=None, batch_delay=None, seed=None):
    script_path = Path(__file__).resolve()
    project_root = script_path.parent.parent
    
    os.chdir(project_root)
    
    executable = project_root / "build" / "palloc"
    
    if not executable.exists():
        print(f"Error: Executable not found at {executable}", file=sys.stderr)
        sys.exit(1)
        
    cmd = [str(executable), "-e", env_file]
    
    cmd.extend(["-o", output_file])
        
    if timesteps is not None:
        cmd.extend(["-t", str(timesteps)])
    if requests is not None:
        cmd.extend(["-r", str(requests)])
    if seed is not None:
        cmd.extend(["-s", str(seed)])
    if duration is not None:
        cmd.extend(["-d", str(duration)])
    if batch_delay is not None:
        cmd.extend(["-b", str(batch_delay)])
        
    print(f"Running: {" ".join(cmd)}")
    
    try:
        subprocess.run(cmd, capture_output=True, text=True, check=True)
        print("Simulation completed successfully")
    except subprocess.CalledProcessError as e:
        print(f"Error running simulation: {e}", file=sys.stderr)
        print(f"Error output: {e.stderr}", file=sys.stderr)
        sys.exit(1)

def load_results(json_file):
    try:
        with open(json_file, "r") as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading results: {e}", file=sys.stderr)
        sys.exit(1)

def create_plots(data):
    output_dir_path = Path("plots")
    if not output_dir_path.is_absolute():
        output_dir_path = Path.cwd() / output_dir_path
    
    traces = pd.DataFrame(data["traces"])
    
    fig1 = px.line(traces, x="timestep", y="available_parking_spots", 
                  title="Available Parking Spots Over Time")
    fig1.update_yaxes(title_text="# available parking spots")

    fig2 = px.line(traces, x="timestep", y="number_of_ongoing_simulations",
                  title="Number of Ongoing Simulations Over Time")
    fig2.update_yaxes(title_text="# simulations")
    
    fig3 = px.line(traces, x="timestep", y="cost",
                  title="Cost Over Time")
    
    fig4 = px.line(traces, x="timestep", y="average_duration",
                  title="Average Duration Over Time")
    fig4.update_yaxes(title_text="average duration")
    
    os.makedirs(output_dir_path, exist_ok=True)
    fig1.write_html(os.path.join(output_dir_path, "parking_spots.html"))
    fig2.write_html(os.path.join(output_dir_path, "simulations.html"))
    fig3.write_html(os.path.join(output_dir_path, "cost.html"))
    fig4.write_html(os.path.join(output_dir_path, "duration.html"))
    print(f"Plots saved to {output_dir_path}")

def main():
    parser = argparse.ArgumentParser(description="Run Palloc and plot results")
    parser.add_argument("-e", "--environment", required=True, help="the environment file to simulate")
    parser.add_argument("-o", "--output", help="the output file to store results in")
    parser.add_argument("-t", "--timesteps", type=int, help="timesteps in minutes to run simulation")
    parser.add_argument("-d", "--duration", type=int, help="max duration in minutes of requests")
    parser.add_argument("-r", "--requests", type=int, help="max requests to generate per timestep")
    parser.add_argument("-b", "--batch-delay", type=int, help="delay in minutes before processing requests")
    parser.add_argument("-s", "--seed", type=int, help="seed for randomization")
    args = parser.parse_args()
    
    use_temp_file = args.output is None
    output_file = args.output
    
    if use_temp_file:
        import tempfile
        temp = tempfile.NamedTemporaryFile(suffix=".json", delete=False)
        output_file = temp.name
        temp.close()  
        
    run_simulation(args.environment, output_file, args.timesteps, args.duration, 
                  args.requests, args.batch_delay, args.seed)
    
    data = load_results(output_file)
    create_plots(data)
    
    if use_temp_file and os.path.exists(output_file):
        os.remove(output_file)
        print(f"Removed temporary output file")
    
if __name__ == "__main__":
    main()
    