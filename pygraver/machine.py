# -*- coding: utf-8 -*-
'''
Machine handler submodule.

This submodule provides classes to handle the motion of a 4-axis CNC engraving machine.
Rotation is assumed to occur in the x-y plane (perpendicular to tool holder/spindle).
The code has been tested with two firmwares:
    - Marlin (https://github.com/MarlinFirmware/Marlin),
    - Duet3D (RepRap, https://www.duet3d.com/)
but other firmwares may work too. A useful comparison of functionalities found
in the RepRap wiki (http://reprap.org/wiki/G-code) may help determine compatibility.
'''

import asyncio
import serial_asyncio
from serial import SerialException
from .core import types, render
from .render import StyledPath
from .exceptions import *
import re
import logging


class Machine(object):
    '''
    Machine handling class.
    
    Most methods are asynchronous. For a synchronous version,
    use the SyncMachine class instead.
    
    Attributes:
        port (str): logical serial port path
        ser (serial.Serial or None): serial connection object, if created, or None
        history (list): list of every movement since object creation
        (see display_history for informations on history entries format)
    
    Note:
        This class assumes that the machine has 3 linear axes (x,y,z) and one rotary axis perpendicular
        to the x-y plane (i.e. rotation occurs in the x-y plane).
    '''
    _axes = {"X":"x", "Y":"y", "Z":"z", "C":"c"}
    _term_char = '\n'
    _response_ok = "ok"
    _serial_baud_rate = 115200
    _timeout = 1.0

    def __init__(self, port:str=""):
        '''
        Constructor.
        
        Args:
            port (str): logical serial port path
        '''
        # serial settings
        self._port = port
        self.__reader = None
        self.__writer = None
        # number of axes
        self._N_axes = len(self._axes)
        # machine settings
        self._feed_rate = 100.0
        self._endstops = True
        # history (for display purpose)
        self.history = [StyledPath()]
        self.history[-1].append(types.Point())
        self.__model = None

    def set_port(self, port:str) -> None:
        '''
        Set path to serial port.
        
        Args:
            port (str): logical serial port path
        
        Raises:
            ValueError: if port path is null.
            SerialException: if a serial connection is already open.
        '''
        if len(port)==0:
            raise ValueError("Port path cannot be null.")
        if self.__reader is not None or self.__writer is not None:
            raise SerialException("A serial connection is already open. It must be closed first.")
        self._port = port
    
    def get_port(self) -> str:
        '''
        Get path to serial port.
        
        Returns:
            str: logical serial port path
        '''
        return self._port

    port = property(get_port, set_port) 

    def set_baud_rate(self, baud_rate:int) -> None:
        '''
        Set baud rate for serial connection.
        
        Args:
            baud_rate (int): baud rate
        
        Raises:
            ValueError: if value is not strictly positive.
            SerialException: if a serial connection is already open.
        '''
        if self.__reader is not None or self.__writer is not None:
            raise SerialException("A serial connection is already open. It must be closed first.")
        if baud_rate<=0:
            raise ValueError("Baud rate must be strictly positive.")
        self._serial_baud_rate = baud_rate

    def get_baud_rate(self) -> int:
        '''
        Get baud rate for serial connection.
        
        Returns:
            int: baud rate
        '''
        return self._serial_baud_rate

    serial_baud_rate = property(get_baud_rate, set_baud_rate)

    def set_timeout(self, timeout:float|None) -> None:
        '''
        Set serial timeout.
        
        Args:
            timeout (float|None): timeout in seconds, or None for infinite.
        
        Raises:
            ValueError: if given value is negative.
        '''
        if timeout is not None and timeout<0:
            raise ValueError("Timeout must be positive.")
        self._timeout = timeout
    
    def get_timeout(self) -> float|None:
        '''
        Get serial timeout.
        
        Returns:
            float|None: serial timeout
        '''
        return self._timeout
    
    timeout = property(get_timeout, set_timeout)

    def set_term_char(self, term_char:str) -> None:
        '''
        Set termination character for serial communication.
        
        Args:
            term_char (str): termination character
        
        Raises:
            ValueError: if given value is negative.
        '''
        self._term_char = term_char
    
    def get_term_char(self) -> str:
        '''
        Get termination character for serial communication.
        
        Returns:
            str: termination character
        '''
        return self._term_char
    
    term_char = property(get_term_char, set_term_char)

    def set_response_ok(self, response:str) -> None:
        '''
        Set OK response string for serial communication.
        
        Args:
            response (str): response string
        
        Raises:
            ValueError: if string is empty.
        '''
        if len(response) == 0:
            raise ValueError("Response string cannot be empty.")
        
        self._response_ok = response
    
    def get_response_ok(self) -> str:
        '''
        Get OK response string for serial communication.
        
        Returns:
            str: response string
        '''
        return self._response_ok
    
    response_ok = property(get_response_ok, set_response_ok)

    def set_tool_size(self, tool_size:float) -> None:
        '''
        Set tool size.
        
        Args:
            tool_size (float): tool size, in relative units.
        
        Raises:
            ValueError: if given value is negative or zero.
        '''
        if tool_size<=0:
            raise ValueError("Tool size must be strictly positive.")
        if len(self.history[-1])>1:
            if self.history[-1].tool_size != tool_size:
                new_path = StyledPath()
                new_path.append(self.history[-1][-1])
                self.history.append(new_path)
        self.history[-1].tool_size = tool_size

    def get_tool_size(self) -> float:
        '''
        Get tool size.
        
        Returns:
            float: tool size
        '''
        return self.history[-1].tool_size

    tool_size = property(get_tool_size, set_tool_size)

    def set_feed_rate(self, feed_rate:float) -> None:
        '''
        Set feed rate.
        
        Args:
            feed_rate (float): feed rate, in relative units.
        
        Raises:
            ValueError: if given value is negative or zero.
        '''
        if feed_rate<=0:
            raise ValueError("Feed rate must be strictly positive.")
        self._feed_rate = feed_rate
    
    def get_feed_rate(self) -> float:
        '''
        Get feed rate.
        
        Returns:
            float: feed rate
        '''
        return self._feed_rate
    
    feed_rate = property(get_feed_rate, set_feed_rate)


    async def open(self) -> bool:
        '''
        Open serial connection.
        
        Returns:
            bool: True if connection was opened successfully, False otherwise 
        
        Raises:
            SerialException: if connection is already open.
            ValueError: if port path is null.
        '''
        if self.__reader is not None or self.__writer is not None:
            raise SerialException("A serial connection is already open. It must be closed first.")
        
        if len(self._port) == 0:
            raise ValueError("Invalid port (empty string).")

        self.__reader, self.__writer = await serial_asyncio.open_serial_connection(url=self._port, baudrate=self._serial_baud_rate)
        return True
    
    async def close(self, timeout:float|None=None) -> bool:
        '''
        Close serial connection.
        
        Returns:
            bool: True if connection was closed successfully, False otherwise
        '''
        if self.__writer is not None:
            self.__writer.close()
            await asyncio.wait_for(self.__writer.wait_closed(), timeout=timeout)
            self.__reader = None
            self.__writer = None
            return True
        return False
    

    async def write(self, cmd:str, timeout:float|None=None) -> None:
        '''
        Send a command to the machine.
        
        Args:
            cmd (str): command to be sent
        
        Raises:
            asyncio.TimeoutError: if timeout is reached
        '''
        # send command to instrument
        if self.__writer is None: return False
        timeout = self._timeout if timeout is None else timeout
        self.__writer.write("{cmd}{term}".format(cmd=cmd, term=self._term_char).encode("utf-8"))
        logging.info(cmd)
        await asyncio.wait_for(self.__writer.drain(), timeout)
    
    async def readline(self, timeout:float|None=None) -> bytes:
        '''
        Read one message line from machine.
        
        Args:
            timeout (Optional[float]): timeout, in seconds (default: 0.5)
        
        Returns:
            bytes: line content if a line could be read, None otherwise
        
        Raises:
            asyncio.TimeoutError: if timeout is reached
        '''
        if self.__reader is None: return None
        timeout = self._timeout if timeout is None else timeout
        return await asyncio.wait_for(self.__reader.readuntil(separator=self._term_char.encode("utf-8")), timeout=timeout)
        
    async def wait_answer(self, n_lines:int=1, timeout:float|None=None) -> 'list[bytes]':
        '''
        Wait for answer from machine.
        
        Args:
            n_lines (Optional[int]): expected number of lines for reply (default: 1).
            timeout (Optional[float]): timeout, in seconds (default: use default timeout)
        
        Returns:
            list[bytes]: replied message split line by line
            None: if an error occurred
        '''
        if self.__reader is None: return None
        ok_message = "{}{}".format(self._response_ok, self._term_char).encode("utf-8")
        msg = []
        while n_lines:
            try:
                msg.append(await self.readline(timeout))
            except asyncio.TimeoutError:
                return None
            n_lines -= 1
            if len(msg)>0 and msg[-1].endswith(ok_message):
                return msg
        
        return None
    
    async def ask(self, cmd:str, n_lines:int=1, timeout:float|None=None) -> 'list[bytes]':
        '''
        Send given command and wait for answer.
        
        Args:
            cmd (str): command to be sent
            n_lines (Optional[int]): expected number of lines for reply (default: 1).
            timeout (Optional[float]): timeout, in seconds (default: use default timeout)
        
        Returns:
            bool: True if movement was completed before timeout, False otherwise
        
        Raises:
            asyncio.TimeoutError: if command couldn't be sent to machine within timeout
        
        '''
        await self.write(cmd=cmd, timeout=timeout)
        return await self.wait_answer(n_lines=n_lines, timeout=timeout)
    
    async def wait(self, timeout:float|None=None) -> bool:
        '''
        Wait for a movement to reach a given position, or until timeout is reached.
        
        Args:
            timeout (Optional[float]): waiting timeout (default: no timeout)
        
        Returns:
            bool: True if movement was completed before timeout, False otherwise
        
        Raises:
            asyncio.TimeoutError: if command couldn't be sent to machine within timeout
        
        '''
        if self.__reader is None: return False
        await self.write(cmd="G4 S0", timeout=timeout)
        try:
            # we don't care about the content of the answer, as long as we get an answer
            # (which means that the machine has finished all the assigned tasks and is now ready)
            await asyncio.wait_for(self.__reader.readuntil(separator=self._term_char.encode("utf-8")), timeout=timeout)
        except asyncio.TimeoutError:
            return False
        return True
    
    async def get_position(self, timeout:float|None=None) -> types.Point:
        '''
        Read current position from machine.
        
        Notes:
            If no machine connection is open, the last history position is returned.
        
        Returns:
            Point: current position

        Raises:
            asyncio.TimeoutError: if machine didn't answer within timeout
        '''
        if self.__writer is None:
            if len(self.history)>0:
                return self.history[-1][-1]
            else:
                return types.Point()

        rep = await self.ask(cmd="M114", n_lines=2, timeout=timeout)

        if rep is None:
            raise asyncio.TimeoutError("Machine didn't answer within timeout.")

        pt = types.Point()
        for ax in self._axes:
            match = re.search("(?i)({}:(?: |)(?:-|)[0-9]{{1,8}}(?:\.|)[0-9]{{0,8}})".format(ax).encode("utf-8"), rep[0])
            if match is not None:
                setattr(pt, self._axes[ax], float(match.group(1).split(b":")[1]))
            else:
                setattr(pt, self._axes[ax], getattr(self.history[-1][-1], self._axes[ax]))
        
        return pt
    
    def _parse_position(self, **kwargs) -> dict:
        '''
        Parse position arguments and generate valid filtered dictionnary.
        
        Args:
            **kwargs (float): each argument must be named after one of the machine's axes.
        
        Returns:
            dict: filtered dictionnary containing only machine's axes

        Raises:
            asyncio.TimeoutError: if machine didn't answer within timeout
        '''
        new_pos = dict()
        for key in kwargs:
            if key in self._axes or key.upper() in self._axes:
                new_pos.update({key.upper():kwargs[key]})
        return new_pos
    
    async def set_position(self, position:types.Point|None=None, timeout:float|None=None, **kwargs) -> bool:
        '''
        Set current machine coordinates.
        
        Args:
            position (types.Point): coordinate in axis units
            timeout (Optional[float]): waiting timeout (default: no timeout)
            axis letter from self._axes (Optional[float]): coordinate in axis units
        
        Notes:
            1) This doesn't produce any movement
            2) axis letters are taken into account only if position isn't provided
        
        Returns:
            bool: True if successful, False otherwise
        '''
        if position is not None:
            return await self.set_position(x=position.x, y=position.y, z=position.z, c=position.c, timeout=timeout)
        
        new_pos = self._parse_position(**kwargs)
        if len(self.history)>0:
            for key in new_pos:
                setattr(self.history[-1][-1], self._axes[key], new_pos[key])
        
        if self.__writer is None or len(new_pos)==0:
            return False
        
        # sets coordinates to given values
        try:
            await self.ask(cmd=self._make_position_string("G92", new_pos), n_lines=1, timeout=timeout)
            return True
        except asyncio.TimeoutError:
            return False

    def _make_position_string(self, cmd:str, positions:dict) -> str:
        '''
        Compile a command with given positions as arguments.
        
        Args:
            cmd (str): G-code command.
            positions (float): dictionnary with axes as keys and positions as values.

        Returns:
            str: formatted command.
        '''
        return "{cmd} {args}".format(
            cmd=cmd,
            args=" ".join(["{key}{pos:f}".format(key=key, pos=positions[key]) for key in positions])
        )
    
    async def _set_position(self, x:list|tuple|dict|types.Point) -> None:
        '''
        Set machine coordinates from a vector or a dictionnary of values.
        This is a private method called while setting the 'position' property.
        
        Args:
            x (list, tuple, dict or types.Point): object containing the new coordinates. It must take the following form:
             - If list or tuple, must contain values for every axis defined in self._axes 
             - If dict, should contain at least one of the coordinates, e.g. {"x": value, "y": value, "z": ...}.
        
        Raises:
            TypeError: if the argument is not a tuple, list or dict
            IndexError: if the argument is of type tuple or list and not of size 4
        
        Returns:
            result from 'set_position' method
        '''
        if isinstance(x, list) or isinstance(x, tuple):
            if len(x)!=self._N_axes:
                raise IndexError("Argument, as a list or tuple, must be of size %d." % self._N_axes)
            return await self.set_position(**dict(zip(self._axes, x)))
        elif isinstance(x, dict):
            return await self.set_position(**x)
        elif isinstance(x, types.Point):
            return await self.set_position(position=x)
        else:
            raise TypeError("Argument must be a vector of length %d or a dictionnary with keys in %s." % (self._N_axes, self._axes))
        
    async def move(self, relative:bool=False, position:types.Point|None=None, timeout:float|None=None, **kwargs) -> bool:
        '''
        Produce absolute or relative linear movement from current point to given point.
        
        Args:
            relative (bool): if True, produce relative movement; if False, produce absolute movement.
            position (types.Point): coordinate in axis units
            timeout (float|None): timeout in seconds, or None for infinite.
            **kwargs (Optional[float]): coordinate values
        
        Returns:
            bool: True if the command was successful, False otherwise
        
        Raises:
            asyncio.TimeoutError: if command didn't return within timeout
        '''
        if position is not None:
            return await self.move(relative=relative, timeout=timeout, x=position.x, y=position.y, z=position.z, c=position.c)

        new_pos = self._parse_position(**kwargs)

        mode_cmd = "G91" if relative else "G90"
        
        # builds g-code line
        cmd = "{mode_cmd}{term_char}{cmd} F{frate} S{endstops:d}".format(
            mode_cmd = mode_cmd,
            term_char = self._term_char,
            cmd = self._make_position_string("G0", new_pos),
            frate = self._feed_rate,
            endstops = self._endstops
        )
        
        pt = types.Point()
        self.history[-1].append(pt)
        for ax in new_pos:
            setattr(pt, self._axes[ax], new_pos[ax])
        
        return await self.write(cmd=cmd, timeout=timeout)

    async def abs_move(self, position:types.Point|None=None, timeout:float=None, **kwargs) -> bool:
        '''
        Produce an absolute linear movement from current point to given point.
        
        Args:
            position (types.Point): coordinate in axis units
            timeout (float|None): timeout in seconds, or None for infinite.
            **kwargs (Optional[float]): coordinate values
        
        Returns:
            bool: True if the command was successful, False otherwise
        
        Raises:
            asyncio.TimeoutError: if command didn't return within timeout
        '''
        if position is not None:
            return await self.move(position=position, relative=False, timeout=timeout)
        
        return await self.move(relative=False, timeout=timeout, **kwargs)

    async def rel_move(self, position:types.Point|None=None, timeout:float=None, **kwargs) -> bool:
        '''
        Produce a relative linear movement from current point to given point.
        
        Args:
            position (types.Point): coordinate in axis units
            timeout (float|None): timeout in seconds, or None for infinite.
            **kwargs (Optional[float]): coordinate values
        
        Returns:
            bool: True if the command was successful, False otherwise
        
        Raises:
            asyncio.TimeoutError: if command didn't return within timeout
        '''
        if position is not None:
            return await self.move(position=position, relative=True, timeout=timeout)
        
        return await self.move(relative=True, timeout=timeout, **kwargs)

    def enable_endstops(self) -> None:
        '''
        Enable endstops.
        '''
        self._endstops = True
    
    def disable_endstops(self) -> None:
        '''
        Disable endstops.
        '''
        self._endstops = False
    
    async def probe_endstops(self, timeout:float|None=None) -> dict:
        '''
        Probe machine endstops.
        
        Args:
            timeout (float|None): timeout in seconds, or None for infinite.

        Returns:
            dict: status of endstops for each axis for which the state could be read.
            Endstop statuses are boolean values.
        
        Raises:
            asyncio.TimeoutError: if command didn't return within timeout
        '''
        if self.__writer is None: return {}
        # reads endstop status
        rep = await self.ask(cmd="M119", n_lines=2, timeout=timeout)

        trigs = {}
        for ax in self._axes:
            match = re.search("{}:([\w\s]*),".format(ax).encode("utf-8"), rep[0])
            if match is None or b"not stopped" in match.group(1):
                trigs.update({ax: False})
            else:
                trigs.update({ax: True})

        return trigs
    
    async def switch_motors(self, state:bool, timeout:float|None=None) -> bool:
        '''
        Switch motors on or off.
        
        Args:
            state (bool): if True, switches motors on. If False, switches motors off.
        
        Returns:
            bool: True if successful, False otherwise
        
        Raises:
            asyncio.TimeoutError: if command didn't return within timeout
        '''
        # switches motors on (state=True) or off (state=False)
        return await self.write(cmd="M84 S30" if state else "M18", timeout=timeout)
        
    async def trace(self, path:types.Path|None=None, xs:'list[float]|None'=None, ys:'list[float]|None'=None, zs:'list[float]|None'=None, cs:'list[float]|None'=None, timeout:float|None=None) -> bool:
        '''
        Trace given path.
        
        Args:
            path (Optional[Path]): path to trace
            xs (Optional[list]): point vector for x coordinates
            ys (Optional[list]): point vector for y coordinates
            zs (Optional[list]): point vector for z coordinates
            cs (Optional[list]): point vector for c coordinates
            timeout (float|None): timeout in seconds, or None for infinite.
        
        Returns:
            bool: True if successful, False otherwise
        
        Notes:
            path argument takes precedence over xs, ys, zs, and cs
        
        Raises:
            ValueError: if all point vectors are omitted
            
        '''
        if path is not None:
            return await self.trace(xs=path.xs, ys=path.ys, zs=path.zs, cs=path.cs, timeout=timeout)
        
        if xs is None and ys is None and zs is None and cs is None:
            raise ValueError("At least one vector must be specified.")
        
        Npts = len(xs) if xs is not None else len(ys) if ys is not None else len(zs) if zs is not None else len(cs)
        if (
            (xs is not None and len(xs)!=Npts) or
            (ys is not None and len(ys)!=Npts) or
            (zs is not None and len(zs)!=Npts) or
            (cs is not None and len(cs)!=Npts)
        ):
            raise ValueError("Provided vectors must have the same length.")

        
        if xs is None: xs = [0]*Npts
        if ys is None: ys = [0]*Npts
        if zs is None: zs = [0]*Npts
        if cs is None: cs = [0]*Npts
        
        success = True
        for n in range(Npts):
            pt = types.Point(xs[n], ys[n], zs[n], cs[n])
            self.history[-1].append(pt)
            chain = "G1 X{xpos:f} Y{ypos:f} Z{zpos:f} C{cpos:f} F{feed_rate:f} S{endstops:d}".format(
                xpos = xs[n],
                ypos = ys[n],
                zpos = zs[n],
                cpos = cs[n],
                feed_rate = self._feed_rate,
                endstops = self._endstops
            )
            success = success and (await self.ask(cmd=chain, timeout=timeout) is not None)
        
        return success and await self.wait(timeout=timeout)
        
    def set_model(self, model:render.Model) -> None:
        '''
        Set render model.
        
        Args:
            model (render.Model|None): model to use to render movements, or None for no model.
        '''
        self.__model = model
        for path in self.history:
            shape = render.CartesianWire(path, path.tool_size, path.color)
            shape.label = "Tool: {}".format(path.tool_size)
            model.add_shape(shape)
            path.shape = shape
    
    def get_model(self) -> render.Model|None:
        '''
        Get render model.
        
        Returns:
            render.Model|None: current render model.
        '''
        return self.__model
    
    model = property(get_model, set_model)


