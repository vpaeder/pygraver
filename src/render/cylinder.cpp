/** \file cylinder.cpp
 *  \brief Implementation file for Cylinder class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkCylinderSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#include <pybind11/stl.h>

#include "cylinder.h"
#include "wire.h"

namespace pygraver::render {

    double Cylinder::color_mapping_function(const double pos[3]) {
        return sqrt(pow(pos[1],2) + pow(pos[2],2));
    }

    Cylinder::~Cylinder() {
        PYG_LOG_V("Deleting cylinder 0x{:x}", (uint64_t)this);
    }

    Cylinder::Cylinder(const double radius,
                       const double height,
                       std::shared_ptr<Point> center,
                       std::shared_ptr<Point> axis,
                       const std::vector<uint8_t> & color) {
        PYG_LOG_V("Creating cylinder 0x{:x}", (uint64_t)this);
        if (radius <= 0)
            throw std::invalid_argument("Cylinder radius must be positive.");
        if (height <= 0)
            throw std::invalid_argument("Cylinder height must be positive.");
        // vtkCylinderSource produces a cylinder aligned with the z axis and centered at the origin.
        // One must use a vtkTransform to align it with the given axis. The angle that the axis makes
        // with primary axes must be computed first.
        // create base cylinder
        vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
        cylinder->SetCenter(0,0,0);
        cylinder->SetRadius(radius);
        cylinder->SetHeight(height);
        cylinder->SetResolution(100);
        // add transform to rotate cylinder
        vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
        transform->Translate(center->x, center->y, center->z);
        transform->RotateY(axis->angle());
        transform->RotateZ(axis->elevation());
        auto trans_filter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        trans_filter->SetTransform(transform);
        trans_filter->SetInputConnection(cylinder->GetOutputPort());
        trans_filter->Update();
        auto cyl = trans_filter->GetOutput();
        // generate actor
        this->set_item(0, cyl);
        // set base colors
        this->set_base_color(color);
        this->set_highlight_color(Shape3D::make_highlight_color(color));
    }


    void py_cylinder_exports(py::module_ & mod) {
        py::class_<Cylinder, std::shared_ptr<Cylinder>, Shape3D>(mod, "Cylinder")
        .def(py::init<const double, const double, std::shared_ptr<Point>, std::shared_ptr<Point>, const std::vector<uint8_t>&>(),
                py::arg("radius"), py::arg("height"), py::arg("center"), py::arg("axis"), py::arg("color"));
    }
}
