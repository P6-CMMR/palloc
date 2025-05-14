import plotly.express as px
import plotly.graph_objects as go
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
import itertools
import numpy as np

UNUSED_SETTINGS = ["seed", "random_generator"]
ENABLE_EXTRA_GRAPH_CONFIGS = False

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

def write_html_with_buttons(figures, filename, button_template, output_dir_path):
    """
    Write HTML with multiple figures, where only one is visible at a time.
    Includes buttons to toggle visibility of figures by their names.
    
    Args:
        figures (dict): A dictionary where keys are figure names and values are Plotly figure objects.
        filename (str): The name of the output HTML file.
        button_template (str): HTML template for additional buttons or elements.
        output_dir_path (str): The directory path to save the HTML file.
    """
    # Generate HTML for each figure and hide all except the first one
    figure_html = ""
    for idx, (name, fig) in enumerate(figures.items()):
        fig_html = fig.to_html(include_plotlyjs=False, full_html=False)
        display_style = "block" if idx == 0 else "none"
        figure_html += f'<div id="{name}" style="display: {display_style};">{fig_html}</div>'

    # Create buttons to toggle visibility of figures
    button_html = '<div class="figure-buttons">'
    for name in figures.keys():
        button_html += f'<button onclick="showFigure(\'{name}\')">{name}</button>'
    button_html += "</div>"

    # Add JavaScript for toggling figures
    script = """
    <script>
        function showFigure(name) {
            const figures = document.querySelectorAll('div[id]');
            figures.forEach(fig => {
                fig.style.display = fig.id === name ? 'block' : 'none';
            });
        }
    </script>
    """

    # Combine everything into the final HTML
    final_html = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
    </head>
    <body>
        {button_template}
        {button_html}
        {figure_html}
        {script}
    </body>
    </html>
    """

    # Write the HTML to the specified file
    with open(os.path.join(output_dir_path, filename), "w") as f:
        f.write(final_html)

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
    dropoff_points = [(point["lat"], point["lon"]) for point in env["dropoff_coords"]]
    parking_points = [(point["lat"], point["lon"]) for point in env["parking_coords"]]

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
                dropoff_coord = assignment["dropoff_coord"]
                dropoff_lat = dropoff_coord["lat"]
                dropoff_lon = dropoff_coord["lon"]
                dropoff_assignment_points.append([dropoff_lat, dropoff_lon, 1.0])
                
                parking_coord = assignment["parking_coord"]
                parking_lat = parking_coord["lat"]
                parking_lon = parking_coord["lon"]
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
        lat = point["lat"]
        lon = point["lon"]
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

def arr_has_duplicate(arr):
    arr.sort()
    prev_el = None
    for el in arr:
        if el == prev_el:
            return True
        prev_el = el
        
    return False

def add_non_duplicate(arr, el):
    try:
        arr.index(el)
    except:
        arr.append(el)
    return arr

def extract_results_object(json_files, unused_metrics):
    """Create and object with the results of all the configurations as an object nested for every metric"""

    metrics = {}
    results = {}
    result_cats = {}
    for json_file in json_files:
        temp_result = results
        data = load_results(json_file)
        
        settings = data.get("settings")
        # Exclude specific entries
        excluded_keys = {"settings", "traces"}
        result_cats = {key: data.get(key) for key in data if key not in excluded_keys}
        

        for setting in settings:
            if setting not in unused_metrics:
                metrics[setting.replace('_', ' ').capitalize()] = settings.get(setting)

        metric_keys = list(metrics.keys())
        key_amount = len(metric_keys)
        for i in range(0, key_amount):
            if "metric" not in temp_result:
                temp_result["metric"] = metric_keys[i]

            metric_val = str(metrics[metric_keys[i]]) 

            if i == (key_amount - 1):
                result = {"metric": "result"}
                for key in result_cats:
                    result[key] =  result_cats[key]
                temp_result[metric_val] = result

                continue

            if metric_val not in temp_result:
                temp_result[metric_val] = {}
            
            temp_result = temp_result[metric_val]

    return results, list(result_cats.keys())

def get_results_list_one_metric(results, metric, other_metric_values, metric1_keys):
    out_list = []

    anchor = results
    anchor_idx = 0
    idx = 0

    metric_keys_idx = 0

    last_val = False

    while True:
        if results["metric"] == metric:
            anchor_idx = idx
            anchor = results

            try:
                results = results[metric1_keys[metric_keys_idx]]
            except:
                results = np.nan
              
            metric_keys_idx += 1
            if metric_keys_idx == len(metric1_keys):
                last_val = True

        else:
            try:
                results = results[other_metric_values[idx]]
            except KeyError as e:
                results = np.nan
            idx += 1

        if results["metric"] == "result" :
            out_list.append(results)
            results = anchor
            idx = anchor_idx
            if last_val:
                break

    return out_list

def add_latex_bar_chart_to(file_path, x, y, title, x_title, y_title):
    """
    Generates LaTeX code for a bar chart and appends it to the specified file.

    Args:
        file_path (str): The path to the file where the LaTeX code will be appended.
        x (list): The x-axis values (numbers).
        y (list): The heights of the bars corresponding to each x-axis value.
        title (str): The title of the chart.
        x_title (str): The label for the x-axis.
        y_title (str): The label for the y-axis.
    """
    string = f"""
