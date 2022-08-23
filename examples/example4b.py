#!/usr/bin/python
'''
This example is a variation of example4.py
which shows how to dynamically update a model
rendered remotely using trame. It adds one path
every second to the model.
'''

from example1 import *
from pygraver.core import render
from pygraver.web import WebLayout, asynchronous
from trame.app import get_server
import asyncio

# create render model
model = render.Model()
# set model background color to white
model.background_color = [255, 255, 255]
# create extrusion of surface
extrusion = render.Extrusion(surface, length=1.0, axis=types.Point(0,0,-1,0), color=[192, 192, 192, 255])
# add shape to model
model.add_shape(extrusion)
# get trame server
server = get_server()
# create layout class with dynamic updates
class DynamicLayout(WebLayout):
    @asynchronous.task
    async def refresh_function(self, **kwargs):
        for n in range(72):
            # wait for 1 second
            await asyncio.sleep(1.0)
            # create wire from n-th path of path group
            wire = render.Wire(pg[n], diameter=0.2, color=[0, 0, 0, 255], sides=4)
            # set to scalar color mode; this displays shape with position-dependent color
            wire.scalar_color_mode = True
            # adjust color range to increase contrast
            wire.set_scalar_color_range(vmin=-0.5, vmax=1.0)
            # add new wire to model
            model.add_shape(wire)
            # refresh layout
            self.refresh()

# create layout
with DynamicLayout(server, model.window) as layout:
    pass

# reset camera to display entire model
model.renderer.ResetCamera()
# start server on http://localhost:8080 and open a browser window
server.start()
