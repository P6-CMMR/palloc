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
    
    # Go through all runs
    for run_traces in data["traces"]:
        # For each run, go through all traces
        for trace in run_traces:
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

    # Dropoff layer
    dropoff_layer = folium.FeatureGroup(name="Dropoff Points", show=False)
    for coords in dropoff_points:
        folium.CircleMarker(
            location=coords,
            radius=5,
            color="blue",
            fill=True,
            fill_color="blue",
            tooltip="Dropoff Point",
        ).add_to(dropoff_layer)
    m.add_child(dropoff_layer)

    # Parking layer
    parking_weights = env["parking_weights"]
    parking_layer = folium.FeatureGroup(name="Parking Spots")
    for i, coords in enumerate(parking_points):
        capacity = env["parking_capacities"][i]
        spot_text = "spot" if capacity == 1 else "spots"
        folium.CircleMarker(
            location=coords,
            radius=5,
            color="green",
            fill=True,
            fill_color="green",
            tooltip=f"Parking Capacity: {capacity} {spot_text}<br>Weight: {parking_weights[i]}",
        ).add_to(parking_layer)
    m.add_child(parking_layer)

    # Bounding box
    bounds = [
        [min(p[0] for p in all_points), min(p[1] for p in all_points)],
        [max(p[0] for p in all_points), max(p[1] for p in all_points)]
    ]
    
    bounds_layer = folium.FeatureGroup(name="Bounding Box", show=False)
    folium.Rectangle(
        bounds=bounds,
        color="blue",
        fill=True,
        fill_opacity=0.1,
        tooltip="Bounding Box"
    ).add_to(bounds_layer)
    m.add_child(bounds_layer)
    
    # Lat/lon tooltip
    m.add_child(folium.LatLngPopup())
    
    # Density grid layer
    density_grid = env["density_grid"]
    density_points = []
    for point in density_grid:
        lat = point["latitude"]
        lon = point["longitude"]
        intensity = point["intensity"]
        
       
        density_points.append([lat, lon, intensity])
        
    density_heatmap = HeatMap(
        density_points,
        name="Density Grid",
        radius=15,
        gradient={
            "0.2": "blue",
            "0.4": "cyan",
            "0.6": "lime",
            "0.8": "yellow",
            "1.0": "red"
        },
        show=False
    )
    m.add_child(density_heatmap)

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

