/** \file shape3d.cpp
 *  \brief Implementation file for Shape3D base class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <vtkUnsignedCharArray.h>
#include <vtkCollectionIterator.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkActor.h>
#include <vtkMapper.h>
#include <vtkPointData.h>
#include <vtkImplicitPolyDataDistance.h>

#include <vtkProperty.h>

#include <vtkProperty.h>
#include <vtkInformation.h>
#include <vtkInformationIntegerKey.h>

#include <vtkCellLocator.h>
#include <vtkMath.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkPoints.h>

#include <pybind11/stl.h>

#include "vtkpybind.h"

#include <fmt/core.h>

#include "../log.h"
#include "shape3d.h"

namespace pygraver::render {

    void Shape3D::flip_colors(vtkSmartPointer<vtkPolyData> polydata) {
        auto pointdata = polydata->GetPointData();
        auto colors = pointdata->GetScalars("Colors");
        auto inv_colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
        inv_colors->SetNumberOfComponents(3);
        inv_colors->SetNumberOfValues(colors->GetNumberOfValues());
        inv_colors->SetName("Colors");
        for (int i = 0; i < polydata->GetNumberOfPoints(); i++) {
            double c_h, c_s, c_v;
            auto rgb = colors->GetTuple3(i);
            vtkMath::RGBToHSV(rgb[0]/255, rgb[1]/255, rgb[2]/255, &c_h, &c_s, &c_v);
            c_h = std::fmod(c_h+0.5, 1);
            vtkMath::HSVToRGB(c_h, c_s, c_v, &rgb[0], &rgb[1], &rgb[2]);
            inv_colors->SetTuple3(i, rgb[0]*255, rgb[1]*255, rgb[2]*255);
        }
        pointdata->SetScalars(inv_colors);
        pointdata->SetActiveScalars("Colors");
    }

    std::vector<double> Shape3D::make_highlight_color(const double color[4]) {
        // convert to LAB to apply highlight transformation
        double lab[3], rgb[3];
        std::vector<double> new_color;
        new_color.reserve(4);
        vtkMath::RGBToLab(color, lab);
        lab[0] += (lab[0]<=50) ? 50 : -50;
        for (auto i=1; i<3; i++)
            lab[i] += (lab[i]<=0) ? 127 : -127;
        vtkMath::LabToRGB(lab, rgb);

        for (auto i=0; i<3; i++) 
            new_color.emplace_back(rgb[i]);
        
        new_color.emplace_back(color[3] + ((color[3]<=0.5) ? 0.3 : -0.3));
        return new_color;
    }


    std::vector<uint8_t> Shape3D::make_highlight_color(const std::vector<uint8_t> & color) {
        if (color.size()!=3 && color.size()!=4)
            throw std::invalid_argument("Color must be a 3 or 4-component vector.");
        double old_color[4] = {double(color[0])/255.0, double(color[1])/255.0, double(color[2])/255.0, 255};
        if (color.size()>3)
            old_color[3] = double(color[3])/255.0;
        
        auto new_color = Shape3D::make_highlight_color(old_color);
        return {
            static_cast<uint8_t>(new_color[0]*255),
            static_cast<uint8_t>(new_color[1]*255),
            static_cast<uint8_t>(new_color[2]*255),
            static_cast<uint8_t>(new_color[3]*255)
        };
    }

    Shape3D::Shape3D() {
        PYG_LOG_V("Creating 3D shape 0x{:x}", (uint64_t)this);
        const char* name = "highlighted";
        const char* location = "Shape3D";
        this->highlight_key = vtkSmartPointer<vtkInformationIntegerKey>::Take(new vtkInformationIntegerKey(name, location));
        this->actors = vtkSmartPointer<vtkActorCollection>::New();
        this->lut = vtkSmartPointer<vtkLookupTable>::New();
        this->lut->SetTableRange(0, 1);
        this->lut->SetHueRange(1.0, 0);
        this->lut->SetSaturationRange(0.9, 1);
        this->lut->SetValueRange(0.8, 1);
        this->lut->Build();
    }

    Shape3D::~Shape3D() {
        PYG_LOG_V("Deleting 3D shape 0x{:x}", (uint64_t)this);
    }

    void Shape3D::set_scalar_color_range(const double vmin, const double vmax) {
        if (vmax < vmin)
            throw std::invalid_argument("Scalar color range maximum must be larger than or equal to minimum.");
        this->lut->SetTableRange(vmin, vmax);
        this->lut->Build();
        // apply updated colors to actors
        this->actors->InitTraversal();
        auto actor = this->actors->GetNextActor();
        while (actor != nullptr) {
            auto polydata = dynamic_cast<vtkPolyData*>(actor->GetMapper()->GetInput());
            auto pointdata = polydata->GetPointData();
            auto colors = pointdata->GetScalars("Colors");
            for (auto idx = 0; idx < polydata->GetNumberOfPoints(); idx++) {
                double p[3];
                double dcolor[3];
                polydata->GetPoint(idx, p);
                this->lut->GetColor(this->color_mapping_function(p), dcolor);
                colors->SetTuple3(idx, dcolor[0]*255, dcolor[1]*255, dcolor[2]*255);
            }
            actor = this->actors->GetNextActor();
        }
    }

    double * Shape3D::get_scalar_color_range() const {
        return this->lut->GetTableRange();
    }

    py::list Shape3D::py_get_scalar_color_range() const {
        auto values = this->get_scalar_color_range();
        std::vector<double> vec{values[0], values[1]};
        return py::cast(vec);
    }


    void Shape3D::set_item(const size_t idx, vtkSmartPointer<vtkPolyData> polydata) {
        // apply colors to data according to mapping function
        auto colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
        // I set a name to the color array together with SetActiveScalars to make sure
        // that we use this array for coloring; otherwise, other arrays (i.e. Normals)
        // may be used instead
        colors->SetName("Colors");
        colors->SetNumberOfComponents(3);
        colors->SetNumberOfTuples(polydata->GetNumberOfPoints());
        for (auto idx = 0; idx < polydata->GetNumberOfPoints(); idx++) {
            double p[3];
            double dcolor[3];
            polydata->GetPoint(idx, p);
            this->lut->GetColor(this->color_mapping_function(p), dcolor);
            colors->SetTuple3(idx, dcolor[0]*255, dcolor[1]*255, dcolor[2]*255);
        }
        polydata->GetPointData()->SetScalars(colors);
        polydata->GetPointData()->SetActiveScalars("Colors");
        // normals are not necessary for rendering but are required
        // for distance measurement; see distance_to_actor for details
        auto norm = vtkSmartPointer<vtkPolyDataNormals>::New();
        norm->SetInputData(polydata);
        norm->FlipNormalsOff();
        norm->AutoOrientNormalsOff();
        norm->ConsistencyOn();
        norm->ComputePointNormalsOn();
        norm->ComputeCellNormalsOn();
        norm->NonManifoldTraversalOn();
        norm->Update();
        // replace polydata with polydata with normals
        //polydata = norm->GetOutput();
        // remove old reference if it exists
        if (auto old_actor = dynamic_cast<vtkActor*>(this->actors->GetItemAsObject(idx)); old_actor != nullptr) {
            auto highlighted = this->get_highlighted(old_actor);
            //old_actor->GetMapper()->SetInputConnection(norm->GetOutputPort());
            old_actor->GetMapper()->SetInputDataObject(norm->GetOutput());
            this->set_highlighted(old_actor, highlighted);
            old_actor->Modified();
        } else {
            auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            //mapper->SetInputConnection(norm->GetOutputPort());
            mapper->SetInputDataObject(norm->GetOutput());
            mapper->SetScalarModeToUsePointData();
            mapper->SetScalarVisibility(0);
            auto actor = vtkSmartPointer<vtkActor>::New();
            actor->SetMapper(mapper);
            auto info = actor->GetProperty()->GetInformation();
            info->Set(this->highlight_key, 0);
            this->actors->AddItem(actor);
            this->set_highlighted(actor, false);
        }
        this->set_color(idx, this->base_color);
    }


    vtkSmartPointer<vtkActorCollection> Shape3D::get_actors() {
        return this->actors;
    }

    void Shape3D::set_label(const std::string & label) {
        this->label = label;
    }

    std::string Shape3D::get_label() const {
        return this->label;
    }


    void Shape3D::set_base_color(const std::vector<uint8_t> & color) {
        if (color.size()!=3 && color.size()!=4)
            throw std::invalid_argument("Color must be a 3 or 4-component vector.");
        this->base_color = color;
        for (size_t idx = 0; idx < this->actors->GetNumberOfItems(); idx++) {
            auto actor = dynamic_cast<vtkActor*>(this->actors->GetItemAsObject(idx));
            if (!this->get_highlighted(actor))
                this->set_color(idx, color);
        }
    }

    void Shape3D::set_color(const size_t idx, const std::vector<uint8_t> & color) {
        if (color.size()!=3 && color.size()!=4)
            throw std::invalid_argument("Color must be a 3 or 4-component vector.");
        if (idx >= this->actors->GetNumberOfItems()) return;
        auto actor = dynamic_cast<vtkActor*>(this->actors->GetItemAsObject(idx));
        actor->GetProperty()->SetColor(
            double(color[0])/255.0,
            double(color[1])/255.0,
            double(color[2])/255.0
        );
        if (color.size()>3)
            actor->GetProperty()->SetOpacity(double(color[3])/255.0);
        else
            actor->GetProperty()->SetOpacity(1);
        
        actor->Modified();
    }

    std::vector<uint8_t> Shape3D::get_base_color() const {
        return this->base_color;
    }

    void Shape3D::set_highlight_color(const std::vector<uint8_t> & color) {
        if (color.size()!=3 && color.size()!=4)
            throw std::invalid_argument("Color must be a 3 or 4-component vector.");
        this->highlight_color = color;
        for (size_t idx = 0; idx < this->actors->GetNumberOfItems(); idx++) {
            auto actor = dynamic_cast<vtkActor*>(this->actors->GetItemAsObject(idx));
            if (this->get_highlighted(actor))
                this->set_color(idx, color);
        }
    }

    std::vector<uint8_t> Shape3D::get_highlight_color() const {
        return this->highlight_color;
    }



    void Shape3D::set_scalar_color_mode(const bool en) {
        this->actors->InitTraversal();
        auto actor = this->actors->GetNextActor();
        while (actor != nullptr) {
            actor->GetMapper()->SetScalarVisibility(en);
            actor = this->actors->GetNextActor();
        }
    }

    void Shape3D::toggle_scalar_color_mode() {
        this->actors->InitTraversal();
        auto actor = this->actors->GetNextActor();
        while (actor != nullptr) {
            actor->GetMapper()->SetScalarVisibility(
                !actor->GetMapper()->GetScalarVisibility()
            );
            actor = this->actors->GetNextActor();
        }
    }


    bool Shape3D::get_scalar_color_mode() const {
        if (this->actors->GetNumberOfItems() == 0) return false;
        this->actors->InitTraversal();
        auto actor = this->actors->GetNextActor();
        return actor->GetMapper()->GetScalarVisibility();
    }


    void Shape3D::set_visible(const bool en) {
        this->actors->InitTraversal();
        auto actor = this->actors->GetNextActor();
        while (actor != nullptr) {
            actor->SetVisibility(en);
            actor = this->actors->GetNextActor();
        }
    }

    void Shape3D::toggle_visibility() {
        this->actors->InitTraversal();
        auto actor = this->actors->GetNextActor();
        while (actor != nullptr) {
            actor->SetVisibility(!actor->GetVisibility());
            actor = this->actors->GetNextActor();
        }
    }


    bool Shape3D::get_visibility() const {
        if (this->actors->GetNumberOfItems() == 0) return false;
        this->actors->InitTraversal();
        auto actor = this->actors->GetNextActor();
        return actor->GetVisibility();
    }


    void Shape3D::set_highlighted(vtkSmartPointer<vtkActor> actor, const bool en) {
        if (this->actors->IsItemPresent(actor) == 0) return;
        if (this->get_highlighted(actor) == en) return;
        if (en) {
            actor->GetProperty()->SetColor(
                double(this->highlight_color[0])/255.0,
                double(this->highlight_color[1])/255.0,
                double(this->highlight_color[2])/255.0
            );
            if (this->highlight_color.size()>3)
                actor->GetProperty()->SetOpacity(double(this->highlight_color[3])/255.0);
            else
                actor->GetProperty()->SetOpacity(1);
        } else {
            actor->GetProperty()->SetColor(
                double(this->base_color[0])/255.0,
                double(this->base_color[1])/255.0,
                double(this->base_color[2])/255.0
            );
            if (this->base_color.size()>3)
                actor->GetProperty()->SetOpacity(double(this->base_color[3])/255.0);
            else
                actor->GetProperty()->SetOpacity(1);
        }
        // flip point colors
        auto polydata = dynamic_cast<vtkPolyData*>(actor->GetMapper()->GetInput());
        Shape3D::flip_colors(polydata);
        auto info = actor->GetProperty()->GetInformation();
        info->Set(this->highlight_key, en);
        actor->Modified();
    }


    void Shape3D::set_highlighted(const unsigned int idx, bool en) {
        if (idx >= this->actors->GetNumberOfItems())
            throw std::out_of_range("Index out of range.");
        auto actor = dynamic_cast<vtkActor*>(this->actors->GetItemAsObject(idx));
        this->set_highlighted(actor, en);
    }


    void Shape3D::set_highlighted(bool en) {
        this->actors->InitTraversal();
        auto actor = this->actors->GetNextActor();
        while (actor != nullptr) {
            this->set_highlighted(actor, en);
            actor = this->actors->GetNextActor();
        }
    }


    void Shape3D::toggle_highlighted(vtkSmartPointer<vtkActor> actor) {
        this->set_highlighted(actor, !this->get_highlighted(actor));
    }

    void Shape3D::toggle_highlighted(const unsigned int idx) {
        if (idx >= this->actors->GetNumberOfItems())
            throw std::out_of_range("Index out of range.");
        auto actor = dynamic_cast<vtkActor*>(this->actors->GetItemAsObject(idx));
        this->set_highlighted(actor, !this->get_highlighted(actor));
    }

    void Shape3D::toggle_highlighted() {
        this->actors->InitTraversal();
        auto actor = this->actors->GetNextActor();
        while (actor != nullptr) {
            this->toggle_highlighted(actor);
            actor = this->actors->GetNextActor();
        }
    }

    bool Shape3D::get_highlighted(vtkSmartPointer<vtkActor> actor) const {
        PYG_LOG_V("Getting highlight state for actor 0x{:x}", (uint64_t)actor.GetPointer());
        if (!this->actors->IsItemPresent(actor))
            throw std::invalid_argument("Actor must belong to requested shape.");
        auto info = actor->GetProperty()->GetInformation();
        auto state = static_cast<bool>(info->Get(this->highlight_key));
        return state;
    }

    bool Shape3D::get_highlighted(const unsigned int idx) const {
        if (idx >= this->actors->GetNumberOfItems())
            throw std::out_of_range("Index out of range.");
        auto actor = dynamic_cast<vtkActor*>(this->actors->GetItemAsObject(idx));
        return this->get_highlighted(actor);
    }

    std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>> Shape3D::get_interactive() {
        std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>> interactive;
        this->actors->InitTraversal();
        auto actor = this->actors->GetNextActor();
        if (this->actors->GetNumberOfItems() == 1) {
            interactive.emplace_back(std::make_tuple(actor, this->label));
        } else {
            int idx = 0;
            while (actor != nullptr) {
                interactive.emplace_back(std::make_tuple(actor, fmt::format("{} {}", this->label, idx++)));
                actor = this->actors->GetNextActor();
            }
        }
        return interactive;
    }


    bool Shape3D::is_point_inside(const double point[3]) const {
        if (this->actors->GetNumberOfItems()==0) return false;
        auto selpt = vtkSmartPointer<vtkSelectEnclosedPoints>::New();
        auto pt = vtkSmartPointer<vtkPoints>::New();
        pt->InsertNextPoint(point);
        auto polpt = vtkSmartPointer<vtkPolyData>::New();
        polpt->SetPoints(pt);
        selpt->SetInputData(polpt);

        this->actors->InitTraversal();
        for (;;) {
            auto actor = this->actors->GetNextActor();
            if (actor == nullptr) break;
            auto polydata = dynamic_cast<vtkPolyData*>(actor->GetMapper()->GetInput());
            selpt->SetSurfaceData(polydata);
            selpt->Update();
            if (selpt->IsInside(0) == 1) return true;
        }
        return false;
    }

    bool Shape3D::is_point_inside(const std::vector<double> & point) const {
        if (point.size()!=3)
            throw std::invalid_argument("A point must have 3 components.");
        return this->is_point_inside(&point[0]);
    }

    bool Shape3D::is_point_inside(std::shared_ptr<const Point> point) const {
        auto cart = point->to_cartesian();
        std::vector<double> values{cart->x, cart->y, cart->z};
        return this->is_point_inside(values);
    }


    double Shape3D::distance_to_actor(vtkSmartPointer<vtkActor> actor, const double point[3]) {
        double closest_pt[3];
        double dist2;
        vtkIdType cell_id;
        int sub_id;
        auto polydata = dynamic_cast<vtkPolyData*>(actor->GetMapper()->GetInput());
        auto dist_filter = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
        dist_filter->SetInput(polydata);
        return abs(dist_filter->EvaluateFunction(point[0], point[1], point[2]));
    }


    double Shape3D::distance_to_actor(vtkSmartPointer<vtkActor> actor, const std::vector<double> & point) {
        if (point.size()!=3)
            throw std::invalid_argument("A point must have 3 components.");
        return Shape3D::distance_to_actor(actor, &point[0]);
    }

    double Shape3D::distance_to_actor(vtkSmartPointer<vtkActor> actor, std::shared_ptr<const Point> point) {
        auto cart = point->to_cartesian();
        std::vector<double> values{cart->x, cart->y, cart->z};
        return Shape3D::distance_to_actor(actor, values);
    }


    std::tuple<double, vtkSmartPointer<vtkActor>> Shape3D::closest_actor(const double point[3]) const {
        double old_dist = std::numeric_limits<double>::max();
        double new_dist;
        vtkSmartPointer<vtkActor> closest_actor;

        this->actors->InitTraversal();
        for (;;) {
            auto actor = this->actors->GetNextActor();
            if (actor == nullptr) break;
            auto new_dist = this->distance_to_actor(actor, point);
            if (new_dist<old_dist) {
                old_dist = new_dist;
                closest_actor = actor;
            }
        }
        // result is distance from point and closest actor
        auto distance = old_dist;
        return std::tie(distance, closest_actor);
    }

    std::tuple<double, vtkSmartPointer<vtkActor>> Shape3D::closest_actor(const std::vector<double> & point) const {
        if (point.size()!=3)
            throw std::invalid_argument("A point must have 3 components.");
        return this->closest_actor(&point[0]);
    }

    std::tuple<double, vtkSmartPointer<vtkActor>> Shape3D::closest_actor(std::shared_ptr<const Point> point) const {
        auto cart = point->to_cartesian();
        std::vector<double> values{cart->x, cart->y, cart->z};
        return this->closest_actor(values);
    }

    vtkSmartPointer<vtkActor> Shape3D::intersecting_actor(const double point1[3], const double point2[3]) const {
        auto cells_ids = vtkSmartPointer<vtkIdList>::New();

        this->actors->InitTraversal();
        for (;;) {
            auto actor = this->actors->GetNextActor();
            if (actor == nullptr) break;
            auto polydata = actor->GetMapper()->GetInput();
            auto locator = vtkSmartPointer<vtkCellLocator>::New();
            locator->SetDataSet(polydata);
            locator->BuildLocator();
            locator->FindCellsAlongLine(point1, point2, 1e-6, cells_ids);
            // this will return the 1st actor that intersects; if more than one does,
            // this takes the 1st one in the list, not necessarily the closest one.
            if (cells_ids->GetNumberOfIds()>0) return actor;
        }
        return nullptr;
    }

    vtkSmartPointer<vtkActor> Shape3D::intersecting_actor(const std::vector<double> & point1, const std::vector<double> & point2) const {
        if (point1.size()!=3 || point2.size()!=3)
            throw std::invalid_argument("A point must have 3 components.");
        
        return this->intersecting_actor(&point1[0], &point2[0]);
    }

    vtkSmartPointer<vtkActor> Shape3D::intersecting_actor(std::shared_ptr<const Point> point1, std::shared_ptr<const Point> point2) const {
        auto cart1 = point1->to_cartesian();
        auto cart2 = point2->to_cartesian();
        return this->intersecting_actor(
            std::vector<double>{cart1->x, cart1->y, cart1->z},
            std::vector<double>{cart2->x, cart2->y, cart2->z});
    }


    std::vector<vtkSmartPointer<vtkActor>> Shape3D::py_get_actors() const {
        std::vector<vtkSmartPointer<vtkActor>> py_data;
        if (this->actors->GetNumberOfItems()==0) return py_data;
        py_data.reserve(this->actors->GetNumberOfItems());
        this->actors->InitTraversal();
        for (;;) {
            auto actor = this->actors->GetNextActor();
            if (actor == nullptr) break;
            py_data.emplace_back(actor);
        }
        return py_data;
    }


    void py_shape3d_exports(py::module_ & mod) {
        py::class_<Shape3D, std::shared_ptr<Shape3D>, PyShape3D>(mod, "Shape3D")
        .def(py::init<>())
        .def("set_item", &Shape3D::set_item, py::arg("index"), py::arg("polydata"))
        .def_property("base_color", &Shape3D::get_base_color, &Shape3D::set_base_color)
        .def_property("highlight_color", &Shape3D::get_highlight_color, &Shape3D::set_highlight_color)
        .def_property("label", &Shape3D::get_label, &Shape3D::set_label)
        .def_property("scalar_color_mode", &Shape3D::get_scalar_color_mode, &Shape3D::set_scalar_color_mode)
        .def_property("visible", &Shape3D::get_visibility, &Shape3D::set_visible)
        .def("set_scalar_color_range", &Shape3D::set_scalar_color_range, py::arg("vmin"), py::arg("vmax"))
        .def("get_scalar_color_range", &Shape3D::py_get_scalar_color_range)
        .def("set_highlighted", static_cast<void(Shape3D::*)(vtkSmartPointer<vtkActor>, const bool)>(&Shape3D::set_highlighted), py::arg("actor"), py::arg("enabled"))
        .def("set_highlighted", static_cast<void(Shape3D::*)(const unsigned int, const bool)>(&Shape3D::set_highlighted), py::arg("index"), py::arg("enabled"))
        .def("set_highlighted", static_cast<void(Shape3D::*)(const bool)>(&Shape3D::set_highlighted), py::arg("enabled"))
        .def("toggle_highlighted", static_cast<void(Shape3D::*)(vtkSmartPointer<vtkActor>)>(&Shape3D::toggle_highlighted), py::arg("actor"))
        .def("toggle_highlighted", static_cast<void(Shape3D::*)(const unsigned int)>(&Shape3D::toggle_highlighted), py::arg("index"))
        .def("toggle_highlighted", static_cast<void(Shape3D::*)()>(&Shape3D::toggle_highlighted))
        .def("get_highlighted", static_cast<bool(Shape3D::*)(vtkSmartPointer<vtkActor>) const>(&Shape3D::get_highlighted), py::arg("actor"))
        .def("get_highlighted", static_cast<bool(Shape3D::*)(const unsigned int) const>(&Shape3D::get_highlighted), py::arg("index"))
        .def("toggle_visibility", &Shape3D::toggle_visibility)
        .def_property_readonly("actors", &Shape3D::py_get_actors, py::return_value_policy::reference)
        .def("get_interactive", &Shape3D::get_interactive, py::return_value_policy::reference)
        .def("is_point_inside", static_cast<bool(Shape3D::*)(std::shared_ptr<const Point>) const>(&Shape3D::is_point_inside), py::arg("point"))
        .def_static("distance_to_actor", static_cast<double(*)(vtkSmartPointer<vtkActor>, std::shared_ptr<const Point>)>(&Shape3D::distance_to_actor), py::arg("actor"), py::arg("point"))
        .def("closest_actor", static_cast<std::tuple<double, vtkSmartPointer<vtkActor>>(Shape3D::*)(std::shared_ptr<const Point>) const>(&Shape3D::closest_actor), py::arg("point"), py::return_value_policy::reference)
        .def("intersecting_actor", static_cast<vtkSmartPointer<vtkActor>(Shape3D::*)(std::shared_ptr<const Point>, std::shared_ptr<const Point>) const>(&Shape3D::intersecting_actor), py::arg("point1"), py::arg("point2"), py::return_value_policy::reference)
        ;
    }

}