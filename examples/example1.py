#!/usr/bin/python
'''
This example demonstrates the following:
 - load shapes from a SVG file
 - create Surface, Path and PathGroup objects
 - arrange paths in path group and mask them with surface
'''

from pygraver.core import types, svg
import numpy as np

# open SVG file for parsing
f = svg.File("test.svg")
# rasterize shape contours from layer 'Shapes' and holes from layer 'Holes'
contours = f.get_paths(layer="Shapes", step_size=0.1)
holes = f.get_paths("Holes", 0.1)
# create surface from loaded paths
surface = types.Surface(contours, holes)

# create base path: wavy line in y direction with increasing amplitude
ts = np.linspace(0, 2*np.pi, 101)
xs = np.linspace(0.3, 1.5, 101)*np.cos(5*ts)
ys = np.linspace(0, 19, 101)
# create path group with 72 copies of base path
pg = types.PathGroup([types.Path(xs,ys) for n in range(72)])
# define steps between paths: 3 degree rotation
pg.steps = [types.Point(x=0, y=0, z=0, c=3) for n in range(71)]
# set paths height outside 
pg.paths = surface.correct_height(pg, clearance=0, safe_height=1.0, outside=True, fix_contours=True)
