import unittest
from pygraver.core.render import Model, Extrusion, Cylinder, Marker, MarkerCollection, Wire, WireCollection
from pygraver.core.types import Path, Point, Surface
from vtkmodules.vtkInteractionWidgets import vtkTextWidget
from vtkmodules.vtkRenderingCore import vtkActor

__all__ = ["TestModel", "TestShape3D", "TestCylinder", "TestExtrusion", "TestMarker", "TestWire"]

class TestModel(unittest.TestCase):
    def setUp(self):
        super().setUp()
        self.model = Model()
    
    def test_shapes(self):
        path = Path()
        path.append(Point(0,0,0,0))
        path.append(Point(1,0,0,0))
        path.append(Point(1,1,0,0))
        path.append(Point(0,1,0,0))
        path.append(Point(0,0,0,0))
        shp1 = Extrusion(path, 1, Point(0,0,1), [255,255,255,255])
        self.assertFalse(self.model.has_shape(shp1))
        self.model.add_shape(shp1)
        self.assertTrue(self.model.has_shape(shp1))
        shp2 = Extrusion(path, 1, Point(0,0,1), [255,255,255,255])
        self.model.remove_shape(shp2)
        self.assertTrue(self.model.has_shape(shp1))
        self.model.remove_shape(shp1)
        self.assertFalse(self.model.has_shape(shp1))

    def test_widgets(self):
        widget1 = vtkTextWidget()
        self.assertFalse(self.model.has_widget(widget1))
        self.model.add_widget(widget1)
        self.assertTrue(self.model.has_widget(widget1))
        widget2 = vtkTextWidget()
        self.model.remove_widget(widget2)
        self.assertTrue(self.model.has_widget(widget1))
        self.model.remove_widget(widget1)
        self.assertFalse(self.model.has_widget(widget1))

    def test_bg_color(self):
        with self.assertRaises(ValueError):
            self.model.background_color = [0]
            self.model.background_color = [0, 0, 0, 0]
        self.model.background_color = [128, 128, 128]
        self.assertEqual(self.model.background_color, [128, 128, 128])


class TestCylinder(unittest.TestCase):
    def test_base(self):
        cylinder = Cylinder(2, 1, Point(0,0,0), Point(0,0,1), [128, 128, 128, 128])
        self.assertEqual(len(cylinder.actors), 1)


class TestShape3D(unittest.TestCase):
    def setUp(self):
        super().setUp()
        self.shape = Cylinder(2.0, 1.0, Point(0,0,0), Point(0,0,1), [128, 128, 128, 128])

    def test_properties(self):
        self.shape.label = "Something"
        self.assertEqual(self.shape.label, "Something")
        with self.assertRaises(ValueError):
            self.shape.base_color = [0]
            self.shape.base_color = [0,0,0,0,0]
        self.shape.base_color = [0,0,0]
        self.shape.base_color = [128,128,128,128]
        self.assertEqual(self.shape.base_color, [128,128,128,128])
        with self.assertRaises(ValueError):
            self.shape.highlight_color = [0]
            self.shape.highlight_color = [0,0,0,0,0]
        self.shape.highlight_color = [0,0,0]
        self.shape.highlight_color = [128,128,128,128]
        self.assertFalse(self.shape.scalar_color_mode)
        self.shape.scalar_color_mode = True
        self.assertTrue(self.shape.scalar_color_mode)
        self.assertTrue(self.shape.visible)
        self.shape.visible = False
        self.assertFalse(self.shape.visible)
        self.shape.toggle_visibility()
        self.assertTrue(self.shape.visible)
    
    def test_methods(self):
        self.shape.set_scalar_color_range(0, 1)
        with self.assertRaises(ValueError):
            self.shape.set_scalar_color_range(1, 0)
        self.assertEqual(self.shape.get_scalar_color_range(), [0,1])
        actors = self.shape.actors
        self.shape.set_highlighted(actors[0], True)
        self.assertTrue(self.shape.get_highlighted(actors[0]))
        # argument names must be specified here, otherwise it's assumed that
        # the 1st argument is the address of an actor object
        self.shape.set_highlighted(index=0, enabled=False)
        self.assertFalse(self.shape.get_highlighted(index=0))
        self.shape.set_highlighted(True)
        self.assertTrue(self.shape.get_highlighted(actors[0]))
        self.shape.toggle_highlighted(actors[0])
        self.assertFalse(self.shape.get_highlighted(actors[0]))
        self.shape.toggle_highlighted(index=0)
        self.assertTrue(self.shape.get_highlighted(actors[0]))
        self.shape.toggle_highlighted()
        self.assertFalse(self.shape.get_highlighted(actors[0]))
        self.shape.label = "Something"
        self.assertEqual(self.shape.get_interactive()[0], (actors[0], "Something"))
        self.assertEqual(type(self.shape.is_point_inside(Point())), bool)
        self.assertEqual(type(self.shape.distance_to_actor(actors[0], Point())), float)
        self.assertEqual(type(self.shape.closest_actor(Point())), tuple)
        self.assertEqual(type(self.shape.intersecting_actor(Point(0,0,-1), Point(0,0,2))), vtkActor)
        self.assertEqual(self.shape.intersecting_actor(Point(), Point()), None)


class TestExtrusion(unittest.TestCase):
    def setUp(self):
        super().setUp()
        self.path = Path()
        self.path.append(Point(0,0,0,0))
        self.path.append(Point(1,0,0,0))
        self.path.append(Point(1,1,0,0))
        self.path.append(Point(0,1,0,0))
        self.path.append(Point(0,0,0,0))

    def test_base(self):
        shape1 = Extrusion(self.path, 1, Point(0,0,1), [255,255,255,255])
        self.assertEqual(len(shape1.actors), 1)
        shape2 = Extrusion(Surface(self.path), 1, Point(0,0,1), [255,255,255,255])
        self.assertEqual(len(shape2.actors), 1)

    def test_methods(self):
        shape = Extrusion(self.path, 1, Point(0,0,1), [255,255,255,255])
        shape.set_shape(self.path, 1, Point(0,0,1), [255,255,255,255])
        shape.set_shape(Surface(self.path), 1, Point(0,0,1), [255,255,255,255])
    

class TestMarker(unittest.TestCase):
    def test_single(self):
        marker = Marker("A", Point(), 1, 1, 1, Point(0,0,1), [255,255,255,255])
        self.assertEqual(len(marker.actors), 1)
    
    def test_collection(self):
        markers = MarkerCollection("A", [Point()]*10, 1, 1, 1, Point(0,0,1), [255,255,255,255])
        self.assertEqual(len(markers.actors), 10)

class TestWire(unittest.TestCase):
    def setUp(self):
        super().setUp()
        self.path = Path()
        self.path.append(Point(0,0,0,0))
        self.path.append(Point(1,0,0,0))
        self.path.append(Point(1,1,0,0))
        self.path.append(Point(0,1,0,0))
        self.path.append(Point(0,0,0,0))

    def test_single(self):
        wire = Wire(self.path, 1, [255, 255, 255, 255])
        self.assertEqual(len(wire.actors), 1)
        wire.set_path(self.path, 1, [255, 255, 255, 255])
        self.assertEqual(len(wire.actors), 1)

    def test_collection(self):
        wires = WireCollection([self.path]*10, 1, [255, 255, 255, 255])
        self.assertEqual(len(wires.actors), 10)
        wires.set_paths([self.path]*3, 1, [255, 255, 255, 255])
        self.assertEqual(len(wires.actors), 3)
        wires.set_path(1, self.path)