def create_experiment_html(env, data, output_dir_path, experiment_name="", result_file="", single_file=False):
    """Create html from simulation data and save to experiment directory."""
    os.makedirs(output_dir_path, exist_ok=True)
    
    map_html_link = create_map_visualization(env, data, output_dir_path)
    
    all_traces = data.get("traces", [])
    num_runs = len(all_traces)
    
    metrics = {
        "available_parking_spots": {"title": "Available Parking Spots Over Time", "y_label": "# available parking spots"},
        "number_of_ongoing_simulations": {"title": "Number of Ongoing Simulations Over Time", "y_label": "# simulations"},
        "cost": {"title": "Cost Over Time", "y_label": "cost"},
        "average_duration": {"title": "Average Duration Over Time", "y_label": "average duration"},
        "dropped_requests": {"title": "Dropped Requests Over Time", "y_label": "# dropped requests"}
    }
    
    figures = {}
    for metric_name, info in metrics.items():
        fig = px.line(title=info["title"])
        
        all_timesteps = set()
        timestep_data = {}
        
        # For each run, add a line to the plot
        for run_idx, run_traces in enumerate(all_traces):
            if not run_traces:
                continue
            
            df = pd.DataFrame(run_traces)
            
            df["time_labels"] = df.apply(
                lambda row: f'{row["timestep"]}: {format_minutes_to_time(row["current_time_of_day"])}', 
                axis=1
            )
            
            for _, row in df.iterrows():
                timestep = row["timestep"]
                all_timesteps.add(timestep)
                
                if timestep not in timestep_data:
                    timestep_data[timestep] = {
                        "values": [],
                        "time_label": row["time_labels"]
                    }
                
                timestep_data[timestep]["values"].append(row[metric_name])
            
            if metric_name == "average_duration":
                df = df[df[metric_name] > 0]
                
            fig.add_scatter(
                x=df["time_labels"],
                y=df[metric_name],
                name=f"Run {run_idx + 1}",
                mode="lines"
            )
        
        if len(all_traces) > 1:
            avg_x = []
            avg_y = []
            
            # Calculate average for each timestep
            for timestep in sorted(all_timesteps):
                if timestep in timestep_data:
                    values = timestep_data[timestep]["values"]
                    filtered_values = [v for v in values if v > 0]
                    if filtered_values: 
                        avg_x.append(timestep_data[timestep]["time_label"])
                        avg_y.append(sum(filtered_values) / len(filtered_values))
                
            # Add average line
            if avg_x:
                fig.add_scatter(
                    x=avg_x,
                    y=avg_y,
                    name="Average",
                    mode="lines",
                    line=dict(color="black", width=2, dash="dash")
                )
        
        fig.update_layout(
            xaxis_title="time of day",
            yaxis_title=info["y_label"],
            xaxis=dict(
                tickangle=45,
                nticks=12
            ),
            legend=dict(
                orientation="h",
                yanchor="bottom",
                y=1.02,
                xanchor="right",
                x=1
            )
        )

        figures[metric_name] = fig
    
    try:
        button_template_path = Path(__file__).parent / "button_template.html"
        with open(button_template_path, "r") as f:
            button_template = f.read()

    except Exception as e:
        print(f"Error loading button template: {e}", file=sys.stderr)
        sys.exit(1)
    
    write_html_with_button(figures["available_parking_spots"], "parking_spots.html", button_template, output_dir_path)
    write_html_with_button(figures["number_of_ongoing_simulations"], "simulations.html", button_template, output_dir_path)
    write_html_with_button(figures["cost"], "cost.html", button_template, output_dir_path)
    write_html_with_button(figures["average_duration"], "duration.html", button_template, output_dir_path)
    write_html_with_button(figures["dropped_requests"], "dropped_requests.html", button_template, output_dir_path)
    
    # Settings
    settings = data.get("settings", {})
    timesteps = settings.get("timesteps", "N/A")
    start_time_raw = settings.get("start_time", "N/A")
    start_time = format_minutes_to_time(start_time_raw)
    
    max_request_duration_raw = settings.get("max_request_duration", "N/A")
    max_request_duration = f"{max_request_duration_raw}m" if max_request_duration_raw != "N/A" else "N/A"

    max_request_arrival_raw = settings.get("max_request_arrival", "N/A")
    max_request_arrival = f"{max_request_arrival_raw}m" if max_request_arrival_raw != "N/A" else "N/A"
    
    min_parking_time_raw = settings.get("min_parking_time", "N/A")
    min_parking_time = f"{min_parking_time_raw}m" if min_parking_time_raw != "N/A" else "N/A"
    
    request_rate = settings.get("request_rate", "N/A")
    
    batch_interval_raw = settings.get("batch_interval", "N/A")
    batch_interval = f"{batch_interval_raw}m" if batch_interval_raw != "N/A" else "N/A"
    
    using_weighted_parking = settings.get("using_weighted_parking", "N/A")
    
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
    
    if not single_file:
        parts = experiment_name.split(" / ")
        exp_part = parts[0].replace("-", " ").title()
        config_part = parts[1]
        experiment_name = f"{exp_part} / {config_part}"
    
    run_tabs_html = ""
    num_runs_html = ""
    if num_runs > 1:
        num_runs_html = f"<p>Number of aggregated runs: <strong>{num_runs}</strong></p>"
        experiment_name += f" (aggregated {num_runs} runs)"
        run_tabs_html = '<div class="run-tabs-controls">'
        run_tabs_html += '<div class="tab-buttons">'

        for run_idx in range(num_runs):
            btn_class = "active" if run_idx == 0 else ""
            run_tabs_html += f'<button class="tab-button {btn_class}" onclick="showTab({run_idx})" id="tab-btn-{run_idx}">Run {run_idx + 1}</button>'

        run_tabs_html += "</div></div>"
    
    assignments_html = ""
    for run_idx, run_traces in enumerate(all_traces):
        display = "block" if run_idx == 0 else "none"
        assignments_html += f'<div class="tab-content" id="tab-{run_idx}" style="display: {display};">'
        
        run_assignments_html = ""
        for trace in run_traces:
            if "assignments" in trace and trace["assignments"]:
                timestep = trace.get("timestep", "N/A")
                run_assignments_html += f"<h3>Timestep {timestep}</h3>"
                
                current_time_of_day_raw = trace.get("current_time_of_day", "N/A")
                current_time_of_day = format_minutes_to_time(current_time_of_day_raw)
                    
                run_assignments_html += f"<p>Time of day: {current_time_of_day}</p>"
                
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
                        
                        # OSRM format
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
                    
                    run_assignments_html += f"""
                    <div class="assignment-item">
                        <div><span class="assignment-label">Assignment {idx+1}:</span></div>
                        <div>Dropoff: (lat: {dropoff_lat}, lon: {dropoff_lon})</div>
                        <div>Parking: (lat: {parking_lat}, lon: {parking_lon})</div>
                        <div>Request duration: {request_duration}</div>
                        <div>Route duration: {route_duration}</div>
                        <div class="route-link">{route_link}</div>
                    </div>
                    """
        
        if not run_assignments_html:
            run_assignments_html = "<p>No assignment data available for this run.</p>"
            
        assignments_html += run_assignments_html + "</div>"
    
    html_content = template.replace("{{timesteps}}", str(timesteps))
    html_content = html_content.replace("{{start_time}}", str(start_time))
    html_content = html_content.replace("{{max_request_duration}}", str(max_request_duration))
    html_content = html_content.replace("{{max_request_arrival}}", str(max_request_arrival))
    html_content = html_content.replace("{{min_parking_time}}", str(min_parking_time))
    html_content = html_content.replace("{{request_rate}}", str(request_rate))
    html_content = html_content.replace("{{batch_interval}}", str(batch_interval))
    html_content = html_content.replace("{{using_weighted_parking}}", str(using_weighted_parking))
    html_content = html_content.replace("{{using_weighted_parking}}", str(using_weighted_parking))
    html_content = html_content.replace("{{seed}}", str(seed))
    html_content = html_content.replace("{{total_dropped}}", str(total_dropped))
    html_content = html_content.replace("{{global_avg_duration}}", global_avg_duration)
    html_content = html_content.replace("{{global_avg_cost}}", str(global_avg_cost))
    html_content = html_content.replace("{{run_tabs}}", run_tabs_html)
    html_content = html_content.replace("{{assignments_list}}", assignments_html)
    html_content = html_content.replace("{{map_link}}", map_html_link)
    html_content = html_content.replace("{{experiment_name}}", experiment_name)
    html_content = html_content.replace("{{result_file}}", os.path.basename(result_file) if result_file else "")
    html_content = html_content.replace("{{num_runs}}", num_runs_html)
    
    if single_file:
        # Remove the back button for single file mode
        html_content = html_content.replace('<a href="../index.html" class="back-button">Back to All Experiments</a>', '')
    
    report_file = os.path.join(output_dir_path, "experiment.html")
    with open(report_file, "w") as f:
        f.write(html_content)

