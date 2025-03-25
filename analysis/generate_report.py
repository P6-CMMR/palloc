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

def create_html(data):
    """Create html from simulation data and save to directory."""
    output_dir_path = Path("plots")

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
    
    os.makedirs(output_dir_path, exist_ok=True)
    
    try:
        button_template_path = Path(__file__).parent / "button_template.html"
        with open(button_template_path, "r") as f:
            button_template = f.read()
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
        template_path = Path(__file__).parent / "template.html"
        with open(template_path, "r") as f:
            template = f.read()
    except Exception as e:
        print(f"Error loading template: {e}", file=sys.stderr)
        sys.exit(1)
    
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
    
    with open(os.path.join(output_dir_path, "index.html"), "w") as f:
        f.write(html_content)
    
    print(f"Plots saved to {output_dir_path}")
    print(f"Open {os.path.join(output_dir_path, "index.html")} to view all plots")

def main():
    parser = argparse.ArgumentParser(description="Create plots from Palloc simulation results")
    parser.add_argument("json_file", help="Path to the JSON results file")
    args = parser.parse_args()
    
    data = load_results(args.json_file)
    create_html(data)

if __name__ == "__main__":
    main()