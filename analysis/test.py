import numpy as np
import plotly.graph_objects as go

# Define your variables
heights = [1, 2, 3, 4]  # Rows (e.g., heights)
lengths = [1, 2, 3, 4, 5]  # Columns (e.g., lengths)

# Define values for each (height, length) combination
values = [
    [5, 2, 1, 3, 4],  # height = 1
    [4, 3, 2, 5, 6],  # height = 2
    [7, 6, 5, 4, 3],  # height = 3
    [8, 7, 6, 5, 4]   # height = 4
]

# Create a contour plot
fig = go.Figure()

fig.add_trace(
    go.Contour(
        z=values,  # Data values
        x=lengths,  # Lengths (X-axis)
        y=heights,  # Heights (Y-axis)
        colorscale="Viridis",
        contours=dict(showlabels=True),  # Add labels to the contours
    )
)

# Add titles and labels
fig.update_layout(
    title="Contour Plot of Length and Height Combinations",
    xaxis_title="Length",
    yaxis_title="Height"
)

fig.show()