----------------------------------------------------------
{title}       

\\begin{{tikzpicture}}
\\begin{{axis}}[
    title={{{title}}},
    xlabel={{{x_title}}},
    ylabel={{{y_title}}},
    xtick=data,
    ybar,
    ymin=0,
    bar width=0.6cm,
    nodes near coords,
    every node near coord/.append style={{font=\\small}},
    enlarge x limits=0.15,
    width=\\linewidth,
    height=0.6\\linewidth
]

\\addplot coordinates {{
"""
    for label, value in zip(x, y):
        string += f"    ({label}, {value})\n"

    string += """
};

\\end{axis}
\\end{tikzpicture}
"""

    with open(file_path, "a") as f:
        f.write(string)

def add_latex_contour_graph_to(file_path, x, y, z, title, x_title, y_title):
    x_len = len(x)
    y_len = len(y)
    string = f"""
----------------------------------------------------------
{title}       

\\begin{{tikzpicture}}
\\begin{{axis}}[
    title={{{title}}},
    xlabel={{{x_title}}},
    ylabel={{{y_title}}},
    width=\\columnwidth*0.9,
    view={{0}}{{90}},
    colormap/viridis,
    colorbar,
    colorbar style={{
        at={{(1.1,0.5)}}, 
        anchor=center, 
        width=0.2cm,
        yticklabel style={{font=\\small}}, 
    }},
    point meta min={min(map(max, z))},
    point meta max={max(map(max, z))}
]

\\addplot3[
    contour filled={{number=15}},
    mesh/rows={x_len},
    mesh/cols={y_len}
] table {{
    x        y        z
"""
    lines = []
    for i in range(x_len):
        for j in range(y_len):
            lines.append(f"\t{x[i]:<8} {y[j]:<8} {z[j][i]}")
    string += "\n".join(lines) + "\n"
    
    string += """
};

\\end{axis}
\\end{tikzpicture}
"""

    with open(file_path, "a") as f:
        f.write(string)

def create_bar_graph_html(results, result_cats,  output_dir_path):
    """Create line graph from results and metric object and save as html"""
    os.makedirs(output_dir_path, exist_ok=True)

    metrics = {}

    temp_results = results
    metric_keys = list(results.keys())

    latex_txt_output_path = output_dir_path / "latex_bar.txt"

    get_metrics(temp_results, metrics)

    for key in metrics:
        values = list(metrics[key])
        if values[0].isnumeric():
            metrics[key] = sorted(values, key=float)
        else:
            metrics[key] = values

    metrics[temp_results["metric"]] = metric_keys[1:]

    bar_results = {}

    metric_key_list = list(metrics.keys())

    for metric1 in metric_key_list:
        bar_results[metric1] = {"label": "x: " + metric1, "results": {}}

    for metric1 in metric_key_list:
        if len(metrics[metric1]) < 2:
            continue
        print(metric1 + " bar is starting...")
        metric1_idx = metric_key_list.index(metric1)

        remaining_metrics = metric_key_list.copy()
        remaining_metrics.pop(metric1_idx)

        remaining_keys = []
        for metric in remaining_metrics:
            remaining_keys.append(metrics[metric])

        remaining_cross_prod = itertools.product(*remaining_keys)

        all_result_lists = []

        for el in remaining_cross_prod:
            other_metrics_list = list(el)
         
            remaining_str = ""
            for i in range(0, len(remaining_metrics)):
                if len(metrics[remaining_metrics[i]]) > 1:
                    remaining_str += " | " + remaining_metrics[i] + ": " + el[i]
            
            # Figure out how to handle multple results
            result = get_results_list_one_metric(results, metric1, other_metrics_list,  metrics[metric1])

            if (ENABLE_EXTRA_GRAPH_CONFIGS):
                bar_results[metric1]["results"][remaining_str] = result
            all_result_lists.append(result)

        for cat in result_cats:
            #temp_all_results_lists = 
            bar_results[metric1]["results"][cat][" | Average"] = np.nanmean(np.array(all_result_lists), axis=0)
        print(metric1 + " bar is now done!")
        
    fig = go.Figure()

    updatemenus = [
        {
            "buttons": [],
            "direction": "down",
            "showactive": True,
        }
    ]

    default_x, default_y = None, None   
    results = bar_results

    for metric1 in results:
        config = results[metric1]
        for inner_key in config["results"]:
            x = metrics[metric1]
            if len(x) < 2:
                continue

            y = config["results"][inner_key]

            if default_x is None and default_y is None:
                with open(latex_txt_output_path, "w") as f:
                    f.write("")
                default_x, default_y = x, y

            title =  "x: " + metric1 + inner_key

            add_latex_bar_chart_to(latex_txt_output_path, x, y, title, metric, "results")

            updatemenus[0]["buttons"].append(
                {
                    "label": title,
                    "method": "update",
                    "args": [
                        {"x": [x], 
                         "y": [y]},
                        {"xaxis": {"title": metric1}},
                    ],
                }
            )


    if default_x is not None and default_y is not None:
        bar_fig = px.bar(
            x=default_x,
            y=default_y
        )
        for trace in bar_fig.data:
            fig.add_trace(trace)
    else: 
        return

    fig.update_layout(
        updatemenus=updatemenus
    )

    try:
        button_template_path = Path(__file__).parent / "index_graph_button_template.html"
        with open(button_template_path, "r") as f:
            button_template = f.read()
            button_template = button_template.replace("{{BUTTON_HREF}}", "latex_bar.txt")
            button_template = button_template.replace("{{DOWNLOAD_FILENAME}}", "latex_graph_bar.txt")

    except Exception as e:
        print(f"Error loading button template: {e}", file=sys.stderr)
        sys.exit(1)

    write_html_with_button(fig, "bar_graph.html", button_template, output_dir_path)

def get_metrics(results, metrics):
    if type(results) in (int, float, complex):
        return

    metric = results["metric"]
    metric_keys = list(results.keys())
    values = metric_keys[1:]
    
    if metric != "result":
        if metric not in metrics:
            metrics[metric] = set(values)
        else:
            metrics[metric].update(values) 

    for key in values:
        get_metrics(results[key], metrics)

def create_contour_graph_html(results, result_cats, output_dir_path):
    """Create contour graph from results and metric object and save as html"""
    os.makedirs(output_dir_path, exist_ok=True)

    metrics = {}

    temp_results = results

    latex_txt_output_path = output_dir_path / "latex_contour.txt"

    get_metrics(temp_results, metrics)

    for key in metrics:
        values = list(metrics[key])
        if values[0].isnumeric():
            metrics[key] = sorted(values, key=float)
        else: 
            metrics[key] = values

    contour_results = {}

    metric_key_list = list(metrics.keys())

    for metric1 in metric_key_list:
        inner_results = {}
        for metric2 in metric_key_list:
            if metric1 == metric2:
                continue
            inner_results[metric2] = {"label": "x: " + metric1 + " | y: "  + metric2, "results": {}}

        contour_results[metric1] = inner_results

    for metric1 in metric_key_list:
        for metric2 in metric_key_list:
            if metric1 == metric2 or len(metrics[metric1]) < 2 or len(metrics[metric2]) < 2:
                continue
            print(metric1 + " " + metric2+ " contour is starting...")

            metric1_idx = metric_key_list.index(metric1)
            metric2_idx = metric_key_list.index(metric2)

            remaining_metrics = metric_key_list.copy()
            remaining_metrics.pop(metric1_idx)
            remaining_metrics.pop(remaining_metrics.index(metric2))

            remaining_keys = []

            for metric in remaining_metrics:
                remaining_keys.append(metrics[metric])

            remaining_cross_prod = itertools.product(*remaining_keys)

            all_result_lists = []
            average_results_list = {}

            for el in remaining_cross_prod:
                el = list(el)
                result_list = []

                remaining_str = ""
                for i in range(0, len(remaining_metrics)):
                    if len(metrics[remaining_metrics[i]]) > 1:
                        remaining_str += " | " + remaining_metrics[i] + ": " + el[i]

                for key in metrics[metric2]:
                    temp_idx = metric2_idx if metric2_idx < metric1_idx else metric2_idx - 1
                    other_metrics_list = el[:temp_idx] + [key] + el[temp_idx:]
                    result_list.append(get_results_list_one_metric(results, metric1, other_metrics_list, metrics[metric1]))

                if (ENABLE_EXTRA_GRAPH_CONFIGS):
                    temp_result_lists = [entry[cat] for entry in result_list]
                    contour_results[metric1][metric2]["results"][remaining_str] = temp_result_lists

                all_result_lists.append(result_list)

                for cat in result_cats:
                    temp_all_results_lists = [[[res[cat] for res in result_lists] for result_lists in row] for row in all_result_lists]
                    average_results_list[cat] = np.nanmean(np.array(temp_all_results_lists), axis=0)

            contour_results[metric1][metric2]["results"][" | Average"] = average_results_list
            print(metric1 + " " + metric2 + " contour is now done!")
        
    results = contour_results

    fig = go.Figure()

    dropdowns =  {}
    

    default_x, default_y, default_z = None, None, None

    for idx, cat in enumerate(result_cats):
        dropdowns[cat] = dict(
            buttons=[],
            direction="down",
            showactive=True,
            y=(idx * 0.12) + 0.05,
        )

    ## make butto for each categori of results
    for metric1 in results:
        config_temp = results[metric1]
        for metric2 in config_temp:
            config = config_temp[metric2]
            for cat in result_cats:
                for inner_key in config["results"]:
                    x = metrics[metric1]
                    y = metrics[metric2]
                    if len(x) < 2 or len(y) < 2:
                        continue

                    z = config["results"][inner_key][cat]


                    if default_x is None and default_y is None and default_z is None:
                        with open(latex_txt_output_path, "w") as f:
                            f.write("")
                        default_x, default_y, default_z = x, y, z

                    title = config["label"] + inner_key
                    add_latex_contour_graph_to(latex_txt_output_path, x, y, z, title, metric1, metric2)

                    dropdowns[cat]["buttons"].append(
                        {
                            "label": title,
                            "method": "update",
                            "args": [
                                {"x": [x], "y": [y], "z": [z]},
                                {"xaxis": {"title": metric1}, "yaxis": {"title": metric2}},
                            ],
                        }
                    )

    if default_x is not None and default_y is not None and default_z is not None:
        fig.add_trace(
            go.Contour(
                z=default_z,
                x=default_x,
                y=default_y,
                colorscale="Viridis",
            )
        )
    else: 
        return

    fig.update_layout(updatemenus=list(dropdowns.values()))

    texts = {}
    for idx, cat in enumerate(result_cats):
        texts[cat] = dict(
            y=(idx * 0.12) + 0.07,
            yref="paper",
            x=-0.30,
            xref="paper",
            text=f"Select {cat} Options",
            align="left", 
            showarrow=False
        )

    fig.update_layout(annotations=list(texts.values()))

    try:
        button_template_path = Path(__file__).parent / "index_graph_button_template.html"
        with open(button_template_path, "r") as f:
            button_template = f.read()
            button_template = button_template.replace("{{BUTTON_HREF}}", "latex_contour.txt")
            button_template = button_template.replace("{{DOWNLOAD_FILENAME}}", "latex_graph_contour.txt")

    except Exception as e:
        print(f"Error loading button template: {e}", file=sys.stderr)
        sys.exit(1)
   
    write_html_with_button(fig, "contour_graph.html", button_template, output_dir_path)

def create_experiment_html(env, data, output_dir_path, experiment_name="", result_file="", single_file=False):
    """Create html from simulation data and save to experiment directory."""
    os.makedirs(output_dir_path, exist_ok=True)
    
    map_html_link = create_map_visualization(env, data, output_dir_path)
    
    all_traces = data.get("traces", [])
    num_runs = len(all_traces)
    
    metrics = {
        "available_parking_spots": {"title": "Available Parking Spots Over Time", "y_label": "# available parking spots"},
        "number_of_ongoing_simulations": {"title": "Number of Ongoing Simulations Over Time", "y_label": "# simulations"},
        "average_cost": {"title": "Average Cost Over Time", "y_label": "cost"},
        "average_duration": {"title": "Average Duration Over Time", "y_label": "average duration"},
        "var_count": {"title": "Variable Count Over Time", "y_label": "# variables"},
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
    write_html_with_button(figures["average_cost"], "cost.html", button_template, output_dir_path)
    write_html_with_button(figures["average_duration"], "duration.html", button_template, output_dir_path)
    write_html_with_button(figures["var_count"], "var_count.html", button_template, output_dir_path)
    write_html_with_button(figures["dropped_requests"], "dropped_requests.html", button_template, output_dir_path)
    
    # Settings
    settings = data.get("settings", {})
    timesteps = settings.get("timesteps", "N/A")
    start_time_raw = settings.get("start_time", "N/A")
    start_time = format_minutes_to_time(start_time_raw)
    
    max_request_duration_raw = settings.get("max_request_duration", "N/A")
    max_request_duration = f"{max_request_duration_raw} min" if max_request_duration_raw != "N/A" else "N/A"

    max_request_arrival_raw = settings.get("max_request_arrival", "N/A")
    max_request_arrival = f"{max_request_arrival_raw} min" if max_request_arrival_raw != "N/A" else "N/A"
    
    min_parking_time_raw = settings.get("min_parking_time", "N/A")
    min_parking_time = f"{min_parking_time_raw} min" if min_parking_time_raw != "N/A" else "N/A"
    
    request_rate = settings.get("request_rate", "N/A")
    
    batch_interval_raw = settings.get("batch_interval", "N/A")
    batch_interval = f"{batch_interval_raw} min" if batch_interval_raw != "N/A" else "N/A"
    
    using_weighted_parking = settings.get("using_weighted_parking", "N/A")
    
    random_generator = settings.get("random_generator", "N/A")
    seed = settings.get("seed", "N/A")

    # Stats
    total_dropped = data.get("total_dropped_requests", "N/A")
    avg_duration = format_duration_min_sec(data.get("avg_duration", 0))
    avg_cost = round(data.get("avg_cost", 0), 2)
    avg_var_count = round(data.get("avg_var_count", 0), 2)

    requests_generated = data.get("requests_generated", "N/A")
    requests_scheduled = data.get("requests_scheduled", "N/A")
    requests_unassigned = data.get("requests_unassigned", "N/A")
    
    time_elapsed_raw = data.get("time_elapsed", "N/A")
    time_elapsed = f"{time_elapsed_raw} ms" if time_elapsed_raw != "N/A" else "N/A"
    
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
                    dropoff = assignment.get("dropoff_coord", {})
                    parking = assignment.get("parking_coord", {})
                    request_duration = assignment.get("req_duration", "N/A")
                    route_duration = assignment.get("route_duration", "N/A")
                    
                    dropoff_lat = dropoff.get("lat", "N/A")
                    dropoff_lon = dropoff.get("lon", "N/A")
                    parking_lat = parking.get("lat", "N/A")
                    parking_lon = parking.get("lon", "N/A")
                    
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
    html_content = html_content.replace("{{random_generator}}", str(random_generator))
    html_content = html_content.replace("{{seed}}", str(seed))
    html_content = html_content.replace("{{total_dropped}}", str(total_dropped))
    html_content = html_content.replace("{{avg_duration}}", avg_duration)
    html_content = html_content.replace("{{avg_cost}}", str(avg_cost))
    html_content = html_content.replace("{{avg_var_count}}", str(avg_var_count))
    html_content = html_content.replace("{{requests_generated}}", str(requests_generated))
    html_content = html_content.replace("{{requests_scheduled}}", str(requests_scheduled))
    html_content = html_content.replace("{{requests_unassigned}}", str(requests_unassigned))
    html_content = html_content.replace("{{time_elapsed}}", str(time_elapsed))
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
                for line in summary_lines[:19]:  # Show first 19 lines
                    experiments_html += line
                experiments_html += "</pre></div>"
        
        json_files = sorted(glob.glob(os.path.join(exp_dir, "*.json")))
        
        if json_files:
            experiments_html += '<div class="run-grid">'
           
            experiments_html += f"""
                <div class="config-item">
                    <a href="{exp_name}/contour_graph.html" class="config-link">
                        <div class="config-name">{exp_name.capitalize()} Contour Graph</div>
                    </a>
                </div>
                """
            experiments_html += f"""
                <div class="config-item">
                    <a href="{exp_name}/bar_graph.html" class="config-link">
                        <div class="config-name">{exp_name.capitalize()} Bar Graph</div>
                    </a>
                </div>
                """

            experiments_html += "</div>"
            
            best_config, best_cost, dropped_in_best = find_best_config(json_files)
            experiments_html += f"""
                <div>
                    <h3>Best Configuration</h3>
                    <p>Total Dropped Requests: <strong>{dropped_in_best}</strong></p>
                    <p>Config: <strong>{best_config}</strong></p>
                    <p>Average Cost: <strong>{best_cost:.2f}</strong></p>
                </div>
                """
            
            experiments_html += '<h3>Configurations</h3><div class="run-grid">'
            
            for json_file in json_files:
                config_name = os.path.basename(json_file).replace(".json", "")
                
                duration = "Unknown"
                rate = "Unknown"
                arrival = "Unknown"
                min_parking_time = "Unknown"
                commit = "Unknown"
                if config_name.startswith("d") and "-A" in config_name and "-m" in config_name and "-r" in config_name and  "-c" in config_name:
                    delimiters = ["-A", "-m", "-r", "-c"]
                    temp_config_name = config_name
                    for delimiter in delimiters:
                            temp_config_name = " ".join(temp_config_name.split(delimiter))
                    parts = temp_config_name.split()
                    if len(parts) == 5:
                        duration = parts[0][1:]  # Remove the "d" prefix
                        arrival = parts[1]
                        min_parking_time = parts[2]
                        rate = parts[3]
                        commit = parts[4]

                exp_config_dir = f"{exp_name}_{config_name}"
                
                experiments_html += f"""
                <div class="config-item">
                    <a href="{exp_config_dir}/experiment.html" class="config-link">
                        <div class="config-name">{config_name}</div>
                        <div class="config-details">
                            <div class="config-detail">Duration: {duration} min</div>
                            <div class="config-detail">Early Arrival: {arrival} min</div>
                            <div class="config-detail">Min Parking Time: {min_parking_time} min</div>
                            <div class="config-detail">Rate: {rate}</div>
                            <div class="config-detail">Commit Interval: {commit} min</div>
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

def find_best_config(json_files):
    best_config = None
    best_cost = float("inf")
    
    for json_file in json_files:
        config_name = os.path.basename(json_file).replace(".json", "")
        try:
            with open(json_file, 'r') as f:
                data = json.load(f)
                cost = float(data["avg_cost"])
                if cost < best_cost:
                    best_cost = cost
                    best_config = config_name
                    dropped_in_best = data.get("total_dropped_requests", 0)
        except (json.JSONDecodeError, ValueError) as e:
            print(f"Error reading {json_file}: {e}")
            continue
    return best_config, best_cost, dropped_in_best
    
def process_experiments(env, experiments_dir, experiment_list=None):
    """Process all experiments in the directory"""
    report_root = Path("report")
    os.makedirs(report_root, exist_ok=True)
    
    exp_dirs = sorted(glob.glob(os.path.join(experiments_dir, "experiment-*")))
    if experiment_list:
        exp_dirs = [exp_dir for exp_dir in exp_dirs if exp_dir in experiment_list]
    
    if not exp_dirs:
        print(f"No experiment directories found in {experiments_dir}")
        return
    
    # Process each experiment directory
    for exp_dir in exp_dirs:
        exp_name = os.path.basename(exp_dir)
        exp_name_display = exp_name.replace("-", " ").title()
        
        json_files = sorted(glob.glob(os.path.join(exp_dir, "*.json")))
        
        results, result_cats = extract_results_object(json_files, UNUSED_SETTINGS)
        #create_bar_graph_html(results, result_cats, report_root / exp_name)
        create_contour_graph_html(results, result_cats, report_root / exp_name)

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
    parser.add_argument("--experiments", nargs="*", help="List of specific experiments to process")
    args = parser.parse_args()
    
    env = load_env(args.env_file)
    
    # Check if the results argument is a directory or a file
    if os.path.isdir(args.results):
        process_experiments(env, args.results, args.experiments)
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