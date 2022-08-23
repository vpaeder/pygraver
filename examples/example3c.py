#!/usr/bin/python
'''
This is a variation of example3 that shows how to
dynamically update a model using the default renderer.
It takes the paths rendered in example3 and add them
to the model with >=1.0 second interval.
'''

from example1 import *
from pygraver.core import render
from pygraver.render import DynamicModel, StyledPath
from time import sleep, time

# define model class with update loop; this is an old school way of writing an update loop, but
# it is necessary with default VTK renderer as it doesn't play well with asyncio or threading;
# these may be used together with another renderer (e.g. Qt, wx, trame)
class CustomModel(DynamicModel):
    n = 0 # path counter
    last_t = 0 # time tracker
    
    def timer_callback(self, **kwargs):
        if time() - self.last_t > 1.0 and self.n<72:
            p = paths[self.n]
            # create styled path associated with model; StyledPath objects
            # can be handled just like a normal Path, and the associated model
            # is also automatically updated
            paths[self.n] = StyledPath(p.xs, p.ys, p.zs, p.cs, model=self, tool_size=0.2)
            # set to scalar color mode; this displays shape with position-dependent color
            paths[self.n].shape.scalar_color_mode = True
            # adjust color range to increase contrast
            paths[self.n].shape.set_scalar_color_range(vmin=-0.5, vmax=1.0)
            # call callback of DynamicModel class
            super().timer_callback()
            self.n += 1
            self.last_t = time()

# create render model
model = CustomModel()
# set model background color to white
model.background_color = [255, 255, 255]
# simplify paths and add ramps; these paths will be used in CustomModel.timer_callback
paths = [p.simplify_above(limit_height=0.5).create_ramps(limit_height=0.5, ramp_height=1.0, ramp_length=1.0, ramp_direction=types.RampDirection.Both) for p in pg.paths]
# create extrusion of surface for display purpose
extrusion = render.Extrusion(surface, length=1.0, axis=types.Point(0,0,-1,0), color=[192, 192, 192, 255])
# add shape to model
model.add_shape(extrusion)
# render model
model.render()

