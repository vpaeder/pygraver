#!/usr/bin/python
'''
This example shows how to send paths generated
in example1 to a machine.
'''

from example1 import *
from pygraver.machine import SyncMachine

# simplify paths and add ramps
pg = pg.simplify_above(limit_height=0.5).create_ramps(limit_height=0.5, ramp_height=1.0, ramp_length=1.0, ramp_direction=types.RampDirection.Both).cartesian
# create machine instance (synchronous version)
machine = SyncMachine("/dev/path_to_port")
# open machine
if machine.open():
    # send each pattern sequentially
    for p in pg:
        machine.trace_pattern(p)
    # wait for machine to finish
    machine.wait()
    # close connection
    machine.close()
