import plotly.graph_objects as go 
import numpy as np

traces = [go.Contour(
    x=[1, 2, 3, 4],
    y=[1, 2, 3, 4],
    z=[[1, 2, 3, 4],
       [1, 2, 3, 4],
       [1, 2, 3, 4],
       [1, 2, 3, 4]]),
    go.Contour(
    x=[5, 6, 7, 8],
    y=[5, 6, 7, 8],
    z=[[8, 7, 6, 5],
       [8, 7, 6, 5],
       [8, 7, 6, 5],
       [8, 7, 6, 5]]),
    go.Contour(
    x=[9, 10, 11, 12],
    y=[9, 10, 11, 12],
    z=[[5, 6, 7, 8],
       [5, 6, 7, 8],
       [5, 6, 7, 8],
       [5, 6, 7, 8]]),
        go.Contour(
    x=[14, 15, 16, 17],
    y=[14, 15, 16, 17],
    z=[[4, 3, 2, 1],
       [4, 3, 2, 1],
       [4, 3, 2, 1],
       [4, 3, 2, 1]]),
       
]
a_range = np.arange(1, 2, 1)
b_range = np.arange(1, 2, 1)

steps_b = [dict(label=b, method="restyle", args=[{"visible": [False]*len(b_range)}]) for b in b_range]
for i, _ in enumerate(b_range):
    steps_b[i]["args"][0]["visible"][i] = True

steps_a =[ dict(label = a, method = "restyle", args = [{"y" : [linear_function(a,b,x) for b in b_range]}])
        for a in a_range]


sliders = [dict(active=1, steps=steps_a, currentvalue={"prefix": "a="}),
           dict(active=1, steps=steps_b, currentvalue={"prefix": "b="}, y=-.15)]

fig = go.FigureWidget(data=traces)
fig.update_yaxes(autorange=False,range=(-3,3))
fig.update_layout(sliders=sliders)

# Save the graph as an HTML file
fig.write_html("/home/vind/P6/palloc/analysis/linear_function_graph.html")

fig.show()