# -*- coding: utf-8 -*-
'''
Rendering submodule. It contains helper classes to display and track toolpaths.
'''
from .core import types, render
import numpy as np

from vtkmodules.vtkInteractionWidgets import vtkTextWidget, vtkTextRepresentation, vtkBalloonWidget, vtkBalloonRepresentation
from vtkmodules.vtkCommonCore import vtkCommand, vtkObject
from vtkmodules.vtkRenderingCore import vtkTextActor


class StyledPath(types.Path):
    '''
    Styled version of types.Path. This allows attaching style attributes
    to a Path object. If one provides a model during initialization, path
    updates get propagated to model.
    
    Attributes:
        tool_size (float): tool size in relative units
        color (list[int]): path color
        _shape (render.Shape3D or None): associated shape
        _model (render.Model or None): associated model
    '''
    tool_size = 1.0
    color = [0, 0, 0, 255]
    shape = None
    _model = None

    def __init__(self, xs=np.array([]), ys=np.array([]), zs=np.array([]), cs=np.array([]), model:render.Model|None=None, tool_size:float=1.0, color:'list[int]'=[0, 0, 0, 255]) -> None:
        '''
        Constructor.
        
        Args:
            xs (np.array): points for x axis
            ys (np.array): points for y axis
            zs (np.array): points for z axis
            cs (np.array): points for c axis
            model (render.Model or None): model to create 3D shape for
            color (list[int]): RGBA color (with 8-bit components)
        '''
        super().__init__(xs=xs, ys=ys, zs=zs, cs=cs)
        self.color = color
        self._model = model
        self.tool_size = tool_size
        if model is not None:
            shape = render.Wire(self, self.tool_size, self.color)
            shape.label = "Tool: {}".format(self.tool_size)
            model.add_shape(shape)
            self.shape = shape
    
    def __del__(self):
        '''
        Destructor.
        '''
        if self.shape is not None:
            self._model.remove_shape(self.shape)
    
    def append(self, pt:types.Point) -> None:
        '''
        Append point to path.
        
        Args:
            pt (types.Point): point to append
        '''
        super().append(pt)
        if self.shape is not None:
            self.shape.set_path(self, self.tool_size, self.color)
    
    def __setitem__(self, idx:int, value:'types.Point|list[types.Point]') -> None:
        '''
        Set item at index with given value.
        
        Args:
            idx (int or slice): index or slice
            value (types.Point or list[types.Point]): value(s) to set
        '''
        super().__setitem__(idx, value)
        if self.shape is not None:
            self.shape.set_path(self, self.tool_size, self.color)
    
    def _copy(self, p:types.Path):
        '''
        Create a StylePath instance from a Path object and copies associated properties.
        
        Args:
            p (types.Path): source path
        '''
        new_path = StyledPath(xs=p.xs, ys=p.ys, zs=p.zs, cs=p.cs, model=self._model, tool_size=self.tool_size, color=self.color)
        return new_path

    def simplify_above(self, limit_height:float):
        '''
        Remove points above given limit.
        
        Args:
            limit_height (float): height above which path gets simplified
        
        Returns:
            StyledPath: simplified path
        '''
        return self._copy(super().simplify_above(limit_height))
    
    def split_above(self, limit_height:float):
        '''
        Split path above given limit; points above limit are discarded.
        
        Args:
            limit_height (float): height above which path is split
        
        Returns:
            list[StyledPath]: split paths
        '''
        paths = super().split_above(limit_height)
        return [StyledPath(xs=p.xs, ys=p.ys, zs=p.zs, cs=p.cs, model=self._model, tool_size=self.tool_size, color=self.color) for p in paths]

    def create_ramps(self, limit_height:float, increase:float, ramp_length:float, ramp_direction:types.RampDirection):
        '''
        Split path above given limit; points above limit are discarded.
        
        Args:
            limit_height (float): height used to define discontinuities
            increase (float): height of ramp
            ramp_length (float): length of ramp
            ramp_direction (types.RampDirection): forward (for tool extraction), backward (for tool insertion) or both

        Returns:
            StyledPath: path with ramps
        '''
        return self._copy(super().create_ramps(limit_height, increase, ramp_length, ramp_direction))

    def create_backward_ramps(self, limit_height:float, increase:float, ramp_length:float):
        '''
        This is a helper method that calls create_ramps for backward ramps only; see create_ramps for technical details.
        
        Args:
            limit_height (float): height used to define discontinuities
            increase (float): height of ramp
            ramp_length (float): length of ramp

        Returns:
            StyledPath: path with ramps
        '''
        return self._copy(super().create_backward_ramps(limit_height, increase, ramp_length))

    def create_forward_ramps(self, limit_height:float, increase:float, ramp_length:float):
        '''
        This is a helper method that calls create_ramps for forward ramps only; see create_ramps for technical details.
        
        Args:
            limit_height (float): height used to define discontinuities
            increase (float): height of ramp
            ramp_length (float): length of ramp

        Returns:
            StyledPath: path with ramps
        '''
        return self._copy(super().create_forward_ramps(limit_height, increase, ramp_length))

    def rearrange(self, limit_height:float, ref_point:types.Point):
        '''
        Produce a copy of path which starts at discontinuity closest to reference point; is considered a discontinuity a segment that crosses limit height.
        
        Args:
            limit_height (float): height used to define discontinuities
            ref_point (types.Point): reference point

        Returns:
            StyledPath: rearranged path
        '''
        return self._copy(super().rearrange(limit_height, ref_point))

    def copy(self):
        '''
        Create copy of path.

        Returns:
            StyledPath: copy of path
        '''
        return self._copy(super().copy())

    def shift(self, vector:types.Point):
        '''
        Translate path by given distance.
        
        Args:
            vector (types.Point): translation vector

        Returns:
            StyledPath: translated path
        '''
        return self._copy(super().shift(vector))

    def scale(self, factor:float, center:types.Point):
        '''
        Scale path uniformly around center point.
        
        Args:
            factor (float): scaling factor
            center (types.Point): center point

        Returns:
            StyledPath: scaled path
        '''
        return self._copy(super().scale(factor, center))

    def mirror(self, along_x:bool, along_y:bool, along_z:bool):
        '''
        Mirror path along specified axes.
        
        Args:
            along_x (bool): if True, mirror along x axis
            along_y (bool): if True, mirror along y axis
            along_z (bool): if True, mirror along z axis

        Returns:
            StyledPath: mirrored path
        '''
        return self._copy(super().mirror(along_x, along_y, along_z))

    def rotate(self, yaw_angle:float, pitch_angle:float, roll_angle:float):
        '''
        Rotate path around origin.
        
        Args:
            yaw_angle (float): yaw angle in degrees
            pitch_angle (float): pitch angle in degrees
            roll_angle (float): roll angle in degrees

        Returns:
            StyledPath: rotated path
        '''
        return self._copy(super().rotate(yaw_angle, pitch_angle, roll_angle))

    def matrix_transform(self, components:'list[float]'):
        '''
        General matrix transform.
        
        Args:
            components (list): components of the 4x4 matrix transform (flattened)

        Returns:
            StyledPath: transformed path
        '''
        return self._copy(super().matrix_transform(components))

    def inflate(self, amount:float):
        '''
        Inflate path by given amount.
        
        Args:
            amount (float): amount to inflate by

        Returns:
            StyledPath: inflated path
        '''
        return self._copy(super().inflate(amount))

    def simplify(self, tolerance:float):
        '''
        Simplify path, removing excess points to remain within given tolerance.
        
        Args:
            tolerance (float): simplification tolerance

        Returns:
            StyledPath: simplified path
        '''
        return self._copy(super().simplify(tolerance))

    def buffer(self, amount:float, cap_style:types.EndCapStyle, join_style:types.JoinStyle, mitre_limit:float):
        '''
        Buffer path by given amount; this uses libgeos buffering algorithm.
        
        Args:
            amount (float): amount to buffer by
            cap_style (EndCapStyle): style of end caps (default: round)
            join_style: (JoinStyle): style of joins (default: round)
            mitre_limit (float): mitre limit

        Returns:
            StyledPath: buffered path
        '''
        return self._copy(super().buffer(amount, cap_style, join_style, mitre_limit))

    def close(self):
        '''
        Produce a copy of path which is closed.

        Returns:
            StyledPath: closed path
        '''
        return self._copy(super().close())

    def flip(self):
        '''
        Produce a copy of path with reversed orientation; it doesn't change the way it looks, but the way it is built.

        Returns:
            StyledPath: flipped path
        '''
        return self._copy(super().flip())

    def interpolate(self, step_size:float):
        '''
        Interpolate path with given step size.

        Args:
            step_size (float): interpolation step size
        
        Returns:
            StyledPath: interpolated path
        '''
        return self._copy(super().interpolate(step_size))

    def cylindrical(self, radius:float):
        '''
        Create a copy of path projected onto a cylinder aligned with x axis and with given radius.

        Args:
             radius (float): cylinder radius
        
        Returns:
            StyledPath: transformed path
        '''
        return self._copy(super().cylindrical(radius))
    
    @property
    def cartesian(self):
        '''
        Create a copy of path in cartesian coordinates.

        Returns:
            StyledPath: path in cartesian coordinates
        '''
        return self._copy(super().cartesian)

    @property
    def polar(self):
        '''
        Create a copy of path in polar coordinates.

        Returns:
            StyledPath: path in polar coordinates
        '''
        return self._copy(super().polar)

    def __add__(self, other:types.Path|types.Point):
        '''
        Concatenate path with other path.

        Args:
             other (types.Path): other path.

        Returns:
            StyledPath: concatenated paths
        '''
        return self._copy(super().__add__(other))

    def __neg__(self):
        '''
        Produce path with negated components.

        Returns:
            StyledPath: path with negated components
        '''
        return self._copy(super().__neg__())

    def __mul__(self, count:int):
        '''
        Duplicate path *count* times.

        Args:
             count (int): number of times to replicate path.

        Returns:
            StyledPath: replicated path.
        '''
        return self._copy(super().__mul__(count))

    def __rmul__(self, count:int):
        '''
        Duplicate path *count* times.

        Args:
             count (int): number of times to replicate path.

        Returns:
            StyledPath: replicated path.
        '''
        return self._copy(super().__rmul__(count))


