import plotly.graph_objects as go
import numpy as np

# Example data
data = {
    "length": np.linspace(1, 5, 10),
    "height": np.linspace(10, 50, 10),
    "width": np.linspace(100, 500, 10)
}

# Generate z-values for the contours
def generate_contour_data(x, y):
    X, Y = np.meshgrid(x, y)
    Z = np.sin(X) + np.cos(Y)  # Example function
    return X, Y, Z

# Default x, y, z
X, Y, Z = generate_contour_data(data["length"], data["height"])

# Create initial contour plot
fig = go.Figure(go.Contour(z=Z, x=data["length"], y=data["height"], contours_coloring='heatmap'))

# Dropdown menus for axes
x_axis_menu = [
    {
        "label": "Length",
        "method": "update",
        "args": [
            {"x": [data["length"]]},
            {"xaxis": {"title": "Length"}}
        ]
    },
    {
        "label": "Height",
        "method": "update",
        "args": [
            {"x": [data["height"]]},
            {"xaxis": {"title": "Height"}}
        ]
    },
    {
        "label": "Width",
        "method": "update",
        "args": [
            {"x": [data["width"]]},
            {"xaxis": {"title": "Width"}}
        ]
    }
]

y_axis_menu = [
    {
        "label": "Length",
        "method": "update",
        "args": [
            {"y": [data["length"]]},
            {"yaxis": {"title": "Length"}}
        ]
    },
    {
        "label": "Height",
        "method": "update",
        "args": [
            {"y": [data["height"]]},
            {"yaxis": {"title": "Height"}}
        ]
    },
    {
        "label": "Width",
        "method": "update",
        "args": [
            {"y": [data["width"]]},
            {"yaxis": {"title": "Width"}}
        ]
    }
]

z_axis_menu = [
    {
        "label": "Sin(X) + Cos(Y)",
        "method": "update",
        "args": [
            {"z": [generate_contour_data(data["length"], data["height"])[2]]},
            {}
        ]
    },
    {
        "label": "X * Y",
        "method": "update",
        "args": [
            {"z": [generate_contour_data(data["length"], data["height"])[0] * generate_contour_data(data["length"], data["height"])[1]]},
            {}
        ]
    }
]

# Add dropdowns to the layout
fig.update_layout(
    updatemenus=[
        {
            "buttons": x_axis_menu,
            "direction": "down",
            "showactive": True,
            "x": 0.1,
            "y": 1.15,
            "xanchor": "left",
            "yanchor": "top"
        },
        {
            "buttons": y_axis_menu,
            "direction": "down",
            "showactive": True,
            "x": 0.3,
            "y": 1.15,
            "xanchor": "left",
            "yanchor": "top"
        },
        {
            "buttons": z_axis_menu,
            "direction": "down",
            "showactive": True,
            "x": 0.5,
            "y": 1.15,
            "xanchor": "left",
            "yanchor": "top"
        }
    ]
)

# Initial axis titles
fig.update_layout(
    xaxis_title="Length",
    yaxis_title="Height"
)

fig.show()
