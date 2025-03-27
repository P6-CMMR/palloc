import plotly.express as px
import pandas as pd
import json
import sys
import os
import argparse
import folium
import os
import glob
from folium.plugins import HeatMap
from pathlib import Path

def load_results(result_file):
    """Load simulation results from a JSON file."""
    try:
        with open(result_file, "r") as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading results: {e}", file=sys.stderr)
        sys.exit(1)
        
def load_env(env_file):
    """Load env file from a JSON file."""
    try:
        with open(env_file, "r") as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading environment: {e}", file=sys.stderr)
        sys.exit(1)

def format_duration_min_sec(duration_in_minutes):
    """Convert decimal minutes to minutes and seconds format."""
    minutes = int(duration_in_minutes)
    seconds = int((duration_in_minutes - minutes) * 60)
    return f"{minutes}m {seconds}s"

def write_html_with_button(fig, filename, button_template, output_dir_path):
    """Write html with backlink"""
    fig_html = fig.to_html(include_plotlyjs="cdn")
    modified_html = fig_html.replace("<body>", f"<body>{button_template}")
    with open(os.path.join(output_dir_path, filename), "w") as f:
        f.write(modified_html)

def format_minutes_to_time(minutes_raw):
    """Convert minutes to HH:MM time format."""
    if minutes_raw == "N/A":
        return "N/A"
    
    try:
        total_minutes = int(minutes_raw)
        hours = total_minutes // 60
        minutes = total_minutes % 60
        return f"{hours:02d}:{minutes:02d}"
    except (ValueError, TypeError):
        return "N/A"

def create_map_visualization(env, data, output_dir_path):
    """Create an interactive map visualization of dropoff points, density, and parking spots."""
    dropoff_points = [(point["latitude"], point["longitude"]) for point in env["dropoff_coords"]]
    parking_points = [(point["latitude"], point["longitude"]) for point in env["parking_coords"]]

    all_points = dropoff_points + parking_points
    center_lat = sum(p[0] for p in all_points) / len(all_points)
    center_lng = sum(p[1] for p in all_points) / len(all_points)

    m = folium.Map(location=[center_lat, center_lng], zoom_start=14)
    simple_heatmap = HeatMap(
        dropoff_points,
        gradient={"0.4": "blue", "0.65": "lime", "1": "red"}, 
        radius=15,
        name="Dropoff Points Distribution",
        show=False
    )
    m.add_child(simple_heatmap)
    
    dropoff_assignment_points = []
    parking_assignment_points = []
    for trace in data["traces"]:
        for assignment in trace["assignments"]:
            dropoff_coord = assignment["dropoff_coordinate"]
            dropoff_lat = dropoff_coord["latitude"]
            dropoff_lon = dropoff_coord["longitude"]
            dropoff_assignment_points.append([dropoff_lat, dropoff_lon, 1.0])
            
            parking_coord = assignment["parking_coordinate"]
            parking_lat = parking_coord["latitude"]
            parking_lon = parking_coord["longitude"]
            parking_assignment_points.append([parking_lat, parking_lon, 1.0])
    
    dropoff_assignments_heatmap = HeatMap(
        dropoff_assignment_points,
        gradient={"0.4": "yellow", "0.65": "orange", "1": "red"}, 
        radius=15,
        name="Assigned Requests Dropoffs",
        show=True 
    )
    m.add_child(dropoff_assignments_heatmap)

    parking_assignments_heatmap = HeatMap(
        parking_assignment_points,
        gradient={"0.4": "purple", "0.65": "magenta", "1": "violet"},
        radius=15, 
        name="Assigned Parking Spots",
        show=False
    )
    m.add_child(parking_assignments_heatmap)

    # Parking layer
    parking_layer = folium.FeatureGroup(name="Parking Spots")
    for i, coords in enumerate(parking_points):
        capacity = env["parking_capacities"][i]
        folium.CircleMarker(
            location=coords,
            radius=5,
            color="green",
            fill=True,
            fill_color="green",
            tooltip=f"Parking Capacity: {capacity}"
        ).add_to(parking_layer)
    m.add_child(parking_layer)

    folium.LayerControl().add_to(m)

    map_path = os.path.join(output_dir_path, "density_map.html")

    button_template_path = Path(__file__).parent / "button_template.html"
    with open(button_template_path, "r") as f:
        button_template = f.read()
    
    button_template = button_template.replace("left: 10px", "left: 60px")
    
    map_html = m.get_root().render()
    map_html = map_html.replace("<body>", f"<body>{button_template}")
    
    with open(map_path, "w") as f:
        f.write(map_html)

    return f'<p><a href="density_map.html" class="nav-button">View Interactive Map</a></p>'