class DynamicModel(render.Model):
    '''
    Rendering model with callback triggering update.
    This is necessary for dynamic models (e.g. animations).
    '''
    def timer_callback(self) -> None:
        '''
        Function periodically called by underlying Model class.
        '''
        self.window.Render()
    

class TextButton(vtkTextWidget):
    '''
    Button widget that displays some text and which action
    affects associated shape's display state.
    '''
    def callback(self, object:vtkObject, event:int) -> None:
        '''
        Callback function triggered when button gets clicked.
        Flips through available states, which are here:
        #   1) associated shape is visible, color mode is plain color
        #   2) associated shape is visible, scalar color mode
        #   3) associated shape is invisible
        
        Args:
            object (vtkObject): object triggering action (required by VTK)
            event (int): event code
        '''
        repr = self.GetRepresentation()
        props = repr.GetTextActor().GetTextProperty()
        if (self.shape.visible and self.shape.scalar_color_mode):
            props.SetBold(0)
            props.SetItalic(0)
            self.shape.visible = False
            self.shape.scalar_color_mode = False
        elif (self.shape.visible and not self.shape.scalar_color_mode):
            props.SetItalic(1)
            self.shape.scalar_color_mode = True
        else:
            props.SetBold(1)
            self.shape.visible = True
        
        self.GetInteractor().GetRenderWindow().Render()
        
    def __init__(self, shape:render.Shape3D) -> None:
        '''
        Constructor.
        
        Args:
            shape (render.Shape3D): associated shape
        '''
        self.shape = shape
        actor = vtkTextActor()
        actor.SetInput(shape.label)
        # text styling
        repr = vtkTextRepresentation()
        repr.SetPaddingTop(20)
        repr.SetPaddingBottom(10)
        repr.SetPaddingRight(50)
        repr.SetPaddingLeft(50)
        repr.SetCornerRadiusStrength(0.0)
        repr.SetBorderThickness(0.0)
        repr.GetPosition2Coordinate().SetValue(1.0, 0.05)
        repr.SetPolygonRGBA(1,1,1,0.8)
        repr.SetShowBorder(True) # show background polygon
        self.SetRepresentation(repr)
        self.SetTextActor(actor)
        # set style based on shape properties
        prop = repr.GetTextActor().GetTextProperty()
        prop.SetUseTightBoundingBox(1)
        prop.SetJustificationToRight()
        prop.ShadowOn()
        prop.SetFontSize(24)
        if (self.shape.visible and self.shape.scalar_color_mode):
            prop.SetBold(1)
            prop.SetItalic(1)
        elif (self.shape.visible and not self.shape.scalar_color_mode):
            prop.SetBold(1)
            prop.SetItalic(0)
        elif (not self.shape.visible):
            prop.SetBold(0)
            prop.SetItalic(0)
        prop.SetColor([v/255.0 for v in shape.base_color[:3]])
        
        self.AddObserver(vtkCommand.WidgetActivateEvent, self.callback)


