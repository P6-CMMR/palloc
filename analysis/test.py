import plotly.graph_objects as go 
import numpy as np

def linear_function(a,b,x):
    return a*x+b

x = np.arange(-10,10,0.1)

a_range = np.arange(-2, 2, 0.1)
a_index = 11
a0 = a_range[a_index]
b_range = np.arange(-1, 1, 0.1)

traces = [go.Scatter(x=x,y=linear_function(a=a0,b=b,x=x),visible=False) for b in b_range]
b_index = 11
b0 = b_range[b_index]
traces[b_index].visible = True

steps_b = [dict(label=b, method="restyle", args=[{"visible": [False]*len(b_range)}]) for b in b_range]
for i, _ in enumerate(b_range):
    steps_b[i]["args"][0]["visible"][i] = True

steps_a =[ dict(label = a, method = "restyle", args = [{"y" : [linear_function(a,b,x) for b in b_range]}])
        for a in a_range]


sliders = [dict(active=a_index, steps=steps_a, currentvalue={"prefix": "a="}),
           dict(active=b_index, steps=steps_b, currentvalue={"prefix": "b="}, y=-.15)]

fig = go.FigureWidget(data=traces)
fig.update_yaxes(autorange=False,range=(-3,3))
fig.update_layout(sliders=sliders)

# Save the graph as an HTML file
fig.write_html("/home/vind/P6/palloc/analysis/linear_function_graph.html")

fig.show()