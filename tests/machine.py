import asyncio
import unittest
import pygraver
from pygraver.machine import Machine, serial_asyncio
from unittest.mock import Mock
from serial import SerialException

from .common import *

__all__ = ["MachineTestCase", "TestOpenMachine", "TestCloseMachine", "TestMachineBaseCommands", "TestMachineCommands"]

class MachineTestCase(unittest.TestCase):
    def setUp(self):
        self.__event_loop = asyncio.new_event_loop()
        asyncio.set_event_loop(self.__event_loop)
        self.machine = Machine()
    
    def tearDown(self):
        self.__event_loop.close()
    

class TestOpenMachine(MachineTestCase):
    @run_async
    async def test_open_success(self):
        '''Open device -> success
        We don't test whether the port is valid or not, as this is a matter
        for asyncio-serial; we mock the asyncio-serial coroutine
        to return something.
        '''
        serial_asyncio.open_serial_connection = AsyncMock(return_value=(object(), object()))
        self.machine.port = "some_port"
        result = await self.machine.open()
        self.assertTrue(result)

    @run_async
    async def test_already_open(self):
        self.machine._Machine_writer = Mock()
        self.machine.port = "some_port"
        with self.assertRaises(SerialException):
            await self.machine.open()
    
    @run_async
    async def test_empty_port(self):
        with self.assertRaises(ValueError):
            await self.machine.open()



class TestCloseMachine(MachineTestCase):
    @run_async
    async def test_close_fail(self):
        result = await self.machine.close()
        self.assertFalse(result)
    
    @run_async
    async def test_close_success(self):
        self.machine._Machine__writer = Mock()
        self.machine._Machine__writer.close = Mock()
        self.machine._Machine__writer.wait_closed = AsyncMock()
        result = await self.machine.close()
        self.assertTrue(result)
        self.assertEqual(self.machine._Machine__writer, None)
        self.assertEqual(self.machine._Machine__reader, None)


class TestMachineBaseCommands(MachineTestCase):
    @run_async
    async def test_write(self):
        self.machine._Machine__writer = Mock()
        self.machine._Machine__writer.write = Mock()
        self.machine._Machine__writer.drain = AsyncMock()
        cmd = "A command string"
        await self.machine.write(cmd)

        self.assertEqual(
            self.machine._Machine__writer.write.call_args.args[0],
            "{}{}".format(cmd, self.machine._term_char).encode("utf-8")
        )

    @run_async
    async def test_readline(self):
        expected = "{}{}".format(self.machine._response_ok, self.machine._term_char).encode("utf-8")
        self.machine._Machine__reader = Mock()
        self.machine._Machine__reader.readuntil = AsyncMock(return_value=expected)
        result = await self.machine.readline()
        self.assertEqual(self.machine._Machine__reader.readuntil.call_args.kwargs["separator"], self.machine._term_char.encode("utf-8"))
        self.assertEqual(result, expected)
    
    @run_async
    async def test_wait_answer(self):
        multiline_answer = ["first line", "second line", "third line", self.machine._response_ok]
        def make_expected(v):
            return "{}{}".format(v, self.machine._term_char).encode("utf-8")
        expected = [make_expected(v) for v in multiline_answer]

        self.machine._Machine__reader = Mock()
        self.machine._Machine__reader.readuntil = AsyncListMock(return_value=expected)

        result = await self.machine.wait_answer(n_lines=len(expected)+3)
        self.assertEqual(result, expected)
        result = await self.machine.wait_answer(n_lines=len(expected)-1)
        self.assertEqual(result, None)
    
    @run_async
    async def test_ask(self):
        # 'ask' calls Machine.write followed by Machine.wait_answer;
        # we tested both already; we only want to make sure that 'ask'
        # calls them with the correct arguments
        self.machine.write = AsyncMock()
        self.machine.wait_answer = AsyncMock()

        test_cmd = "A command"
        test_n_lines = 10
        test_timeout = 25
        await self.machine.ask(
            cmd=test_cmd,
            n_lines=test_n_lines,
            timeout=test_timeout
        )
        self.assertEqual(self.machine.write.call_args.kwargs["cmd"], test_cmd)
        self.assertEqual(self.machine.write.call_args.kwargs["timeout"], test_timeout)
        self.assertEqual(self.machine.wait_answer.call_args.kwargs["n_lines"], test_n_lines)
        self.assertEqual(self.machine.wait_answer.call_args.kwargs["timeout"], test_timeout)
    
    @run_async
    async def test_ask_success(self):
        self.machine.write = AsyncMock()
        self.machine.wait_answer = AsyncMock()
        rvalue = "{}{}".format(self.machine._response_ok, self.machine._term_char).encode("utf-8")
        self.machine.wait_answer.return_value = [rvalue]
        res = await self.machine.ask(cmd="A command", n_lines=1)
        self.assertEqual(res, [rvalue])

    @run_async
    async def test_wait(self):
        self.machine.write = AsyncMock()
        self.machine._Machine__reader = Mock()
        self.machine._Machine__reader.readuntil = AsyncMock()
        await self.machine.wait()
        self.assertEqual(self.machine.write.call_args.kwargs["cmd"], "G4 S0")
    
    @run_async
    async def test_get_position_no_serial(self):
        self.machine._Machine__writer = None
        res = await self.machine.get_position()
        self.assertEqual(type(res), pygraver.core.types.Point)


