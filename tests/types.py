import unittest
from pygraver.core.types import Point, Path, PathGroup, Surface, DivComponent, SortPredicate
import numpy as np

__all__ = ["TestPoint", "TestPath", "TestPathGroup", "TestSurface"]

class TestPoint(unittest.TestCase):
    def test_base(self):
        point1 = Point()
        self.assertEqual(point1.x, 0)
        self.assertEqual(point1.y, 0)
        self.assertEqual(point1.z, 0)
        self.assertEqual(point1.c, 0)
        point2 = Point(1,2,3,4)
        self.assertEqual(point2.x, 1)
        self.assertEqual(point2.y, 2)
        self.assertEqual(point2.z, 3)
        self.assertEqual(point2.c, 4)

    def test_operators(self):
        point1 = Point(1,2,3,4)
        point2 = point1 + point1
        self.assertEqual(point2.x, 2)
        self.assertEqual(point2.y, 4)
        self.assertEqual(point2.z, 6)
        self.assertEqual(point2.c, 8)
        point3 = point2 - point1
        self.assertEqual(point3.x, 1)
        self.assertEqual(point3.y, 2)
        self.assertEqual(point3.z, 3)
        self.assertEqual(point3.c, 4)
        point4 = -point1
        self.assertEqual(point4.x, -1)
        self.assertEqual(point4.y, -2)
        self.assertEqual(point4.z, -3)
        self.assertEqual(point4.c, -4)
        point5 = point1*3.5
        self.assertEqual(point5.x, 3.5)
        self.assertEqual(point5.y, 7.0)
        self.assertEqual(point5.z, 10.5)
        self.assertEqual(point5.c, 14.0)
        point6 = 3.5*point1
        self.assertEqual(point6.x, 3.5)
        self.assertEqual(point6.y, 7.0)
        self.assertEqual(point6.z, 10.5)
        self.assertEqual(point6.c, 14.0)

        self.assertEqual(point1, point3)
        self.assertNotEqual(point1, point2)

    def test_properties(self):
        point = Point()
        point.x = 1
        point.y = 2
        point.z = 3
        point.c = 4
        self.assertEqual(point.x, 1)
        self.assertEqual(point.y, 2)
        self.assertEqual(point.z, 3)
        self.assertEqual(point.c, 4)
        # binding tests; the following functions are tested in C++ tests otherwise
        self.assertEqual(type(point.radius), float)
        self.assertEqual(type(point.cartesian), Point)
        self.assertEqual(type(point.polar), Point)
    
    def test_methods(self):
        point = Point()
        # binding tests
        self.assertEqual(type(point.copy()), Point)
        self.assertEqual(type(point.distance_to(point)), float)
        self.assertEqual(type(point.angle(radians=False)), float)
        self.assertEqual(type(point.elevation(radians=True)), float)
        self.assertEqual(type(point.cylindrical(5)), Point)