def create_experiment_html(env, data, output_dir_path, experiment_name="", result_file=""):
    """Create html from simulation data and save to experiment directory."""
    os.makedirs(output_dir_path, exist_ok=True)
    
    map_html_link = create_map_visualization(env, data, output_dir_path)
    
    traces = pd.DataFrame(data["traces"])
    traces["time_labels"] = traces.apply(
        lambda row: f"{row["timestep"]}: {format_minutes_to_time(row["current_time_of_day"])}", 
        axis=1
    )

    fig1 = px.line(traces, x="time_labels", y="available_parking_spots", 
                  title="Available Parking Spots Over Time")
    fig1.update_yaxes(title_text="# available parking spots")

    fig2 = px.line(traces, x="time_labels", y="number_of_ongoing_simulations",
                  title="Number of Ongoing Simulations Over Time")
    fig2.update_yaxes(title_text="# simulations")
    
    fig3 = px.line(traces, x="time_labels", y="cost",
                  title="Cost Over Time")
    
    fig4 = px.line(traces, x="time_labels", y="average_duration",
                  title="Average Duration Over Time")
    fig4.update_yaxes(title_text="average duration")
    
    fig5 = px.line(traces, x="time_labels", y="dropped_requests",
                  title="Dropped Requests Over Time")
    fig5.update_yaxes(title_text="# dropped requests")
    
    for fig in [fig1, fig2, fig3, fig4, fig5]:
        fig.update_xaxes(
            nticks=12,
            tickangle=45,
            title_text="time of day"
        )
    
    try:
        button_template_path = Path(__file__).parent / "button_template.html"
        with open(button_template_path, "r") as f:
            button_template = f.read()
            
        # Add backlink to experiment
        back_button = '<a href="../experiment.html" class="nav-button" style="right:10px;">Back to Experiments</a>'
        button_template += back_button
    except Exception as e:
        print(f"Error loading button template: {e}", file=sys.stderr)
        sys.exit(1)
    
    write_html_with_button(fig1, "parking_spots.html", button_template, output_dir_path)
    write_html_with_button(fig2, "simulations.html", button_template, output_dir_path)
    write_html_with_button(fig3, "cost.html", button_template, output_dir_path)
    write_html_with_button(fig4, "duration.html", button_template, output_dir_path)
    write_html_with_button(fig5, "dropped_requests.html", button_template, output_dir_path)
    
    # Settings
    settings = data.get("settings", {})
    timesteps = settings.get("timesteps", "N/A")
    start_time_raw = settings.get("start_time", "N/A")
    start_time = format_minutes_to_time(start_time_raw)
    
    max_request_duration_raw = settings.get("max_request_duration", "N/A")
    max_request_duration = f"{max_request_duration_raw}m" if max_request_duration_raw != "N/A" else "N/A"
    
    request_rate = settings.get("request_rate", "N/A")
    
    batch_interval_raw = settings.get("batch_interval", "N/A")
    batch_interval = f"{batch_interval_raw}m" if batch_interval_raw != "N/A" else "N/A"
    
    seed = settings.get("seed", "N/A")
    
    # Stats
    total_dropped = data.get("total_dropped_requests", "N/A")
    global_avg_duration = format_duration_min_sec(data.get("global_avg_duration", 0))
    global_avg_cost = round(data.get("global_avg_cost", 0), 2)
    
    try:
        template_path = Path(__file__).parent / "experiment_template.html"
        with open(template_path, "r") as f:
            template = f.read()
    except Exception as e:
        print(f"Error loading template: {e}", file=sys.stderr)
        sys.exit(1)
    
    parts = experiment_name.split(" / ")
    exp_part = parts[0].replace("-", " ").title()
    config_part = parts[1]
    experiment_name = f"{exp_part} / {config_part}"
    
    assignments_html = ""
    if "traces" in data:
        for trace in data["traces"]:
            if "assignments" in trace and trace["assignments"]:
                timestep = trace.get("timestep", "N/A")
                assignments_html += f"<h3>Timestep {timestep}</h3>"
                
                current_time_of_day_raw = trace.get("current_time_of_day", "N/A")
                current_time_of_day = format_minutes_to_time(current_time_of_day_raw)
                    
                assignments_html += f"<p>Time of day: {current_time_of_day}</p>"
                
                for idx, assignment in enumerate(trace["assignments"]):
                    dropoff = assignment.get("dropoff_coordinate", {})
                    parking = assignment.get("parking_coordinate", {})
                    request_duration = assignment.get("request_duration", "N/A")
                    route_duration = assignment.get("route_duration", "N/A")
                    
                    dropoff_lat = dropoff.get("latitude", "N/A")
                    dropoff_lon = dropoff.get("longitude", "N/A")
                    parking_lat = parking.get("latitude", "N/A")
                    parking_lon = parking.get("longitude", "N/A")
                    
                    request_duration = f"{request_duration} min" if request_duration != "N/A" else "N/A"
                    route_duration = f"{route_duration} min" if route_duration != "N/A" else "N/A"
                    
                    # Calculate center coordinates for the map view
                    route_link = ""
                    if (dropoff_lat != "N/A" and dropoff_lon != "N/A" and 
                        parking_lat != "N/A" and parking_lon != "N/A"):
                        # Center point between dropoff and parking
                        center_lat = (float(dropoff_lat) + float(parking_lat)) / 2
                        center_lon = (float(dropoff_lon) + float(parking_lon)) / 2
                        
                        # OSRM format: ?z=16&center=lat,lon&loc=lat,lon&loc=lat,lon&loc=lat,lon
                        osrm_url = (
                            f"https://map.project-osrm.org/?"
                            f"z=16&"
                            f"center={center_lat}%2C{center_lon}&"
                            f"loc={dropoff_lat}%2C{dropoff_lon}&"
                            f"loc={parking_lat}%2C{parking_lon}&"
                            f"loc={dropoff_lat}%2C{dropoff_lon}&"
                            f"hl=en&alt=0&srv=0"
                        )
                        route_link = f'<a href="{osrm_url}" target="_blank" class="route-link-btn">View route on OSRM</a>'
                    
                    assignments_html += f"""
                    <div class="assignment-item">
                        <div><span class="assignment-label">Assignment {idx+1}:</span></div>
                        <div>Dropoff: (lat: {dropoff_lat}, lon: {dropoff_lon})</div>
                        <div>Parking: (lat: {parking_lat}, lon: {parking_lon})</div>
                        <div>Request duration: {request_duration}</div>
                        <div>Route duration: {route_duration}</div>
                        <div class="route-link">{route_link}</div>
                    </div>
                    """

    if not assignments_html:
        assignments_html = "<p>No assignment data available.</p>"
    
    html_content = template.replace("{{timesteps}}", str(timesteps))
    html_content = html_content.replace("{{start_time}}", str(start_time))
    html_content = html_content.replace("{{max_request_duration}}", str(max_request_duration))
    html_content = html_content.replace("{{request_rate}}", str(request_rate))
    html_content = html_content.replace("{{batch_interval}}", str(batch_interval))
    html_content = html_content.replace("{{seed}}", str(seed))
    html_content = html_content.replace("{{total_dropped}}", str(total_dropped))
    html_content = html_content.replace("{{global_avg_duration}}", global_avg_duration)
    html_content = html_content.replace("{{global_avg_cost}}", str(global_avg_cost))
    html_content = html_content.replace("{{assignments_list}}", assignments_html)
    html_content = html_content.replace("{{map_link}}", map_html_link)
    html_content = html_content.replace("{{experiment_name}}", experiment_name)
    html_content = html_content.replace("{{result_file}}", os.path.basename(result_file) if result_file else "")
    
    with open(os.path.join(output_dir_path, "experiment.html"), "w") as f:
        f.write(html_content)

