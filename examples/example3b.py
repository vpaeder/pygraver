#!/usr/bin/python
'''
This is a variation of example3 with paths corrected
in such a way to remove points outside surface
and add insertion and extraction ramps.
'''

from example1 import *
from pygraver.core import render

# create render model
model = render.Model()
# set model background color to white
model.background_color = [255, 255, 255]
# create wires from path group
pg = pg.simplify_above(limit_height=0.5).create_ramps(limit_height=0.5, ramp_height=1.0, ramp_length=1.0, ramp_direction=types.RampDirection.Both)
wires = render.WireCollection(pg.paths, diameter=0.2, color=[0, 0, 0, 255], sides=4)
# set to scalar color mode; this displays shape with position-dependent color
wires.scalar_color_mode = True
# adjust color range to increase contrast
wires.set_scalar_color_range(vmin=-0.5, vmax=1.0)
# add shape to model
model.add_shape(wires)
# create extrusion of surface for display purpose
extrusion = render.Extrusion(surface, length=1.0, axis=types.Point(0,0,-1,0), color=[192, 192, 192, 255])
# add shape to model
model.add_shape(extrusion)
# render model
model.render()