class TestPath(unittest.TestCase):
    def test_base(self):
        path1 = Path()
        self.assertEqual(len(path1), 0)
        point = Point()
        path1.append(point)
        self.assertEqual(len(path1), 1)
        path2 = path1 + point
        self.assertEqual(len(path2), 2)
        xs = np.linspace(0,1,11)
        path3 = Path(xs)
        self.assertEqual(len(path3), 11)
        path4 = Path(xs,xs)
        self.assertEqual(len(path4), 11)
        path5 = Path(xs,xs,xs)
        self.assertEqual(len(path5), 11)
        path6 = Path(xs,xs,xs,xs)
        self.assertEqual(len(path6), 11)
        path7 = Path(point)
        self.assertEqual(len(path7), 1)
        path8 = Path([point, point, point])
        self.assertEqual(len(path8), 3)

    def test_operators(self):
        path1 = Path(Point())
        path2 = path1 + path1
        self.assertEqual(len(path2), 2)
        path3 = path1 * 3
        self.assertEqual(len(path3), 3)
        # it seems there's an issue with pybind11 and that one;
        # it won't work reversed; maybe switching to operators
        # with Path arguments (instead of shared_ptr<Path>) and int() * py::self
        # would help;
        # path4 = 4 * path1
        # self.assertEqual(len(path4), 4)
    
    def test_slices(self):
        xs = np.linspace(0, 1, 11)
        path = Path(xs, xs, xs, xs)
        self.assertEqual(path[0], Point(0,0,0,0))
        self.assertEqual(path[-1], Point(1,1,1,1))
        self.assertEqual(len(path[2:5]), 3)
        n = 0
        for p in path:
            n+=1
        self.assertEqual(n, 11)

    def test_properties(self):
        xs = np.linspace(0, 1, 11)
        path = Path(xs, xs, xs, xs)
        self.assertEqual(type(path.xs), np.ndarray)
        self.assertEqual(type(path.ys), np.ndarray)
        self.assertEqual(type(path.zs), np.ndarray)
        self.assertEqual(type(path.cs), np.ndarray)
        xs2 = np.linspace(0, 1, 6)
        path.xs = xs2
        self.assertEqual(len(path), 11)
        self.assertTrue(np.all(path.xs[:6]==xs2))
        xs3 = np.linspace(0, 1, 21)
        path.xs = xs3
        self.assertEqual(len(path), 11)
        self.assertTrue(np.all(path.xs==xs3[:11]))
        self.assertEqual(type(path.radii), np.ndarray)
        self.assertEqual(type(path.angles), np.ndarray)
        self.assertEqual(type(path.elevations), np.ndarray)
        self.assertEqual(type(path.xy), np.ndarray)
        self.assertEqual(type(path.rmax), float)
        self.assertEqual(type(path.length), float)
        self.assertEqual(type(path.centroid), Point)
        self.assertEqual(type(path.cartesian), Path)
        self.assertEqual(type(path.polar), Path)
        self.assertEqual(type(path.is_ccw), bool)
        self.assertEqual(type(path.is_closed), bool)

    def test_methods(self):
        xs = np.linspace(0, 1, 11)
        path = Path(xs, xs, xs, xs)
        self.assertEqual(type(path.simplify_above(0.5)), Path)
        self.assertEqual(type(path.split_above(0.5)), list)
        self.assertEqual(type(path.create_forward_ramps(0.5, 0.2, 1.0)), Path)
        self.assertEqual(type(path.create_backward_ramps(0.5, 0.2, 1.0)), Path)
        self.assertEqual(type(path.rearrange(0.5, path[0])), Path)
        self.assertEqual(type(path.copy()), Path)
        self.assertEqual(type(path.shift(Point())), Path)
        self.assertEqual(type(path.scale(2, Point())), Path)
        self.assertEqual(type(path.mirror(True, True, True)), Path)
        self.assertEqual(type(path.rotate(0,0,0)), Path)
        self.assertEqual(type(path.inflate(0)), Path)
        self.assertEqual(type(path.simplify(0.1)), Path)
        self.assertEqual(type(path.interpolate(0.1)), Path)
        self.assertEqual(type(path.cylindrical(1)), Path)
        self.assertEqual(type(path.tangent_angle(False)), np.ndarray)
        self.assertEqual(type(path.divergence(DivComponent.DxDx)), np.ndarray)
        

