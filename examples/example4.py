#!/usr/bin/python
'''
This example shows how to render a model
remotely using trame.
'''

from example1 import *
from pygraver.core import render
from pygraver.web import WebLayout
from trame.app import get_server

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
# get trame server
server = get_server()
# create layout
with WebLayout(server, model.window) as layout:
    # one can place here layout elements as demonstrated for instance in [trame tutorial](https://kitware.github.io/trame/docs/tutorial.html)
    pass

# reset camera to display entire model
model.renderer.ResetCamera()
# start server on http://localhost:8080 and open a browser window
server.start()
