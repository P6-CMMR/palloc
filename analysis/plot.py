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

def create_plots(data):
    """Create plots from simulation data and save to directory."""
    output_dir_path = Path("plots")

    traces = pd.DataFrame(data["traces"])

    fig1 = px.line(traces, x="timestep", y="available_parking_spots", 
                  title="Available Parking Spots Over Time")
    fig1.update_yaxes(title_text="# available parking spots")
    fig1.update_xaxes(title_text="Time (minutes)")

    fig2 = px.line(traces, x="timestep", y="number_of_ongoing_simulations",
                  title="Number of Ongoing Simulations Over Time")
    fig2.update_yaxes(title_text="# simulations")
    fig2.update_xaxes(title_text="Time (minutes)")
    
    fig3 = px.line(traces, x="timestep", y="cost",
                  title="Cost Over Time")
    fig3.update_xaxes(title_text="Time (minutes)")
    
    fig4 = px.line(traces, x="timestep", y="average_duration",
                  title="Average Duration Over Time")
    fig4.update_yaxes(title_text="average duration")
    fig4.update_xaxes(title_text="Time (minutes)")
    
    os.makedirs(output_dir_path, exist_ok=True)
    fig1.write_html(os.path.join(output_dir_path, "parking_spots.html"))
    fig2.write_html(os.path.join(output_dir_path, "simulations.html"))
    fig3.write_html(os.path.join(output_dir_path, "cost.html"))
    fig4.write_html(os.path.join(output_dir_path, "duration.html"))
    
    index_html = """<!DOCTYPE html>
    <html>
    <head>
        <title>Palloc Simulation Results</title>
        <style>
            body { font-family: Arial, sans-serif; margin: 20px; }
            h1 { color: #2c3e50; }
            .plot-links { margin: 20px 0; }
            .plot-links a { 
                display: block; 
                margin: 10px 0; 
                padding: 10px; 
                background: #f5f5f5; 
                text-decoration: none;
                color: #3498db;
                border-radius: 5px;
            }
            .plot-links a:hover {
                background: #e0e0e0;
            }
        </style>
    </head>
    <body>
        <h1>Palloc Simulation Results</h1>
        <div class="plot-links">
            <a href="parking_spots.html">Available Parking Spots Over Time</a>
            <a href="simulations.html">Number of Ongoing Simulations Over Time</a>
            <a href="cost.html">Cost Over Time</a>
            <a href="duration.html">Average Duration Over Time</a>
        </div>
    </body>
    </html>"""
    
    with open(os.path.join(output_dir_path, "index.html"), "w") as f:
        f.write(index_html)
    
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