class TestPathGroup(unittest.TestCase):
    def test_base(self):
        pathgroup1 = PathGroup()
        self.assertEqual(len(pathgroup1), 0)
        xs = np.linspace(0, 1, 11)
        path = Path(xs, xs, xs, xs)
        pathgroup2 = PathGroup([path, path, path, path])
        self.assertEqual(len(pathgroup2), 4)
    
    def test_slices(self):
        xs = np.linspace(0, 1, 11)
        path = Path(xs, xs, xs, xs)
        pathgroup = PathGroup([path]*20)
        self.assertEqual(len(pathgroup[3:6]), 3)
        n = 0
        for p in pathgroup:
            n+=1
        self.assertEqual(n, 20)
    
    def test_operators(self):
        xs = np.linspace(0, 1, 11)
        path = Path(xs, xs, xs, xs)
        pathgroup = PathGroup([path]*3)
        pathgroup2 = pathgroup + pathgroup
        self.assertEqual(len(pathgroup2), 6)
        pathgroup3 = pathgroup * 3
        self.assertEqual(len(pathgroup3), 9)
        pathgroup.append(path)
        self.assertEqual(len(pathgroup), 4)
    
    def test_properties(self):
        xs = np.linspace(0, 1, 11)
        path = Path(xs, xs, xs, xs)
        pathgroup = PathGroup([path]*5)
        self.assertEqual(type(pathgroup.paths), list)
        pathgroup.paths = [path, path, path]
        self.assertEqual(len(pathgroup), 3)
        pathgroup2 = PathGroup()
        for n in range(10): # need to be done that way to have 10 different path instances
            pathgroup2.append(Path(xs, xs, xs, xs))
        self.assertEqual(type(pathgroup2.steps), list)
        self.assertEqual(pathgroup2.steps[0], Point())
        pathgroup2.steps = [Point(1,1,1,1)]
        self.assertEqual(pathgroup2.steps[0], Point(1,1,1,1))
        self.assertEqual(type(pathgroup2.cartesian), PathGroup)
        self.assertEqual(type(pathgroup2.polar), PathGroup)
        self.assertEqual(type(pathgroup2.radius), float)
        self.assertEqual(type(pathgroup2.centroid), Point)
        self.assertEqual(type(pathgroup2.envelope), list)
    
    def test_methods(self):
        xs = np.linspace(0, 1, 11)
        path = Path(xs, xs, xs, xs)
        pathgroup = PathGroup([path]*5)
        self.assertEqual(type(pathgroup.cylindrical(5)), PathGroup)
        self.assertEqual(type(pathgroup.scale(2, Point())), PathGroup)
        self.assertEqual(type(pathgroup.scale_to_size(2, Point())), PathGroup)
        self.assertEqual(type(pathgroup.simplify_above(0.5)), PathGroup)
        self.assertEqual(type(pathgroup.split_above(0.5)), PathGroup)
        self.assertEqual(type(pathgroup.create_backward_ramps(0.5, 0.2, 0.3)), PathGroup)
        self.assertEqual(type(pathgroup.create_forward_ramps(0.5, 0.2, 0.3)), PathGroup)
        self.assertEqual(type(pathgroup.sort_paths(Point(), SortPredicate.StartToStart)), PathGroup)
        self.assertEqual(type(pathgroup.rearrange(0.5)), PathGroup)
        with self.assertRaises(ValueError):
            pathgroup.reorder(range(11))
        self.assertEqual(type(pathgroup.reorder(range(5))), PathGroup)


class TestSurface(unittest.TestCase):
    def setUp(self):
        super().setUp()
        ts = np.linspace(0, 2*np.pi, 51)
        self.path = Path(np.cos(ts), np.sin(ts), 0, 0)
    
    def test_base(self):
        surf1 = Surface(self.path)
        self.assertEqual(len(surf1.contours), 1)
        self.assertEqual(len(surf1.holes), 0)
        surf2 = Surface([self.path, self.path.shift(Point(5,5,0,0))])
        self.assertEqual(len(surf2.contours), 2)
        surf3 = Surface(self.path, [self.path.scale(0.5, Point())])
        self.assertEqual(len(surf3.contours), 1)
        self.assertEqual(len(surf3.holes), 1)
    
    def test_operators(self):
        surf1 = Surface(self.path)
        surf2 = Surface(self.path.shift(Point(0.5,0.5,0,0)))
        surf3 = Surface(self.path.scale(0.5, Point()))
        union = surf1 + surf2
        diff1 = surf1 - surf2
        diff2 = surf1 - surf3
        inter = surf1 * surf2
        self.assertEqual(len(union), 1)
        self.assertEqual(len(union[0].contours), 1)
        self.assertEqual(len(union[0].holes), 0)
        self.assertEqual(len(diff1), 1)
        self.assertEqual(len(diff1[0].contours), 1)
        self.assertEqual(len(diff1[0].holes), 0)
        self.assertEqual(len(diff2), 1)
        self.assertEqual(len(diff2[0].contours), 1)
        # doesn't generate holes
        self.assertEqual(len(diff2[0].holes), 0)
        self.assertEqual(len(inter), 1)
        self.assertEqual(len(inter[0].contours), 1)
        self.assertEqual(len(inter[0].holes), 0)

    def test_methods(self):
        surf = Surface(self.path)
        self.assertEqual(type(surf.get_milling_paths(0.5, 0.3)), list)
        self.assertEqual(type(surf.get_milled_surface(0.5, 0.3)), list)
        self.assertTrue(surf.contains(Point()))
        self.assertFalse(surf.contains(Point(3,3,0,0)))
        self.assertEqual(type(surf.combine()), list)
        self.assertEqual(type(surf.correct_height([self.path], 0, 1.0)), list)
        # because of automatic typecasting and PathGroup.__iter__, this calls
        # the method taking std::vector<Path> instead of PathGroup argument
        self.assertEqual(type(surf.correct_height(PathGroup([self.path]), 0, 1.0)), list)
        self.assertEqual(type(surf.correct_height(pathgroup=PathGroup([self.path]), clearance=0, safe_height=1.0)), PathGroup)
