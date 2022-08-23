/** \file marker.cpp
 *  \brief Implementation file for Marker classes.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <vtkVectorText.h>
#include <vtkTriangleFilter.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyData.h>
#include <vtkMapper.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#include <pybind11/stl.h>

#include "marker.h"

namespace pygraver::render {

    Marker::Marker(const char glyph,
                   std::shared_ptr<const Point> position,
                   const double size_x,
                   const double size_y,
                   const double thickness,
                   std::shared_ptr<const Point> orientation,
                   const std::vector<uint8_t> & color) : Extrusion() {
        PYG_LOG_V("Creating marker 0x{:x}", (uint64_t)this);
        // create base shape
        auto vectext = vtkSmartPointer<vtkVectorText>::New();
        char text[2] = {glyph, '\0'};
        vectext->SetText(text);
        auto extr_filter = vtkSmartPointer<vtkLinearExtrusionFilter>::New();
        extr_filter->SetInputConnection(vectext->GetOutputPort());
        extr_filter->SetExtrusionTypeToNormalExtrusion();
        extr_filter->SetCapping(1);
        extr_filter->SetVector(0,0,1);
        extr_filter->SetScaleFactor(thickness);
        auto tri_filter = vtkSmartPointer<vtkTriangleFilter>::New();
        tri_filter->SetInputConnection(extr_filter->GetOutputPort());
        tri_filter->Update();
        auto polydata = tri_filter->GetOutput();
        // get marker size
        double bounds[6];
        double min_x = std::numeric_limits<double>::max();
        double max_x = -std::numeric_limits<double>::max();
        double min_y = std::numeric_limits<double>::max();
        double max_y = -std::numeric_limits<double>::max();
        for (auto cid = 0; cid<polydata->GetNumberOfCells(); cid++) {
            polydata->GetCellBounds(cid, bounds);
            min_x = std::min(min_x, bounds[0]);
            max_x = std::max(max_x, bounds[1]);
            min_y = std::min(min_y, bounds[2]);
            max_y = std::max(max_y, bounds[3]);
        }
        // make marker copies 
        auto cart = position->to_cartesian();
        auto transform = vtkSmartPointer<vtkTransform>::New();
        transform->Translate(cart->x, cart->y, cart->z);
        transform->RotateZ(orientation->angle());
        transform->RotateX(orientation->elevation()-90);
        transform->Scale(size_x/(max_x-min_x), size_y/(max_y-min_y), 1);
        transform->Translate(-(min_x + max_x)/2, -(min_y + max_y)/2, 0);
        auto trans_filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        trans_filter->SetTransform(transform);
        trans_filter->SetInputData(polydata);
        trans_filter->Update();
        // generate actor
        this->set_item(0, trans_filter->GetOutput());
        // set base colors
        this->set_base_color(color);
        this->set_highlight_color(Shape3D::make_highlight_color(this->base_color));
    }

    Marker::~Marker() {
        PYG_LOG_V("Deleting marker 0x{:x}", (uint64_t)this);
    }

    std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>> Marker::get_interactive() {
        return Shape3D::get_interactive();
    }

    MarkerCollection::MarkerCollection(const char glyph,
                                       const std::vector<std::shared_ptr<Point>> & positions,
                                       const double size_x,
                                       const double size_y,
                                       const double thickness,
                                       std::shared_ptr<const Point> orientation,
                                       const std::vector<uint8_t> & color) : Extrusion() {
        PYG_LOG_V("Creating marker collection 0x{:x}", (uint64_t)this);
        auto base = Marker(glyph, std::make_shared<Point>(), size_x, size_y, thickness, orientation, color);
        base.get_actors()->InitTraversal();
        auto base_data = dynamic_cast<vtkPolyData*>(base.get_actors()->GetNextActor()->GetMapper()->GetInput());
        size_t idx = 0;
        for (auto pt: positions) {
            auto cart = pt->to_cartesian();
            auto transform = vtkSmartPointer<vtkTransform>::New();
            transform->Translate(cart->x, cart->y, cart->z);
            auto trans_filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
            trans_filter->SetTransform(transform);
            trans_filter->SetInputData(base_data);
            trans_filter->Update();
            this->set_item(idx++, trans_filter->GetOutput());
        }
        // set base colors
        this->set_base_color(color);
        this->set_highlight_color(Shape3D::make_highlight_color(this->base_color));
    }

    MarkerCollection::~MarkerCollection() {
        PYG_LOG_V("Deleting marker collection 0x{:x}", (uint64_t)this);
    }

    std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>> MarkerCollection::get_interactive() {
        return Shape3D::get_interactive();
    }

    void py_marker_exports(py::module_ & mod) {

        auto marker_cls = py::class_<Marker, std::shared_ptr<Marker>, Shape3D>(mod, "Marker")
            .def(py::init<const char, std::shared_ptr<const Point>, const double, const double, const double, std::shared_ptr<const Point>, const std::vector<uint8_t> &>(),
                 py::arg("glyph"), py::arg("position"), py::arg("size_x"), py::arg("size_y"), py::arg("thickness"), py::arg("orientation"), py::arg("color"));

        auto markercol_cls = py::class_<MarkerCollection, std::shared_ptr<MarkerCollection>, Shape3D>(mod, "MarkerCollection")
            .def(py::init<const char, const std::vector<std::shared_ptr<Point>> &, const double, const double, const double, std::shared_ptr<const Point>, const std::vector<uint8_t> &>(),
                 py::arg("glyph"), py::arg("positions"), py::arg("size_x"), py::arg("size_y"), py::arg("thickness"), py::arg("orientation"), py::arg("color"));

    }

}