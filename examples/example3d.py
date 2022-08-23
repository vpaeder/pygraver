#!/usr/bin/python
'''
This example is a variation of example3
which demonstrates the use of widgets.
'''

from example1 import *
from pygraver.core import render
from pygraver.render import TextButton, BalloonText

# create render model
model = render.Model()
# set model background color to white
model.background_color = [255, 255, 255]
# create wires from path group
wires = render.WireCollection(pg, diameter=0.2, color=[0, 0, 0, 255], sides=4)
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
# add labels to shapes, otherwise text buttons won't display anything
wires.label = "paths"
extrusion.label = "base"
# create button widgets for shapes
model.add_widget(TextButton(wires))
model.add_widget(TextButton(extrusion))
# create balloon text for wires
model.add_widget(BalloonText(wires))
# render model
model.render()
