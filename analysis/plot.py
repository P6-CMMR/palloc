import plotly.express as px
import pandas as pd
import json
import sys
import os
import argparse
from pathlib import Path

def load_results(json_file):
    """Load simulation results from a JSON file."""
    try:
        with open(json_file, "r") as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading results: {e}", file=sys.stderr)
        sys.exit(1)

def format_duration_min_sec(duration_in_minutes):
    """Convert decimal minutes to minutes and seconds format."""
    minutes = int(duration_in_minutes)
    seconds = int((duration_in_minutes - minutes) * 60)
    return f"{minutes}m {seconds}s"

def create_plots(data):
    """Create plots from simulation data and save to directory."""
    output_dir_path = Path("plots")

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
    
    fig5 = px.line(traces, x="timestep", y="dropped_requests",
                  title="Dropped Requests Over Time")
    fig5.update_yaxes(title_text="# dropped requests")
    
    os.makedirs(output_dir_path, exist_ok=True)
    fig1.write_html(os.path.join(output_dir_path, "parking_spots.html"))
    fig2.write_html(os.path.join(output_dir_path, "simulations.html"))
    fig3.write_html(os.path.join(output_dir_path, "cost.html"))
    fig4.write_html(os.path.join(output_dir_path, "duration.html"))
    fig5.write_html(os.path.join(output_dir_path, "dropped_requests.html"))
    
    # Settings
    settings = data.get("settings", {})
    timesteps = settings.get("timesteps", "N/A")
    max_duration = settings.get("max_duration", "N/A")
    max_request_per_step = settings.get("max_request_per_step", "N/A")
    batch_interval = settings.get("batch_interval", "N/A")
    seed = settings.get("seed", "N/A")
    
    # Stats
    total_dropped = data.get("total_dropped_requests", "N/A")
    global_avg_duration = format_duration_min_sec(data.get("global_avg_duration", 0))
    global_avg_cost = round(data.get("global_avg_cost", 0), 2)
    
    try:
        template_path = Path(__file__).parent / "template.html"
        with open(template_path, "r") as f:
            template = f.read()
    except Exception as e:
        print(f"Error loading template: {e}", file=sys.stderr)
        sys.exit(1)
    
    html_content = template.replace("{{timesteps}}", str(timesteps))
    html_content = html_content.replace("{{max_duration}}", str(max_duration))
    html_content = html_content.replace("{{max_request_per_step}}", str(max_request_per_step))
    html_content = html_content.replace("{{batch_interval}}", str(batch_interval))
    html_content = html_content.replace("{{seed}}", str(seed))
    html_content = html_content.replace("{{total_dropped}}", str(total_dropped))
    html_content = html_content.replace("{{global_avg_duration}}", global_avg_duration)
    html_content = html_content.replace("{{global_avg_cost}}", str(global_avg_cost))
    
    with open(os.path.join(output_dir_path, "index.html"), "w") as f:
        f.write(html_content)
    
    print(f"Plots saved to {output_dir_path}")
    print(f"Open {os.path.join(output_dir_path, 'index.html')} to view all plots")

def main():
    parser = argparse.ArgumentParser(description="Create plots from Palloc simulation results")
    parser.add_argument("json_file", help="Path to the JSON results file")
    args = parser.parse_args()
    
    data = load_results(args.json_file)
    create_plots(data)

if __name__ == "__main__":
    main()