class TestMachineCommands(MachineTestCase):
    def setUp(self):
        super().setUp()
        self.machine._Machine__writer = Mock()
        self.machine.write = AsyncMock()
        rvalue = "{}{}".format(self.machine._response_ok, self.machine._term_char).encode("utf-8")
        self.machine.ask = AsyncMock(return_value=[rvalue])
    
    def make_positions(self):
        return {key:10 for key in self.machine._axes}
    
    @run_async
    async def test_get_position(self):
        await self.machine.get_position()
        self.assertEqual(self.machine.ask.call_args.kwargs["cmd"], "M114")
        self.assertEqual(self.machine.ask.call_args.kwargs["n_lines"], 2)
    
    def test_parse_position(self):
        positions = self.make_positions()
        res = self.machine._parse_position(**positions)
        self.assertEqual(res, positions)
        # parsed positions must not contain this axis as it's not in machine._axes
        positions.update({"not_in_axes":10})
        res = self.machine._parse_position(**positions)
        self.assertNotEqual(res, positions)

    @run_async
    async def test_set_position(self):
        positions = self.make_positions()
        res = await self.machine.set_position(**positions)
        self.assertTrue(res)
        # check that history got modified
        self.assertEqual(self.machine.history[-1][-1], pygraver.core.types.Point(x=10,y=10,z=10,c=10))
        # check 'ask' parameters
        cmd = self.machine._make_position_string("G92", positions)
        self.assertEqual(self.machine.ask.call_args.kwargs["cmd"], cmd)
        self.assertEqual(self.machine.ask.call_args.kwargs["n_lines"], 1)

    def test_make_position_string(self):
        positions = self.make_positions()
        cmd = "A command"
        args = " ".join(["{}{:f}".format(key, positions[key]) for key in positions])
        self.assertEqual(
            self.machine._make_position_string(cmd, positions),
            "{} {}".format(cmd, args)
        )

    @run_async
    async def test_set_position_prop(self):
        # set with dict
        await self.machine._set_position(self.make_positions())
        # set with list
        await self.machine._set_position([10,10,10,10])
        # test with unsupported type
        with self.assertRaises(TypeError):
            await self.machine._set_position("not supported")
    
    def move_make_expected(self, relative):
        mode_cmd = "G91" if relative else "G90"
        return "{mode_cmd}{term_char}{cmd} F{frate} S{endstops:d}".format(
            mode_cmd = mode_cmd,
            term_char = self.machine._term_char,
            cmd = self.machine._make_position_string("G0", self.make_positions()),
            frate = self.machine._feed_rate,
            endstops = self.machine._endstops
        )
    
    @run_async
    async def test_move(self):
        positions = self.make_positions()
        history_length = len(self.machine.history[-1])
        # relative move
        await self.machine.move(relative=True, **positions)
        # check that entry got added to history
        self.assertEqual(len(self.machine.history[-1]), history_length+1)
        self.assertEqual(self.machine.write.call_args.kwargs["cmd"], self.move_make_expected(relative=True))
        # absolute move
        await self.machine.move(relative=False, **positions)
        self.assertEqual(len(self.machine.history[-1]), history_length+2)
        self.assertEqual(self.machine.write.call_args.kwargs["cmd"], self.move_make_expected(relative=False))

    @run_async
    async def test_probe_endstops(self):
        rmsg = [
            "Endstops - X: not stopped, Y: not stopped, Z: at min stop, C: not stopped, Z probe: at min stop",
            self.machine._response_ok
        ]
        self.machine.ask.return_value = ["{}{}".format(m, self.machine._term_char).encode("utf-8") for m in rmsg]
        res = await self.machine.probe_endstops()
        self.assertEqual(self.machine.ask.call_args.kwargs["cmd"], "M119")
        self.assertEqual(self.machine.ask.call_args.kwargs["n_lines"], 2)
        expected = {"X":False, "Y":False, "Z":True, "C":False}
        self.assertEqual(res, expected)

    @run_async
    async def test_switch_motors(self):
        await self.machine.switch_motors(True)
        self.assertEqual(self.machine.write.call_args.kwargs["cmd"], "M84 S30")
        await self.machine.switch_motors(False)
        self.assertEqual(self.machine.write.call_args.kwargs["cmd"], "M18")

    @run_async
    async def test_trace_pattern_fail(self):
        with self.assertRaises(ValueError):
            await self.machine.trace_pattern()
        
        v1 = [1, 2, 3, 4]
        v2 = [1, 2, 3]
        with self.assertRaises(ValueError):
            await self.machine.trace_pattern(xs=v1, ys=v2)
    
    @run_async
    async def test_trace_pattern(self):
        v1 = [1, 2, 3, 4]
        await self.machine.trace_pattern(xs=v1, ys=v1, zs=v1, cs=v1)