def create_browser_index(experiments_root):
    """Create main index.html browser from template"""
    output_path = Path("report")
    os.makedirs(output_path, exist_ok=True)
    
    # Find all experiment directories
    exp_dirs = sorted(glob.glob(os.path.join(experiments_root, "experiment-*")))
    
    if not exp_dirs:
        print(f"No experiment directories found in {experiments_root}")
        return
    
    try:
        template_path = Path(__file__).parent / "browser_template.html"
        with open(template_path, "r") as f:
            browser_template = f.read()
    except Exception as e:
        print(f"Error loading browser template: {e}", file=sys.stderr)
        sys.exit(1)
    
    experiments_html = ""
    
    # Process each experiment directory
    for exp_dir in exp_dirs:
        exp_name = os.path.basename(exp_dir)
        exp_name_html = exp_name.replace("-", " ").title()
        experiments_html += f'<h2 class="experiment-title">{exp_name_html}</h2><div class="experiment-card">'
        
        # Check for summary file
        summary_file = os.path.join(exp_dir, "summary.txt")
        if os.path.exists(summary_file):
            with open(summary_file, "r") as f:
                summary_lines = f.readlines()
                experiments_html += '<div class="summary-info"><h3>Summary</h3><pre>'
                for line in summary_lines[:15]:  # Show first 15 lines
                    experiments_html += line
                experiments_html += "</pre></div>"
        
        json_files = sorted(glob.glob(os.path.join(exp_dir, "*.json")))
        
        if json_files:
            experiments_html += '<h3>Configurations</h3><div class="run-grid">'
            
            for json_file in json_files:
                config_name = os.path.basename(json_file).replace(".json", "")
                
                duration = "Unknown"
                rate = "Unknown"
                if config_name.startswith("d") and "-r" in config_name:
                    parts = config_name.split("-r")
                    if len(parts) == 2:
                        duration = parts[0][1:]  # Remove the "d" prefix
                        rate = parts[1]
                
                exp_config_dir = f"{exp_name}_{config_name}"
                
                experiments_html += f"""
                <div class="config-item">
                    <a href="{exp_config_dir}/experiment.html" class="config-link">
                        <div class="config-name">{config_name}</div>
                        <div class="config-details">
                            <div class="config-detail">Duration: {duration} min</div>
                            <div class="config-detail">Rate: {rate} req/min</div>
                        </div>
                    </a>
                </div>
                """
            
            experiments_html += "</div>"
        else:
            experiments_html += '<p class="no-configs">No configuration files found in this experiment.</p>'
        
        experiments_html += "</div>"
    
    browser_html = browser_template.replace("{{experiments}}", experiments_html)
    
    with open(os.path.join(output_path, "index.html"), "w") as f:
        f.write(browser_html)
    
    print(f"Experiments browser created at {os.path.join(output_path, "index.html")}")
    return os.path.join(output_path, "index.html")

