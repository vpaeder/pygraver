# -*- coding: utf-8 -*-
'''
Web submodule. It contains a basic layout to display rendered toolpaths remotely using trame.
'''
from trame.ui.vuetify import SinglePageLayout
from trame.widgets import vuetify, vtk
from trame.app import asynchronous, Server

from vtkmodules.vtkRenderingCore import vtkRenderWindow


class WebLayout(SinglePageLayout):
    '''
    A basic web layout class which is made to update its content dynamically.

    Note that the correct manner to instantiate this class is:

        with WebLayout(args...) as layout:
            pass
    
    Using the classical assignment syntax (layout = WebLayout(...)) won't work.
    '''
    @asynchronous.task
    async def refresh_function(self, **kwargs):
        '''
        Refresh callback function triggered on server startup.
        Sub-class WebLayout and define your own update loop by overloading
        this function.
        '''
        pass

    def __init__(self, server:Server, window: vtkRenderWindow) -> None:
        '''
        Constructor.
        
        Args:
            server (Server): trame server object.
            window (vtkRenderWindow): window to display on server.
        '''
        super().__init__(server)
        ctrl = server.controller
        self.title.set_text("PyGraver")
        self.icon.click = ctrl.view_update
        # toolbar
        with self.toolbar:
            # toolbar components
            vuetify.VSpacer()
            vuetify.VDivider(vertical=True, classes="mx-2")
            with vuetify.VBtn(icon=True, click="$refs.view.resetCamera()"):
                vuetify.VIcon("mdi-crop-free")
        
        with self.content:
            # content components
            with vuetify.VContainer(
                fluid=True,
                classes="pa-0 fill-height",
            ):
                view = vtk.VtkLocalView(window, namespace="view")
                ctrl.view_update = view.update
                ctrl.view_reset_camera = view.reset_camera
                ctrl.on_server_ready.add(view.update)
        
        ctrl.on_server_ready.add(self.refresh_function)
        ctrl.flush_content = self.flush_content
    
    def refresh(self) -> None:
        '''Force rendering.
        '''
        self.server.controller.view_update()
        self.server.state.flush()