class SyncMachine():
    '''
    Machine handling class. This is the synchronous version of the Machine class.
    '''
    def __init__(self, port: str = ""):
        self.__machine = Machine(port)
        self.__loop = asyncio.get_event_loop()
    
    feed_rate = property(lambda self: self.__machine.feed_rate, lambda self, feed_rate: setattr(self.__machine, "feed_rate", feed_rate))
    model = property(lambda self: self.__machine.model, lambda self, model: setattr(self.__machine, "model", model))
    port = property(lambda self: self.__machine.port, lambda self, port: setattr(self.__machine, "port", port))
    serial_baud_rate = property(lambda self: self.__machine.serial_baud_rate, lambda self, baud_rate: setattr(self.__machine, "baud_rate", baud_rate))
    timeout = property(lambda self: self.__machine.timeout, lambda self, timeout: setattr(self.__machine, "timeout", timeout))
    term_char = property(lambda self: self.__machine.term_char, lambda self, term_char: setattr(self.__machine, "term_char", term_char))
    response_ok = property(lambda self: self.__machine.response_ok, lambda self, response_ok: setattr(self.__machine, "response_ok", response_ok))
    tool_size = property(lambda self: self.__machine.tool_size, lambda self, tool_size: setattr(self.__machine, "tool_size", tool_size))

    def disable_endstops(self) -> None:
        self.__machine.disable_endstops()

    def enable_endstops(self) -> None:
        self.__machine.enable_endstops()
    
    def open(self) -> bool:
        return self.__loop.run_until_complete(self.__machine.open())
    
    def close(self, timeout: float|None=None) -> bool:
        return self.__loop.run_until_complete(self.__machine.close(timeout))
    
    def write(self, cmd: str, timeout: float|None=None) -> None:
        return self.__loop.run_until_complete(self.__machine.write(cmd, timeout))
    
    def readline(self, timeout: float|None=None) -> bytes:
        return self.__loop.run_until_complete(self.__machine.readline(timeout))
    
    def wait_answer(self, n_lines: int = 1, timeout: float|None=None) -> 'list[bytes]':
        return self.__loop.run_until_complete(self.__machine.wait_answer(n_lines, timeout))
    
    def ask(self, cmd: str, n_lines: int = 1, timeout: float|None=None) -> 'list[bytes]':
        return self.__loop.run_until_complete(self.__machine.ask(cmd, n_lines, timeout))
    
    def wait(self, timeout: float|None=None) -> bool:
        return self.__loop.run_until_complete(self.__machine.wait(timeout))
    
    def get_position(self, timeout: float|None=None) -> types.Point:
        return self.__loop.run_until_complete(self.__machine.get_position(timeout))
    
    def set_position(self, position:types.Point|None=None, timeout: float|None=None, **kwargs) -> bool:
        return self.__loop.run_until_complete(self.__machine.set_position(position, timeout, **kwargs))

    def _set_position(self, x:list|tuple|dict|types.Point) -> None:
        return self.__loop.run_until_complete(self.__machine._set_position(x))
    
    position = property(get_position, _set_position)
    
    def move(self, relative=False, position:types.Point|None=None, timeout:float=None, **kwargs) -> bool:
        return self.__loop.run_until_complete(self.__machine.move(relative, position, timeout, **kwargs))
    
    def abs_move(self, position:types.Point|None=None, timeout:float=None, **kwargs) -> bool:
        return self.__loop.run_until_complete(self.__machine.abs_move(position, timeout, **kwargs))
    
    def rel_move(self, position:types.Point|None=None, timeout:float=None, **kwargs) -> bool:
        return self.__loop.run_until_complete(self.__machine.rel_move(position, timeout, **kwargs))
    
    def probe_endstops(self, timeout:float=None) -> dict:
        return self.__loop.run_until_complete(self.__machine.probe_endstops(timeout))
    
    endstops = property(probe_endstops)
    
    def switch_motors(self, state:bool) -> bool:
        return self.__loop.run_until_complete(self.__machine.switch_motors(state))
    
    def trace(self, path:types.Path|None=None, xs=None, ys=None, zs=None, cs=None, timeout:float=None) -> None:
        return self.__loop.run_until_complete(self.__machine.trace(path, xs, ys, zs, cs, timeout))