def process_experiments(env, experiments_dir):
    """Process all experiments in the directory"""
    report_root = Path("report")
    os.makedirs(report_root, exist_ok=True)
    
    # Get all experiment directories
    exp_dirs = sorted(glob.glob(os.path.join(experiments_dir, "experiment-*")))
    
    if not exp_dirs:
        print(f"No experiment directories found in {experiments_dir}")
        return
    
    # Process each experiment directory
    for exp_dir in exp_dirs:
        exp_name = os.path.basename(exp_dir)
        
        json_files = sorted(glob.glob(os.path.join(exp_dir, "*.json")))
        
        for json_file in json_files:
            config_name = os.path.basename(json_file).replace(".json", "")

            exp_config_dir = f"{exp_name}_{config_name}"
            output_dir = report_root / exp_config_dir
            
            # Load data and create HTML
            data = load_results(json_file)
            create_experiment_html(env, data, output_dir, f"{exp_name} / {config_name}", json_file)
            
            print(f"Created report for {exp_name}/{config_name}")

def main():
    parser = argparse.ArgumentParser(description="Create plots from Palloc simulation results")
    parser.add_argument("env_file", help="Path to environment file")
    parser.add_argument("experiments_dir", help="Path to experiments directory")
    args = parser.parse_args()
    
    env = load_env(args.env_file)
    
    process_experiments(env, args.experiments_dir)
    index_path = create_browser_index(args.experiments_dir)
    
    print(f"\nReport generated. Open {index_path} to browse experiments.")

if __name__ == "__main__":
    main()