def create_browser_index(experiments_root):
    """Create main index.html browser from template"""
    output_path = Path("report")
    os.makedirs(output_path, exist_ok=True)
    
    # Find all experiment directories
    exp_dirs = sorted(glob.glob(os.path.join(experiments_root, "experiment-*")))
    
    if not exp_dirs:
        print(f"No experiment directories found in {experiments_root}")
        return None
    
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
                            <div class="config-detail">Rate: {rate}</div>
                        </div>
                    </a>
                </div>
                """
            
            experiments_html += "</div>"
        else:
            experiments_html += '<p class="no-configs">No configuration files found in this experiment.</p>'
        
        experiments_html += "</div>"
    
    browser_html = browser_template.replace("{{experiments}}", experiments_html)
    
    index_path = os.path.join(output_path, "index.html")
    with open(index_path, "w") as f:
        f.write(browser_html)
    
    print(f"Experiments browser created at {index_path}")
    return index_path

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
        exp_name_display = exp_name.replace("-", " ").title()
        
        json_files = sorted(glob.glob(os.path.join(exp_dir, "*.json")))
        
        for json_file in json_files:
            config_name = os.path.basename(json_file).replace(".json", "")

            exp_config_dir = f"{exp_name}_{config_name}"
            output_dir = report_root / exp_config_dir
            
            # Load data and create HTML
            data = load_results(json_file)
            create_experiment_html(env, data, output_dir, f"{exp_name_display} / {config_name}", json_file)
            
            print(f"Created report for {exp_name}/{config_name}")

def process_single_file(env, result_file):
    """Process a single result file"""
    report_root = Path("report")
    os.makedirs(report_root, exist_ok=True)
    
    base_name = os.path.basename(result_file).replace(".json", "")
    output_dir = report_root / base_name
    
    # Load data and create HTML
    data = load_results(result_file)
    create_experiment_html(env, data, output_dir, "", result_file, True)
    
    print(f"Created report for {base_name}")
    
    return os.path.join(output_dir, "experiment.html")

def main():
    parser = argparse.ArgumentParser(description="Create plots from Palloc simulation results")
    parser.add_argument("env_file", help="Path to environment file")
    parser.add_argument("results", help="Path to experiment directory or a single JSON result file")
    args = parser.parse_args()
    
    env = load_env(args.env_file)
    
    # Check if the results argument is a directory or a file
    if os.path.isdir(args.results):
        # Process all experiments
        process_experiments(env, args.results)
        index_path = create_browser_index(args.results)
        
        if index_path:
            print(f"\nAll reports generated. Open {index_path} to browse experiments.")
        else:
            print("\nNo experiments found.")
    else:
        # Process single file
        report_path = process_single_file(env, args.results)
        print(f"Open {report_path} to view results.")

if __name__ == "__main__":
    main()