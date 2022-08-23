/** \file model.cpp
 *  \brief Implementation file for Model class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <vtkRenderWindowInteractor.h>
#include <vtkCollectionIterator.h>

#include <pybind11/stl.h>

#include "../log.h"

#include "../types/point.h"
#include "../types/path.h"

#include "marker.h"
#include "extrusion.h"

#include "model.h"
#include "vtkevents.h"


namespace pygraver::render {

    using namespace pygraver::types;

    void Model::create_window() {
        // create window
        this->window = vtkSmartPointer<vtkRenderWindow>::New();
        auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        auto interstyle = vtkSmartPointer<vtkCustomInteractorStyle>::New();
        interactor->SetRenderWindow(this->window);
        interstyle->SetCurrentRenderer(this->renderer);
        interstyle->set_model(this);
        interactor->SetInteractorStyle(interstyle);
        this->window->AddRenderer(this->renderer);
        this->window->SetFullScreen(1);
        // initialize interactor
        interactor->Initialize();
        // create refresh callback
        auto refresh_callback = vtkSmartPointer<vtkRefreshCallback>::New();
        refresh_callback->set_model(this);
        this->window->AddObserver(vtkCommand::RenderEvent, refresh_callback);
        // create timer callback
        auto timer_callback  = vtkSmartPointer<vtkTimerCallback>::New();
        timer_callback->set_model(this);
        interactor->AddObserver(vtkCommand::TimerEvent, timer_callback);
        interactor->CreateRepeatingTimer(10);
    }


    Model::Model() {
        PYG_LOG_V("Creating 3D model 0x{:x}", (uint64_t)this);
        // create renderer
        this->renderer = vtkSmartPointer<vtkRenderer>::New();
        if (this->renderer == nullptr)
            throw std::runtime_error("Cannot allocate memory for VTK renderer.");
    }


    Model::~Model() {
        PYG_LOG_V("Deleting 3D model 0x{:x}", (uint64_t)this);
    }


    void Model::set_background_color(const std::vector<uint8_t> & color) {
        if (color.size()!=3)
            throw std::invalid_argument("Color must be a 3-component vector.");
        this->bg_color = color;
        // set background color
        this->renderer->SetBackground(double(this->bg_color[0])/255.0,
                                      double(this->bg_color[1])/255.0,
                                      double(this->bg_color[2])/255.0);
    }


    void Model::add_shape(std::shared_ptr<Shape3D> shape) {
        if (this->has_shape(shape)) return;
        // add shape actors to model and store shape
        shape->get_actors()->InitTraversal();
        auto actor = dynamic_cast<vtkActor*>(shape->get_actors()->GetNextProp());
        while (actor != nullptr) {
            this->renderer->AddActor(actor);
            actor = dynamic_cast<vtkActor*>(shape->get_actors()->GetNextProp());
        }
        this->shapes.emplace_back(shape);
    }


    void Model::remove_shape(std::shared_ptr<Shape3D> shape) {
        if (this->has_shape(shape)) {
            this->shapes.erase(std::find(this->shapes.begin(), this->shapes.end(), shape));
            shape->get_actors()->InitTraversal();
            auto actor = dynamic_cast<vtkActor*>(shape->get_actors()->GetNextProp());
            while (actor != nullptr) {
                this->renderer->RemoveActor(actor);
                actor = dynamic_cast<vtkActor*>(shape->get_actors()->GetNextProp());
            }
        }
    }


    bool Model::has_shape(std::shared_ptr<Shape3D> shape) {
        auto result = std::find(this->shapes.begin(), this->shapes.end(), shape);
        return result != this->shapes.end();
    }


    void Model::add_widget(vtkSmartPointer<vtkAbstractWidget> widget) {
        if (this->has_widget(widget)) return;
        if (this->window == nullptr) this->create_window();
        widget->SetInteractor(this->window->GetInteractor());
        widget->SetEnabled(1);
        this->widgets.emplace_back(widget);
    }


    void Model::remove_widget(vtkSmartPointer<vtkAbstractWidget> widget) {
        if (this->has_widget(widget))
            this->widgets.erase(std::find(this->widgets.begin(), this->widgets.end(), widget));
    }


    bool Model::has_widget(vtkSmartPointer<vtkAbstractWidget> widget) {
        auto result = std::find(this->widgets.begin(), this->widgets.end(), widget);
        return result != this->widgets.end();
    }


    void Model::render() {
        if (this->renderer == nullptr || this->window == nullptr)
            this->create_window();
        this->renderer->ResetCamera();
        this->window->Render();
        this->window->GetInteractor()->Start();
        this->window->Finalize();
    }


    void py_model_exports(py::module_ & mod) {
        py::class_<Model, std::shared_ptr<Model>, PyModel>(mod, "Model")
            .def(py::init<>())
            .def("add_shape", &Model::add_shape, py::arg("shape"))
            .def("remove_shape", &Model::remove_shape, py::arg("shape"))
            .def("has_shape", &Model::has_shape, py::arg("shape"))
            .def_property("background_color", &Model::get_background_color, &Model::set_background_color)
            .def("render", &Model::render)
            .def_property_readonly("renderer", &Model::get_renderer, py::return_value_policy::reference)
            .def_property_readonly("window", &Model::get_render_window, py::return_value_policy::reference)
            .def("add_widget", &Model::add_widget, py::arg("widget"))
            .def("remove_widget", &Model::remove_widget, py::arg("widget"))
            .def("has_widget", &Model::has_widget, py::arg("widget"))
            ;
    }

}