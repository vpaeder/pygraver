# PyGraver: a toolkit to generate and display toolpaths for a 4-axis engraver with a basic machine interface

PyGraver is a Python package with a C++ core that provides functions and classes to generate and display toolpaths. It is meant for a 4-axis engraver/milling machine with guilloche engraving in mind (see my [website](https://paeder.fi) or my [Etsy boutique](https://www.etsy.com/shop/VincentPaeder) for realisations). It includes methods to generate paths for different configurations, such as router/milling machine, guilloche lathe/mill or ornamental lathe.

PyGraver contains a rendering submodule with local and remote rendering features, as well as a simple SVG reader to produce toolpaths and masks from SVG files. It also provides a basic machine interface to control a CNC machine with Reprap-compatible firmware.

<div style="width:100%;text-align:center"><img src="https://user-images.githubusercontent.com/6388158/185990437-6d2894b0-45ae-4c8f-9027-f1114c49b84d.png" alt="A model generated with PyGraver" width="66%"><p><strong>A model generated with PyGraver</strong></p></div>

## Table of contents

1. [Dependencies](#dependencies)
2. [Installation](#installation)
3. [Tests](#tests)
4. [Usage](#usage)
5. [Examples](#examples)

## Dependencies

You need Python 3.10 or newer and your C++ compiler must support C++20 or newer.

Beyond this, you'll need the following C/C++ libraries:

- [libgeos 3.10.0](https://libgeos.org/) or newer
- [libxml2](https://gitlab.gnome.org/GNOME/libxml2/-/wikis/home)
- [boost](https://www.boost.org/)
- [VTK 9.0.0](https://vtk.org/) or newer, with Python wrapper
- [fmt](https://fmt.dev/latest/index.html)

Build tools:

- [CMake](https://cmake.org/)
- [Ninja](https://ninja-build.org/)

and the following Python libraries (should be installed automatically by the setup script):

- [scikit-build](https://pypi.org/project/scikit-build/)
- [pybind11](https://pypi.org/project/pybind11/)
- [NumPy](https://numpy.org/)
- [trame](https://kitware.github.io/trame/) for remote rendering
- [pySerial](https://github.com/pyserial/pyserial) for the machine driver
- [pyserial-asyncio](https://github.com/pyserial/pyserial-asyncio)

For testing:

- [tox](https://pypi.org/project/tox/) (optional)
- [googletest](https://google.github.io/googletest/) (pulled by CMake if necessary)

Optional for 2D rendering examples:

- [matplotlib](https://matplotlib.org/)

## Installation

First of all, check that you have all the necessary C/C++ libraries and build tools. Then you can use the Python setup script to do the rest:

```bash
python -m pip install git+https://github.com/vpaeder/pygraver
```

## Tests

Unit tests are included for most of PyGraver's features. Run the following command from the within the cloned directory:

```bash
python -m unittest
```

Or use *tox* if you have it installed.

## Usage

PyGraver is structured in the following way: the C++ part of PyGraver is in the *pygraver.core* submodule; rendering aids written in Python are placed in *pygraver.render* for local rendering and *pygraver.web* for remote rendering; machine-related classes are in *pygraver.machine*.

The features of PyGraver can be classified in three categories - 1) toolpath generation, 2) toolpath rendering, 3) machine control. I try to explain here what each category contains. The Python API is described alongside. Besides, the API for the C++ core module is detailled in the *doc* folder.

### Toolpath generation

PyGraver provides 4 base types to generate toolpaths, present in the submodule *pygraver.core.types*: Point, Path, PathGroup and Surface. Moreover, the submodule *pygraver.core.svg* hosts a basic SVG file parser and rasterizer that converts SVG shapes into paths.

#### EndCapStyle enum (pygraver.core.types.EndCapStyle)

This defines options for the *buffer* methods of *Path* and *PathGroup* classes.

##### Elements

- *Round*: round end caps
- *Flat*: flat end caps
- *Square*: square end caps

#### JoinStyle enum (pygraver.core.types.JoinStyle)

This defines options for the *buffer* methods of *Path* and *PathGroup* classes.

##### Elements

- *Round*: round join style
- *Mitre*: mitre join style
- *Bevel*: bevel join style

#### DivComponent enum (pygraver.core.types.DivComponent)

This is used to select a divergence matrix component in *divergence* methods of *Path* and *PathGroup* classes.

##### Elements

- *DxDx*: $\delta x/\delta x$
- *DxDy*: $\delta x/\delta y$
- *DxDz*: $\delta x/\delta z$
- *DyDx*: $\delta y/\delta x$
- *DyDy*: $\delta y/\delta y$
- *DyDz*: $\delta y/\delta z$
- *DzDx*: $\delta z/\delta x$
- *DzDy*: $\delta z/\delta y$
- *DzDz*: $\delta z/\delta z$

#### DivComponent enum (pygraver.core.types.RampDirection)

This defines ramp direction for the *create_ramps* methods of *Path* and *PathGroup* classes.

##### Elements

- *Forward*: forward ramp (= upward ramp in forward direction)
- *Backward*: backward ramp (= backward ramp in forward direction)
- *Both*: both directions

#### DivComponent enum (pygraver.core.types.SortPredicate)

This defines sort predicates for the *sort_paths* method of the *PathGroup* class.

##### Elements

- *StartToStart*: sort paths by distance between start points
- *EndToStart*: sort paths by distance between end and start points
- *EndToEnd*: sort paths by distance between end points

#### DivComponent enum (pygraver.core.types.BooleanOperation)

This defines boolean operations for the *boolean_operation* method of the *Surface* class.

##### Elements

- *Union*: boolean union
- *Difference*: boolean difference
- *SymmetricDifference*: boolean symmetric difference
- *Intersection*: boolean intersection

#### Point class (pygraver.core.types.Point)

The Point class represents a point in 3D space. It has the three usual coordinates (*x*, *y*, *z*) and a supplementary *c* coordinate that represents a rotation in the xy plane. This is used to control the 4th axis of an actual machine. To draw in the xy plane, one can work with the *x* and *y* axes (cartesian coordinates) or the *x* and *c* axes (polar coordinates). There's also an option to draw along the *x* axis with the 4th axis perpendicular to it (ornamental lathe setup; see *cylindrical* method).

##### Constructor

```python
Point(x=0, y=0, z=0, c=0)
```

###### Arguments

- *x* (float): value for x coordinate (default: 0)
- *y* (float): value for y coordinate (default: 0)
- *z* (float): value for z coordinate (default: 0)
- *c* (float): value for c coordinate (default: 0)

##### Properties

| Name | Type | Description |
|------|------|-------------|
| `x` | getter/setter (float) | access x coordinate |
| `y` | getter/setter (float) | access y coordinate |
| `z` | getter/setter (float) | access z coordinate |
| `c` | getter/setter (float) | access c coordinate |
| `radius` | getter (float) | compute point radius; return ```r=sqrt(x**2 + y**2 + z**2)``` |
| `cartesian` | getter (Point) | produce a copy of point in cartesian coordinates |
| `polar` | getter (Point) | produce a copy of point in polar coordinates |

##### Methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `angle(radians:bool=False) -> float` | compute angle in xy plane | *radians* (bool): if True, return angle in radians; if False, return angle in degrees (default) |
| `elevation(radians:bool=False) -> float` | compute angle above xy plane | *radians* (bool): if True, return angle in radians; if False, return angle in degrees (default) |
| `distance_to(point:Point) -> float` | compute distance between point and another point | *point* (Point): point to compute distance to |
| `cylindrical(radius:float) -> Point` | produce a copy of point projected onto a cylinder align with x axis and of given radius | *radius* (float): radius of cylinder |

##### Implemented standard methods

- `__add__`: pt1 + pt2
- `__sub__`: pt1 - pt2
- `__neg__`: -pt
- `__mul__`: pt*n
- `__rmul__`: n*pt
- `__eq__`: pt1 == pt2; True if pt1 and pt2 represent the same point
- `__neq__`: pt1 != pt2
- `__repr__`: print out coordinate values

#### Path class (pygraver.core.types.Path)

The Path class uses points to represent paths in 3D. It comes with a range of methods to manipulate paths and optimize them for fabrication. For instance, you may want to apply some geometric operations (scaling, rotation, ...) to generate a pattern, then simplify on various criteria (machine resolution, height, ...), then add some ramps to avoid crashing your tool into the workpiece.

##### Constructor

```python
Path(xs=numpy.array(), ys=numpy.array(), zs=numpy.array(), cs=numpy.array())
Path(point:Point)
Path(points:list[Point])
```

###### Arguments

- *xs* (numpy.array|list[float]): x coordinates (default: numpy.array())
- *ys* (numpy.array|list[float]): y coordinates (default: numpy.array())
- *zs* (numpy.array|list[float]): z coordinates (default: numpy.array())
- *cs* (numpy.array|list[float]): c coordinates (default: numpy.array())

- *point* (Point): first point of new path

- *points* (list[Point]): list of points to build path from

##### Properties

| Name | Type | Description |
|------|------|-------------|
| `xs` | getter/setter (float) | access x coordinates |
| `ys` | getter/setter (float) | access y coordinates |
| `zs` | getter/setter (float) | access z coordinates |
| `cs` | getter/setter (float) | access c coordinates |
| `length` | getter (float) | compute path length; this gives the geometric length, not the number of elements |
| `is_ccw` | getter (bool) | tell if path's winding is counterclockwise |
| `is_closed` | getter (bool) | tell if path is closed |
| `rmax` | getter (float) | compute largest distance from path centroid |
| `centroid` | getter (Point) | compute path centroid |
| `cartesian` | getter (Path) | create a copy of path in cartesian coordinates |
| `polar` | getter (Path) | create a copy of path in polar coordinates |
| `convex_hull` | getter (Path) | compute convex hull |
| `xy` | getter (numpy.array) | convenience property to produce data for 2D plotting |

##### Methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `copy() -> Path` | produce a copy of path | |
| `shift(distance:Point) -> Path` | translate path by given distance | *distance* (Point): translation vector |
| `scale(factor:float, center:Point) -> Path` | scale path uniformly around center point | *factor* (float): scaling factor<br/> - *center* (Point): center point |
| `rotate(yaw_angle:float, pitch_angle:float, roll_angle:float) -> Path` | rotate path around origin | *yaw_angle* (float): yaw angle in degrees <br/> *pitch_angle* (float): pitch angle in degrees<br/> *roll_angle* (float): roll angle in degrees |
| `mirror(along_x:bool, along_y:bool, along_z:bool) -> Path` | mirror path along specified axes | *along_x* (bool): if True, mirror along x axis<br/> *along_y* (bool): if True, mirror along y axis<br/> *along_z* (bool): if True, mirror along z axis |
| `matrix_transform(components:list) -> Path` | general matrix transform | *components* (list): components of the 4x4 matrix transform (flattened) |
| `inflate(amount:float) -> Path` | inflate path by given amount | *amount* (float): amount to inflate by |
| `buffer(amount:float, cap_style:EndCapStyle=Round, join_style:JoinStyle=Round, mitre_limit:float=1.0) -> Path` | buffer path by given amount; this uses libgeos buffering algorithm | *amount* (float): amount to buffer by<br/> *cap_style* (EndCapStyle): style of end caps (default: round)<br/> *join_style*: (JoinStyle): style of joins (default: round)<br/> *mitre_limit* (float): mitre limit |
| `close() -> Path` | produce a copy of path which is closed | |
| `flip() -> Path` | produce a copy of path with reversed orientation; it doesn't change the way it looks, but the way it is built | |
| `simplify(tolerance:float) -> Path` | simplify path, removing excess points to remain within given tolerance | *tolerance* (float): simplification tolerance |
| `interpolate(step_size) -> Path` | interpolate path with given step size | *step_size* (float): interpolation step size |
| `append(point:Point) -> None` | append a point to path | *point* (Point): point to append |
| `cylindrical(radius:float) -> Path` | create a copy of path projected onto a cylinder aligned with x axis and with given radius | *radius* (float): cylinder radius |
| `tangent_angle(radians:bool=False) -> list[float]` | compute tangent angle for each path point | *radians* (bool): if True, return angles in radians; if False, return angles in degrees (default) |
| `divergence(component:DivComponent) -> list[float]` | compute given divergence matrix component for each path point | *component* (DivComponent): component to compute |
| `simplify_above(limit_height:float) -> Path` | remove points above given limit | *limit_height* (float): height above which path gets simplified |
| `split_above(limit_height:float) -> list[Path]` | split path above given limit; points above limit are discarded | *limit_height* (float): height above which path is split |
| `rearrange(limit_height:float, ref_point:Point) -> Path` | produce a copy of path which starts at discontinuity closest to reference point; is considered a discontinuity a segment that crosses limit height | *limit_height* (float): height used to define discontinuities<br/> *ref_point* (Point): reference point |
| `create_ramps(limit_height:float, ramp_height:float, ramp_length:float, ramp_direction:RampDirection) -> Path` | create ramps on every discontinuity identified using given limit height;  is considered a discontinuity a segment that crosses limit height; for forward ramps, only discontinuities from high to low are considered; for backward ramps, discontinuities from low to high.| *limit_height* (float): height used to define discontinuities<br/> *ramp_height* (float): height of ramp<br/> *ramp_length* (float): length of ramp<br/> *ramp_direction* (RampDirection): forward (for tool extraction), backward (for tool insertion) or both |
| `create_forward_ramps(limit_height:float, ramp_height:float, ramp_length:float) -> Path` | this is a helper method that calls *create_ramps* for forward ramps only; see *create_ramps* for technical details. | *limit_height* (float): height used to define discontinuities<br/> *ramp_height* (float): height of ramp<br/> *ramp_length* (float): length of ramp |
| `create_backward_ramps(limit_height:float, ramp_height:float, ramp_length:float) -> Path` | this is a helper method that calls *create_ramps* for backward ramps only; see *create_ramps* for technical details. | *limit_height* (float): height used to define discontinuities<br/> *ramp_height* (float): height of ramp<br/> *ramp_length* (float): length of ramp |

##### Implemented standard methods

- `__getitem__`: path[n] -> Point, path[m:n] -> list[Point]
- `__setitem__`: path[n] = Point, path[m:n] = list[Point]
- `__iter__`: for p in path: type(p) == Point
- `__len__`: len(path) -> number of points
- `__add__`: path1 + path2 -> concatenate paths
- `__neg__`: -path -> path with negated components
- `__mul__`: path*n -> duplicate path n times
- `__rmul__`: n*path -> like `__mul__`

##### StylePath class (pygraver.render.StylePath)

This is a subclass of the *Path* class that permits storing styling data (color, tool size). It is available at *pygraver.render.StylePath*. It can dynamically update a *Model* object if one is associated at creation through the *model* argument. By design, it is not possible to assign a model after creation.

##### Constructor

```python
StyledPath(xs=numpy.array(), ys=numpy.array(), zs=numpy.array(), cs=numpy.array(), model=None, color=[0,0,0,255])
```

###### Arguments

- *xs* (numpy.array|list[float]): x coordinates (default: numpy.array())
- *ys* (numpy.array|list[float]): y coordinates (default: numpy.array())
- *zs* (numpy.array|list[float]): z coordinates (default: numpy.array())
- *cs* (numpy.array|list[float]): c coordinates (default: numpy.array())
- *model* (pygraver.core.render.Model): associated model (default: None)
- *color* (list[uint8]): path color (default: [0,0,0,255])

#### PathGroup class (pygraver.core.types.PathGroup)

The PathGroup class represents a group of paths. It allows applying operations on all paths at once. Moreover, it comes with a number of methods to improve fabricability.

##### Constructor

```python
PathGroup(paths:list[Path]=[])
```

###### Arguments

- *paths* (list[Path]): paths forming path group (default: empty list)

##### Properties

| Name | Type | Description |
|------|------|-------------|
| `paths` | getter/setter (list[Path]) | direct access to paths |
| `envelope` | getter (list[Path]) | compute envelope of combined paths |
| `steps` | getter/setter (list[Point]) | get/set distance between start points of adjacent paths |
| `rmax` | getter (float) | compute largest distance from path group centroid |
| `centroid` | getter (Point) | compute path group centroid |
| `cartesian` | getter (PathGroup) | create a copy of path group in cartesian coordinates |
| `polar` | getter (PathGroup) | create a copy of path group in polar coordinates |

##### Methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `shift(distance:Point) -> PathGroup` | translate path group by given distance | *distance* (Point): translation vector |
| `scale(factor:float, center:Point) -> PathGroup` | scale path group uniformly around center point | *factor* (float): scaling factor<br/> - *center* (Point): center point |
| `rotate(yaw_angle:float, pitch_angle:float, roll_angle:float) -> PathGroup` | rotate path group around origin | *yaw_angle* (float): yaw angle in degrees <br/> *pitch_angle* (float): pitch angle in degrees<br/> *roll_angle* (float): roll angle in degrees |
| `mirror(along_x:bool, along_y:bool, along_z:bool) -> PathGroup` | mirror path group along specified axes | *along_x* (bool): if True, mirror along x axis<br/> *along_y* (bool): if True, mirror along y axis<br/> *along_z* (bool): if True, mirror along z axis |
| `matrix_transform(components:list) -> PathGroup` | general matrix transform | *components* (list): components of the 4x4 matrix transform (flattened) |
| `inflate(amount:float) -> PathGroup` | inflate path group by given amount | *amount* (float): amount to inflate by |
| `buffer(amount:float, cap_style:EndCapStyle=Round, join_style:JoinStyle=Round, mitre_limit:float=1.0) -> PathGroup` | buffer path group by given amount; this uses libgeos buffering algorithm | *amount* (float): amount to buffer by<br/> *cap_style* (EndCapStyle): style of end caps (default: round)<br/> *join_style*: (JoinStyle): style of joins (default: round)<br/> *mitre_limit* (float): mitre limit |
| `flip() -> PathGroup` | produce a copy of path group with reversed orientation; it doesn't change the way it looks, but the way it is built | |
| `simplify(tolerance:float) -> PathGroup` | simplify path group, removing excess points to remain within given tolerance | *tolerance* (float): simplification tolerance |
| `interpolate(step_size) -> PathGroup` | interpolate path group with given step size | *step_size* (float): interpolation step size |
| `append(path:Path) -> None` | append a path to path group | *path* (Path): path to append |
| `cylindrical(radius:float) -> PathGroup` | create a copy of path group projected onto a cylinder aligned with x axis and with given radius | *radius* (float): cylinder radius |
| `simplify_above(limit_height:float) -> PathGroup` | remove points above given limit | *limit_height* (float): height above which paths get simplified |
| `split_above(limit_height:float) -> list[PathGroup]` | split path group above given limit; points above limit are discarded | *limit_height* (float): height above which paths are split |
| `rearrange(limit_height:float) -> PathGroup` | produce a copy of path group with paths starting at a discontinuity closest to their start; is considered a discontinuity a segment that crosses limit height | *limit_height* (float): height used to define discontinuities |
| `create_ramps(limit_height:float, ramp_height:float, ramp_length:float, ramp_direction:RampDirection) -> PathGroup` | create ramps on every discontinuity identified using given limit height;  is considered a discontinuity a segment that crosses limit height; for forward ramps, only discontinuities from high to low are considered; for backward ramps, discontinuities from low to high.| *limit_height* (float): height used to define discontinuities<br/> *ramp_height* (float): height of ramp<br/> *ramp_length* (float): length of ramp<br/> *ramp_direction* (RampDirection): forward (for tool extraction), backward (for tool insertion) or both |
| `create_forward_ramps(limit_height:float, ramp_height:float, ramp_length:float) -> PathGroup` | this is a helper method that calls *create_ramps* for forward ramps only; see *create_ramps* for technical details. | *limit_height* (float): height used to define discontinuities<br/> *ramp_height* (float): height of ramp<br/> *ramp_length* (float): length of ramp |
| `create_backward_ramps(limit_height:float, ramp_height:float, ramp_length:float) -> PathGroup` | this is a helper method that calls *create_ramps* for backward ramps only; see *create_ramps* for technical details. | *limit_height* (float): height used to define discontinuities<br/> *ramp_height* (float): height of ramp<br/> *ramp_length* (float): length of ramp |
| `sort_paths(ref_point:Point, predicate:SortPredicate) -> PathGroup` | create a copy of path group with paths sorted according to given predicate | *ref_point* (Point): path closest to this point is used as first path<br/> *predicate* (SortPredicate): sorting predicate |
| `reorder(order:list[int]) -> PathGroup` | create a copy of path group with paths reordered according to provided index list | *order* (list[int]): index list |

##### Implemented standard methods

- `__getitem__`: pathgroup[n] -> Path, pathgroup[m:n] -> list[Path]
- `__setitem__`: pathgroup[n] = Path, pathgroup[m:n] = list[Path]
- `__iter__`: for p in pathgroup: type(p) == Path
- `__len__`: len(pathgroup) -> number of paths
- `__add__`: pathgroup1 + pathgroup2 -> concatenate path groups
- `__mul__`: pathgroup*n -> duplicate pathgroup n times
- `__rmul__`: n*pathgroup -> like `__mul__`

#### Surface class (pygraver.core.types.Surface)

The Surface class represents a surface. It is used for two different purposes: calculating toolpaths for milling and masking areas.

##### Constructor

```python
Surface()
Surface(contour:Path)
Surface(contour:Path, holes:list[Path])
Surface(contours:list[Path], holes:list[Path])
Surface(contours:Surface, holes:list[Path])
Surface(contours:Surface, holes:Surface)
```

###### Arguments

- *contour* (Path): path forming surface contour
- *contours* (list[Path]): list of paths forming surface contour (can be disjoint)
- *contours* (Surface): surface used to define surface contour
- *holes* (list[Path]): paths defining hole contours
- *holes* (Surface): surface to define hole contours

| Name | Type | Description |
|------|------|-------------|
| `contours` | getter/setter (list[Path]) | direct access to surface contours |
| `holes` | getter/setter (list[Path]) | direct access to hole boundaries |

##### Methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `combine() -> list[Surface]` | combine surface contours and holes and split optimized result into closed shapes | |
| `contains(point:Point) -> bool` | test if surface contains given point | *point* (Point): point to test |
| `boolean_operation(other:Surface, operation_type:BooleanOperation) -> list[Surface]` | perform selected boolean operation between two surfaces | *other* (Surface): surface to perform operation with<br/> *operation_type* (BooleanOperation): union, difference, symmetric difference or intersection |
| `get_milling_paths(tool_size:float, increment:float) -> list[Path]` | compute paths necessary to mill surface with given tool size and increment | *tool_size* (float): tool size<br/> *increment* (float): increment between paths |
| `get_milled_surface(tool_size:float, increment:float) -> list[Surface]` | compute surface milled with given tool size, approximating original surface | *tool_size* (float): tool size<br/> *increment* (float): increment between paths |
| <code>correct_height(paths:list[Path]\|PathGroup, clearance:float, safe_height:float, outside:bool, fix_boundaries:bool) -> list[Path]</code> | from given paths or path group, produce paths with corrected height in order to either lift tool outside surface (outside=True) or inside (outside=False) | *paths* (list[Path]\|PathGroup): list of paths or path group<br/> *clearance* (float): distance from boundary to start from<br/> *safe_height* (float): height of corrected points<br/> *outside* (bool): if True, paths are corrected outside surface; if False, inside surface<br/> *fix_boundaries* (bool): if True, add points around boundaries to increase accuracy (slower) |

##### Implemented standard methods

- `__add__`: surface1 + surface2 -> boolean union
- `__sub__`: surface1 - surface2 -> boolean difference
- `__mul__`: surface1 * surface2 -> boolean intersection

#### SVG file parser (pygraver.core.svg.File)

It is often convenient to draw models with a vector drawing tool. For this purpose I use Inkscape, therefore files generated with Inkscape will likely work. Other tools may work as well provided that one can produce SVG groups (layers) with them. To prepare your model, create a layer and name it with the name of your choice, then fill it with the shapes you want to use in PyGraver. Coordinates are computed relative to the center of the SVG view box.

##### Constructor

```python
File()
File(file_name:str)
```

###### Arguments

- *file_name* (str): path to the file to parse

##### Methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `open(file_name:str) -> None` | open file | *file_name* (str): path to the file to parse |
| `from_memory(buffer:str) -> None` | open file from buffer | *buffer* (str): content of file to parse |
| `get_size() -> list[float]` | get viewport size (w x h) | |
| `get_paths(layer:str, step_size:float) -> list[Path]` | rasterize paths on given layer | *layer* (str): layer name or id<br/> *step_size* (float): rasterization step size |
| `get_points(layer:str) -> list[Point]` | get centers of ellipses and rectangles on given layer; this is used to generate drill maps | *layer* (str): layer name or id |

###### Limitations

- The parser uses the *viewBox* attribute of the *svg* tag to assess image size. Images generated without it will fail to load.
- SVG styles are not supported.
- For now, holes in shapes won't be rasterized. To generate holes, place them on a separate layer, rasterize them and use the produced paths as holes for a *Surface* object.

### Toolpath rendering

Rendering classes are spread across several submodules.
First of all, submodule *pygraver.core.render* contains the base classes to render toolpaths in 3D. These are extended with specific classes for local rendering in *pygraver.render* and for remote rendering in *pygraver.web*.

#### Model class (pygraver.core.render.Model)

This is the class used to gather elements and render them.

##### Constructor

```python
Model()
```

##### Properties

| Name | Type | Description |
|------|------|-------------|
| `window` | getter (vtkRenderWindow) | underlying VTK render window |
| `renderer` | getter (vtkRenderer) | underlying VTK renderer |
| `background_color` | getter/setter (list[uint8]) | RGB color used as background color |

##### Methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `add_shape(shape:Shape3D) -> None` | add shape to model | *shape* (Shape3D): shape to add |
| `remove_shape(shape:Shape3D) -> None` | remove shape from model | *shape* (Shape3D): shape to remove |
| `has_shape(shape:Shape3D) -> bool` | tell if model has shape | *shape* (Shape3D): shape to test |
| `add_widget(widget:vtkAbstractWidget) -> None` | add widget to model | *widget* (vtkAbstractWidget): widget to add |
| `remove_widget(widget:vtkAbstractWidget) -> None` | remove widget from model | *widget* (vtkAbstractWidget): widget to remove |
| `has_widget(widget:vtkAbstractWidget) -> bool` | test if model has widget | *widget* (vtkAbstractWidget): widget to test |
| `render() -> None` | render model | |

#### DynamicModel class (pygraver.render.DynamicModel)

For animations or dynamic updates, it is necessary that Python hands over priority periodically to VTK in order to render updates. With the *Model* class, this can be done manually by calling the *window.Render()* method. Unfortunately, the default VTK renderer doesn't play well with asyncio or threading. Once called with the *render* method, it won't release priority until terminated. A solution to this issue is the *DynamicModel* class present in *pygraver.render*. As a subclass of the *Model* class, it shares all its constructors, methods and properties. Additionnally, it overloads the *timer_callback* method, which forces VTK to get back to Python. If one uses a renderer that supports multithreading (such as *trame* together with *pygraver.web.WebLayout*), one should use the *DynamicModel* class and define the update routines in separate threads or coroutines. Otherwise, one can subclass the *DynamicModel* class and overload the *timer_callback* method, as demonstrated in *example_3c.py*.

##### Specific methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `timer_callback() -> None` | method called periodically by internal timer; it can be overloaded to implement dynamic updates | |


#### Shape3D class (pygraver.core.render.Shape3D)

The Shape3D class is the base class for shapes displayed in a model. Every shape to be rendered must derive from the Shape3D class. I wrote several subclasses to deal with common cases. It's also possible to create your own in Python by subclassing *pygraver.core.render.Shape3D*.

A shape is composed of one or more objects (vtkActor), the data of which can be set independently.

##### Constructor

```python
Shape3D()
```

##### Properties

| Name | Type | Description |
|------|------|-------------|
| `base_color` | getter/setter (list[uint8]) | RGBA color for default state |
| `highlight_color` | getter/setter (list[uint8]) | RGBA color for highlighted state |
| `label` | getter/setter (str) | shape label |
| `scalar_color_mode` | getter/setter (bool) | if True, scalar color mode is active; if False, default color mode is active |
| `visible` | getter/setter (bool) | if True, shape is visible; if False, shape is hidden |
| `actors` | getter (list[vtkActor]) | access actors that compose shape |

##### Methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `set_item(index:int, polydata:vtkPolyData) -> None` | set data for actor at given index | *index* (int): actor's index<br/> *polydata* (vtkPolyData): data to set |
| `set_scalar_color_range(vmin:float, vmax:float) -> None` | set color range for scalar color mode | *vmin* (float): minimum value<br/> *vmax* (float): maximum value (>=vmin) |
| `get_scalar_color_range() -> list[float]` | get color range for scalar color mode (vmin, vmax) | |
| `set_highlighted(actor:vtkActor, enabled:bool) -> None` | set given actor's state (default or highlighted) | *actor* (vtkActor): actor to act upon (must be member of shape)<br/> *enabled* (bool): if True, enable highlighted mode; if False, disable highlighted mode |
| `set_highlighted(index:int, enabled:bool) -> None` | set state for actor at given index (default or highlighted) | *index* (int): index of actor to act upon<br/> *enabled* (bool): if True, enable highlighted mode; if False, disable highlighted mode |
| `set_highlighted(enabled:bool) -> None` | set state for all actors (default or highlighted) | *enabled* (bool): if True, enable highlighted mode; if False, disable highlighted mode |
| `toggle_highlighted(actor:vtkActor) -> None` | toggle given actor's state (default or highlighted) | *actor* (vtkActor): actor to act upon (must be member of shape) |
| `toggle_highlighted(index:int) -> None` | toggle state for actor at given index (default or highlighted) | *index* (int): index of actor to act upon |
| `toggle_highlighted() -> None` | toggle state for all actors (default or highlighted) | |
| `get_highlighted(actor:vtkActor) -> bool` | get given actor's state (default or highlighted); return True if actor is highlighted, False otherwise | *actor* (vtkActor): actor of which to request state (must be member of shape) |
| `get_highlighted(index:int) -> bool` | get state of actor at given index (default or highlighted); return True if actor is highlighted, False otherwise | *index* (int): index of actor to act upon |
| `toggle_visibility() -> None` | toggle shape visibility | |
| `is_point_inside(point:Point) -> bool` | tell if given point is inside shape | *point* (Point): point to test for |
| `distance_to_actor(actor:vtkActor, point:Point) -> float` | compute distance between given point and actor (must be member of shape) | *actor* (vtkActor): actor to compare with<br/> *point* (Point): point |
| `closest_actor(point:Point) -> (distance:float, actor:vtkActor)` | find actor closest to given point; return distance and found actor | *point* (Point): point |
| `intersecting_actor(point1:Point, point2:Point) -> vtkActor` | find first actor intersected by segment defined by point1 and point2 | *point1* (Point): segment start<br/> *point2* (Point): segment end |
| `get_interactive() -> list[(actor:vtkActor, label:str)]` | get a list of shape actors that can be interacted with; an associated label comes with each actor | |


#### Extrusion subclass (pygraver.core.render.Extrusion)

This is a Shape3D subclass that extrudes a contour linearly.

##### Constructor

```python
Extrusion(contour:Path, length:float, axis:Point, color:list[uint8])
```

###### Arguments

- *contour* (Path): extrusion contour
- *length* (float): extrusion length
- *axis* (Point): vector defining extrusion axis
- *color* (list[uint8]): shape RGBA color

#### Cylinder subclass (pygraver.core.render.Cylinder)

This is a Shape3D subclass that creates a cylinder.

##### Constructor

```python
Cylinder(radius:float, height:float, center:Point, axis:Point, color:list[uint8])
```

###### Arguments

- *radius* (float): cylinder radius
- *height* (float): cylinder height
- *center* (Point): center point
- *axis* (Point): vector defining cylinder axis
- *color* (list[uint8]): shape RGBA color

#### Wire subclass (pygraver.core.render.Wire)

This is a Shape3D subclass that creates a wire along a path.

##### Constructor

```python
Wire(path:Path, diameter:float, color:list[uint8], sides:int)
```

###### Arguments

- *path* (Path): path to extrude along
- *diameter* (float): wire diameter
- *color* (list[uint8]): shape RGBA color
- *sides* (int): number of sides (>=4)

#### WireCollection subclass (pygraver.core.render.WireCollection)

This is a Shape3D subclass that creates wires from a bunch of paths.

##### Constructor

```python
WireCollection(paths:list[Path], diameter:float, color:list[uint8], sides:int)
```

###### Arguments

- *paths* (list[Path]): paths to extrude along
- *diameter* (float): wire diameter
- *color* (list[uint8]): shape RGBA color
- *sides* (int): number of sides (>=4)

#### Marker subclass (pygraver.core.render.Marker)

This is a Shape3D subclass that creates an alphanumeric marker.

##### Constructor

```python
Marker(glyph:str, position:Point, size_x:float, size_y:float, thickness:float, orientation:Point, color:list[uint8])
```

###### Arguments

- *glyph* (str): character
- *position* (Point): marker position
- *size_x* (float): size in x direction
- *size_y* (float): size in y direction
- *thickness* (float): size in z direction
- *orientation* (Point): vector defining extrusion direction
- *color* (list[uint8]): shape RGBA color

#### MarkerCollection subclass (pygraver.core.render.MarkerCollection)

This is a Shape3D subclass that creates a bunch of alphanumeric markers.

##### Constructor

```python
Marker(glyph:str, positions:list[Point], size_x:float, size_y:float, thickness:float, orientation:Point, color:list[uint8])
```

###### Arguments

- *glyph* (str): character
- *positions* (list[Point]): marker positions
- *size_x* (float): size in x direction
- *size_y* (float): size in y direction
- *thickness* (float): size in z direction
- *orientation* (Point): vector defining extrusion direction
- *color* (list[uint8]): shape RGBA color

#### TextButton class (pygraver.render.TextButton)

This is a *vtkTextWidget* subclass that draws a clickable text associated with a *Shape3D* object. Clicking toggles through different states: 1) visible, uniform color; 2) visible, scalar color mode; 3) hidden.

##### Constructor

```python
TextButton(shape:Shape3D)
```

###### Arguments

- *shape* (Shape3D): shape to associate with

#### BalloonText class (pygraver.render.BalloonText)

This is a *vtkBalloonWidget* subclass that causes hovering over an associated *Shape3D* object display the object name while highlighting it. If the object contains more than one actor (e.g. *WireCollection* with multiple wires), it appends the actor's index to the object name.

##### Constructor

```python
BalloonText(shape:Shape3D)
```

###### Arguments

- *shape* (Shape3D): shape to associate with

#### WebLayout class (pygraver.web.WebLayout)

This is a simple web layout to render a *Model* object remotely using [trame](https://kitware.github.io/trame/). You may also be interested in [this addendum](@ref addendum) to configure your server adequately.

##### Constructor

```python
WebLayout(server:trame.app.Server, window:vtkRenderWindow)
```

Note that construction must be done in the following way:

```python
with WebLayout(server, window) as layout:
    # fill layout here; more details on how to compose layouts
    # can be found here: https://kitware.github.io/trame/docs/tutorial.html
```

###### Arguments

- *server* (trame.app.Server): trame server
- *window* (vtkRenderWindow): window to render through server

##### Methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `async refresh_function(**kwargs) -> None` | this function is called on layout creation; one can overload it to implement a refresh loop | |
| `refresh() -> None` | force rendering | |

### Machine control

This submodule contains classes to communicate with a CNC machine.

#### Machine class (pygraver.machine.Machine)

This is a basic machine control class that communicates with a RepRap-compatible firmware. Most methods are asynchronous. A synchronous class is also available (see below).

##### Constructor

```python
Machine(port:str="")
```

###### Arguments

- *port* (str): logical serial port path (default: "")

##### Properties

| `port` | getter/setter (str) | logical serial port path |
| `timeout` | getter/setter (float) | serial timeout, in seconds |
| `serial_baud_rate` | getter/setter (int) | baud rate for serial line |
| `term_char` | getter/setter (str) | termination character for serial communication |
| `response_ok` | getter/setter (str) | OK response string for serial communication |
| `tool_size` | getter/setter (float) | tool size, for display purpose |
| `feed_rate` | getter/setter (float) | machine feed rate |
| `model` | getter/setter (pygraver.core.render.Model | None) | associated rendering model for display |

##### Asynchronous methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `open() -> bool` | open serial connection; return True if succesful | |
| <code>close(timeout:float\|None=None) -> bool</code> | close serial connection; return True if succesful | *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>write(cmd:str, timeout:float\|None=None) -> None</code> | send command | *cmd* (str): command string<br/> *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>readline(timeout:float\|None=None) -> bytes</code> | read one data line; return line read if succesful, or None if failed | *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>wait_answer(n_lines:int=1, timeout:float\|None=None) -> list[bytes]</code> | read *n_lines* data lines; return lines read if succesful, or None if failed | *n_lines* (int): number of lines to read (default: 1)<br/> *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>ask(cmd:str, n_lines:int=1, timeout:float\|None=None) -> list[bytes]</code> | send given command and read *n_lines* data lines; return lines read if succesful, or None if failed | *cmd* (str): command string<br/> *n_lines* (int): number of lines to read (default: 1)<br/> *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>wait(timeout:float\|None=None) -> bool</code> | wait until machine is ready; return True if succesful | *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>get_position(timeout:float\|None=None) -> Point</code> | request machine position | *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>set_position(timeout:float\|None=None, **kwargs) -> bool</code> | set machine position; position must be set as named arguments that match axes names (i.e. x=..., y=..., z=..., c=...); return True if operation is successful | *timeout* (float\|None): operation timeout (default: None = infinite timeout)<br/> *x*, *y*, *z*, *c* (float): coordinates |
| <code>set_position(position:Point, timeout:float\|None=None) -> bool</code> | set machine position; return True if operation is successful | *position* (Point): new position<br/> *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>move(relative:bool=False, timeout:float\|None=None, **kwargs) -> bool</code> | move machine to given position; position must be set as named arguments that match axes names (i.e. x=..., y=..., z=..., c=...); return True if succesful | *relative* (bool): if True, position is set relative to current position; if False, position is absolute<br/>*timeout* (float\|None): operation timeout (default: None = infinite timeout)<br/> *x*, *y*, *z*, *c* (float): coordinates |
| <code>move(position:Point, relative:bool=False, timeout:float\|None=None) -> bool</code> | move machine to given position; return True if succesful | *position* (Point): new position<br/> *relative* (bool): if True, position is set relative to current position; if False, position is absolute<br/> *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>abs_move(timeout:float\|None=None, **kwargs) -> bool</code> | move machine to given absolute position; position must be set as named arguments that match axes names (i.e. x=..., y=..., z=..., c=...); return True if succesful | *timeout* (float\|None): operation timeout (default: None = infinite timeout)<br/> *x*, *y*, *z*, *c* (float): coordinates |
| <code>abs_move(position:Point, timeout:float\|None=None, **kwargs) -> bool</code> | move machine to given absolute position; return True if succesful | *position* (Point): new position<br/> *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>rel_move(timeout:float\|None=None, **kwargs) -> bool</code> | move machine to given relative position; position must be set as named arguments that match axes names (i.e. x=..., y=..., z=..., c=...); return True if succesful | *timeout* (float\|None): operation timeout (default: None = infinite timeout)<br/> *x*, *y*, *z*, *c* (float): coordinates |
| <code>rel_move(position:Point, timeout:float\|None=None, **kwargs) -> bool</code> | move machine to given relative position; return True if succesful | *position* (Point): new position<br/> *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>probe_endstops(timeout:float\|None=None) -> dict</code> | probe endstops state; return a dictionnary with one entry per axis and boolean values indicating endstop state | *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>switch_motors(state:bool, timeout:float\|None=None) -> bool</code> | switch machine motors on or off; return True if operation is succesful | *state* (bool): True to enable motors, False to disable<br/> *timeout* (float\|None): operation timeout (default: None = infinite timeout) |
| <code>trace(path:types.Path\|None=None, xs:'list[float]\|None'=None, ys:'list[float]\|None'=None, zs:'list[float]\|None'=None, cs:'list[float]\|None'=None, timeout:float\|None=None) -> bool</code> | make machine to trace given path | *path* (types.Path): path to trace; if given, takes precedence over other arguments<br/> *xs*, *ys*, *zs*, *cs* (list[float]\|None): coordinate vector for matching axis; if more than one is given, must be of the same length<br/> *timeout* (float\|None): operation timeout (default: None = infinite timeout) |

##### Synchronous methods

| Name | Description | Arguments |
|------|-------------|-----------|
| `enable_endstops() -> None` | enable machine endstops for future commands | |
| `disable_endstops() -> None` | disable machine endstops for future commands | |

#### SyncMachine class (pygraver.machine.SyncMachine)

This is essentially the same as the *Machine* class, but entirely synchronous. It relies on the machine class to operate. Every properties and methods of the *Machine* class are available as synchronous equivalents.

##### Additional properties

| Name | Type | Description |
|------|------|-------------|
| `endstops` | getter (dict) | return a dictionnary with one entry per axis and boolean values indicating endstop state |
| `position` | getter (types.Point) / setter (list\|tuple\|dict\|types.Point) | get/set machine position; as a setter, if using *list*, or *tuple* argument, it must contain one value per axis; if using *dict* argument, it must contain keys named after axes names and float values |

## Examples

Some examples are available in the *examples* folder. Here is a short description of what they cover.

| Example name | Covered items | Screenshot |
|--------------|---------------|------------|
| *example1.py* | - load shapes from a SVG file<br/> - create Surface, Path and PathGroup objects<br/> - arrange paths in path group and mask them with surface | |
| *example2.py* | extends example 1 and shows how to display result with Matplotlib | <div style="text-align:center"><img src="https://user-images.githubusercontent.com/6388158/185990541-0da58b84-d4ec-4f4a-ab29-c31111d2ca8f.png" alt="Screenshot of example2.py" style="max-width:33vw"></div> |
| *example3.py* | extends example 1 and shows how to render result with VTK | <div style="text-align:center"><img src="https://user-images.githubusercontent.com/6388158/185990558-782c2c51-bd10-4e11-bf39-1b8eb357093a.png" alt="Screenshot of example3.py" style="max-width:33vw"></div> |
| *example3b.py* | a variation of example3 with paths corrected in such a way to remove points outside surface and add insertion and extraction ramps | <div style="text-align:center"><img src="https://user-images.githubusercontent.com/6388158/185990567-cd72dde4-8851-4515-acd2-5254036571ac.png" alt="Screenshot of example3b.py" style="max-width:33vw"></div> |
| *example3c.py* | A variation of example3 that shows how to dynamically update a model using the default renderer. It takes the paths rendered in example3 and add them to the model with >=1.0 second interval. | |
| *example3d.py* | a variation of example3 which demonstrates the use of widgets | <div style="text-align:center"><img src="https://user-images.githubusercontent.com/6388158/185990593-b3200ab6-d440-46f2-9df6-5f546714e8a7.png" alt="Screenshot of example3d.py" style="max-width:33vw"></div> |
| *example4.py* | shows how to render a model remotely using trame | <div style="text-align:center"><img src="https://user-images.githubusercontent.com/6388158/185990621-49281927-f901-490b-b069-2d821edd4501.png" alt="Screenshot of example4.py" style="max-width:33vw"></div> |
| *example4b.py* | A variation of example4.py which shows how to dynamically update a model rendered remotely using trame. It adds one path every second to the model. | |
| *example5.py* | shows how to send paths generated in example1 to a machine | |

## Addendum - configuring nginx for remote access

While this is not specific to PyGraver but rather to trame, I describe here how to configure [nginx](https://www.nginx.com/) to give access to trame server remotely (in this example, access is given on default http port 80). With the default trame configuration (port 8080), this is what should be added to the *nginx.conf* file in the *http* section:

```nginx
    # for websocket connection upgrade
    map $http_upgrade $connection_upgrade {
        default upgrade;
        ''      close;
    }

    server {
        listen 80;
        server_name localhost;
        root /usr/share/nginx;
        
        location / {
            proxy_pass   http://localhost:8080;
            add_header Access-Control-Allow-Origin '*';
            proxy_pass_header Server;
            proxy_set_header Host $http_host;
            proxy_redirect off;
            proxy_set_header X-Forwarded-For  $remote_addr;
            proxy_set_header X-Scheme $scheme;
            proxy_set_header X-Forwarded-Protocol $scheme;
            proxy_connect_timeout 50;
            proxy_read_timeout 50;
        }

        # for websockets        
        location /ws {
            proxy_pass http://localhost:8080/ws;
            proxy_http_version 1.1;
            proxy_set_header Upgrade $http_upgrade;
            proxy_set_header Connection $connection_upgrade;
        }
    }
```