class BalloonText(vtkBalloonWidget):
    '''
    Balloon text widget that displays a legend when hovering above
    some elements of a shape with the mouse pointer.
    '''
    def callback(self, object:vtkObject, event:int) -> None:
        '''
        Callback function triggered when linked object gets hovered.
        
        Args:
            object (vtkObject): object triggering action (required by VTK)
            event (int): event code
        '''
        actor = self.GetCurrentProp()
        if (actor == None):
            return
        if self.shape.actors.count(actor)>0:
            if event == "TimerEvent":
                pass
                self.shape.set_highlighted(actor, True)
            elif event == "EndInteractionEvent":
                pass
                self.shape.set_highlighted(actor, False)
        # force render
        self.GetInteractor().GetRenderWindow().Render()
    
    def __init__(self, shape:render.Shape3D) -> None:
        '''
        Constructor.
        
        Args:
            shape (render.Shape3D): associated shape
        '''
        self.shape = shape
        repr = vtkBalloonRepresentation()
        repr.SetBalloonLayoutToImageRight()
        repr.SetPadding(20)
        repr.SetHandleSize(0)
        repr.GetFrameProperty().SetColor(1,1,1)
        repr.GetFrameProperty().SetOpacity(0.8)
        self.SetRepresentation(repr)
        self.SetTimerDuration(100)
        # set style
        prop = repr.GetTextProperty()
        prop.ShadowOn()
        prop.BoldOn()
        prop.SetFontSize(48)
        prop.SetBackgroundOpacity(0)
        prop.SetFrameWidth(0)
        prop.SetUseTightBoundingBox(1)
        color = shape.base_color
        prop.SetColor(color[0],color[1],color[2])
        prop.SetOpacity(1.0)
        # add callbacks
        self.AddObserver(vtkCommand.TimerEvent, self.callback)
        self.AddObserver(vtkCommand.EndInteractionEvent, self.callback)
        # register shapes
        for actor, label in shape.get_interactive():
            self.AddBalloon(actor, label)
