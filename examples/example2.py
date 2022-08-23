#!/usr/bin/python
'''
This example extends example 1 and shows
how to display result with Matplotlib.
'''

from example1 import *
from matplotlib import pyplot as plt

# plot surface contour
plt.plot(*surface.contours[0].xy.T)
# plot hole contour
plt.plot(*surface.holes[0].xy.T)
# plot paths
for p in pg.paths:
    # split paths in order to only keep points within surface; split_above splits paths every time
    # they cross z=0.5, and discards every point above this value
    for q in p.split_above(limit_height=0.5):
        plt.plot(*q.xy.T, "k", linewidth=0.5)
# set plot ratio
plt.axis("equal")
# flip vertical axis to make it look like in Inkscape
plt.axis([-20,20,20,-20])
# render
plt.show()
