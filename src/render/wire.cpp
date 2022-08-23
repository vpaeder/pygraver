/** \file wire.cpp
 *  \brief Implementation file for Wire classes.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkTubeFilter.h>
#include <pybind11/stl.h>

#include "../types/point.h"

#include "wire.h"
#include "../log.h"

namespace pygraver::render {

    /** \brief Make wire out of Path object.
     *  \param path: pointer to Path object.
     *  \param diameter: wire diameter.
     *  \param sides: number of sides (>=4).
     *  \returns pointer to vtkPolyData object.
     */
    static vtkSmartPointer<vtkPolyData> make_wire(std::shared_ptr<Path> path,
                                                  const double diameter,
                                                  const uint16_t sides) {
        PYG_LOG_V("Creating wire out of path 0x{:x}", (uint64_t)path.get());
        auto n = path->size();
        auto points = vtkSmartPointer<vtkPoints>::New();
        auto polyline = vtkSmartPointer<vtkPolyLine>::New();
        points->SetNumberOfPoints(n + path->is_closed());
        polyline->GetPointIds()->SetNumberOfIds(n + path->is_closed());
        for (auto i=0; i<n; i++) {
            points->SetPoint(i, (*path)[i]->x, (*path)[i]->y, (*path)[i]->z);
            polyline->GetPointIds()->SetId(i,i);
        }
        if (path->is_closed()) {
            points->SetPoint(n, (*path)[0]->x, (*path)[0]->y, (*path)[0]->z);
            polyline->GetPointIds()->SetId(n,n);
        }
        auto cells = vtkSmartPointer<vtkCellArray>::New();
        cells->InsertNextCell(polyline);
        auto polydata = vtkSmartPointer<vtkPolyData>::New();
        polydata->SetPoints(points);
        polydata->SetLines(cells);

        auto tube_filter = vtkSmartPointer<vtkTubeFilter>::New();
        tube_filter->SetInputData(polydata);
        tube_filter->SetRadius(diameter/2.0);
        tube_filter->SetNumberOfSides(sides);
        tube_filter->SetSidesShareVertices(1);
        tube_filter->SetCapping(1);
        tube_filter->Update();
        return tube_filter->GetOutput();
    }

    Wire::Wire(const std::shared_ptr<Path> path,
               const double diameter,
               const std::vector<uint8_t> & color,
               const unsigned int sides) : Shape3D() {
        PYG_LOG_V("Creating wire 0x{:x}", (uint64_t)this);
        this->set_path(path, diameter, color, sides);
    }

    Wire::~Wire() {
        PYG_LOG_V("Deleting wire 0x{:x}", (uint64_t)this);
    }

    void Wire::set_path(const std::shared_ptr<Path> path,
                        const double diameter,
                        const std::vector<uint8_t> & color,
                        const unsigned int sides) {
        PYG_LOG_V("Setting path for wire 0x{:x} from path 0x{:x}", (uint64_t)this, (uint64_t)path.get());
        if (path->size() == 0)
            std::invalid_argument("Cannot create wire from empty path.");
        // representation is in cartesian coordinates
        auto cartesian = path->to_cartesian();
        // define color range
        double vmin = std::numeric_limits<double>::max();
        double vmax = -std::numeric_limits<double>::max();
        for (auto point: *cartesian) {
            double pos[3] = {point->x, point->y, point->z};
            auto vcur = this->color_mapping_function(pos);
            vmin = std::min(vmin, vcur);
            vmax = std::max(vmax, vcur);
        }
        this->set_scalar_color_range(vmin, vmax);
        
        // make actor
        this->set_item(0, make_wire(cartesian, diameter, sides));
        this->set_base_color(color);
        this->set_highlight_color(Shape3D::make_highlight_color(color));
    }

    double Wire::color_mapping_function(const double pos[3]) {
        return pos[2];
    }

    double CylindricalWire::color_mapping_function(const double pos[3]) {
        return sqrt(pow(pos[1],2) + pow(pos[2],2));
    }

    CylindricalWire::CylindricalWire(const double cylinder_radius,
                                     const std::shared_ptr<Path> path,
                                     const double diameter,
                                     const std::vector<uint8_t> & color,
                                     const unsigned int sides) {
        PYG_LOG_V("Creating cylindrical wire 0x{:x}", (uint64_t)this);
        this->cylinder_radius = cylinder_radius;
        this->set_path(path, diameter, color, sides);
    }


    void CylindricalWire::set_path(const std::shared_ptr<Path> path,
                                   const double diameter,
                                   const std::vector<uint8_t> & color,
                                   const unsigned int sides) {
        auto cyl_path = path->to_cylindrical(this->cylinder_radius);
        Wire::set_path(cyl_path, diameter, color, sides);
    }


    WireCollection::WireCollection(const std::vector<std::shared_ptr<Path>> & paths,
                                   const double diameter,
                                   const std::vector<uint8_t> & color,
                                   const unsigned int sides) {
        PYG_LOG_V("Creating wire collection 0x{:x}", (uint64_t)this);
        this->set_paths(paths, diameter, color, sides);
    }

    WireCollection::~WireCollection() {
        PYG_LOG_V("Deleting wire collection 0x{:x}", (uint64_t)this);
    }

    void WireCollection::set_paths(const std::vector<std::shared_ptr<Path>> & paths,
                                   const double diameter,
                                   const std::vector<uint8_t> & color,
                                   const unsigned int sides) {
        PYG_LOG_V("Setting {:d} paths for wire collection 0x{:x}", paths.size(), (uint64_t)this);
        if (paths.size() == 0)
            std::invalid_argument("Cannot create wire collection from empty list.");
        // define color range
        double vmin = std::numeric_limits<double>::max();
        double vmax = -std::numeric_limits<double>::max();
        for (auto const path: paths) {
            auto cartesian = path->to_cartesian();
            for (auto point: *cartesian) {
                double pos[3] = {point->x, point->y, point->z};
                auto vcur = this->color_mapping_function(pos);
                vmin = std::min(vmin, vcur);
                vmax = std::max(vmax, vcur);
            }
        }
        this->set_scalar_color_range(vmin, vmax);

        // make actors
        this->actors->RemoveAllItems();
        this->diameter = diameter;
        size_t idx = 0;
        for (auto const path: paths) {
            if (path->size()==0) continue;
            this->set_path(idx++, path->to_cartesian(), sides);
        }
        
        this->set_base_color(color);
        this->set_highlight_color(Shape3D::make_highlight_color(color));
    }

    void WireCollection::set_path(const size_t idx,
                                  const std::shared_ptr<Path> path,
                                  const unsigned int sides) {
        PYG_LOG_V("Adding path 0x{:x} to wire collection 0x{:x} as index {:d}", (uint64_t)path.get(), (uint64_t)this, idx);
        this->set_item(idx, make_wire(path, this->diameter, sides));
        
    }

    double WireCollection::color_mapping_function(const double pos[3]) {
        return pos[2];
    }

    double CylindricalWireCollection::color_mapping_function(const double pos[3]) {
        return sqrt(pow(pos[1],2) + pow(pos[2],2));
    }

    CylindricalWireCollection::CylindricalWireCollection(const double cylinder_radius,
                                                         const std::vector<std::shared_ptr<Path>> & paths,
                                                         const double diameter,
                                                         const std::vector<uint8_t> & color,
                                                         const unsigned int sides) {
        PYG_LOG_V("Creating cylindrical wire collection 0x{:x}", (uint64_t)this);
        // build cylindrical paths
        std::vector<std::shared_ptr<Path>> cyl_paths;
        for (auto path: paths)
            cyl_paths.emplace_back(path->to_cylindrical(cylinder_radius));

        this->set_paths(cyl_paths, diameter, color, sides);
    }


    void py_wire_exports(py::module_ & mod) {
        py::class_<Wire, std::shared_ptr<Wire>, Shape3D>(mod, "Wire")
            .def(py::init<const std::shared_ptr<Path>, const double, const std::vector<uint8_t> &, const unsigned int>(),
                 py::arg("path"), py::arg("diameter"), py::arg("color"), py::arg("sides")=4)
            .def("set_path", &Wire::set_path, py::arg("path"), py::arg("diameter"), py::arg("color"), py::arg("sides")=4);

        py::class_<CylindricalWire, std::shared_ptr<CylindricalWire>, Wire>(mod, "CylindricalWire")
            .def(py::init<const double, const std::shared_ptr<Path>, const double, const std::vector<uint8_t> &, const unsigned int>(),
                 py::arg("cylinder_radius"), py::arg("path"), py::arg("diameter"), py::arg("color"), py::arg("sides")=4)
            .def("set_path", &CylindricalWire::set_path, py::arg("path"), py::arg("diameter"), py::arg("color"), py::arg("sides")=4);

        py::class_<WireCollection, std::shared_ptr<WireCollection>, Shape3D>(mod, "WireCollection")
            .def(py::init<const std::vector<std::shared_ptr<Path>> &, const double, const std::vector<uint8_t> &, const unsigned int>(),
                 py::arg("paths"), py::arg("diameter"), py::arg("color"), py::arg("sides")=4)
            .def("set_paths", &WireCollection::set_paths, py::arg("paths"), py::arg("diameter"), py::arg("color"), py::arg("sides")=4)
            .def("set_path", &WireCollection::set_path, py::arg("index"), py::arg("path"), py::arg("sides")=4)
            ;

        py::class_<CylindricalWireCollection, std::shared_ptr<CylindricalWireCollection>, WireCollection>(mod, "CylindricalWireCollection")
            .def(py::init<const double, const std::vector<std::shared_ptr<Path>> &, const double, const std::vector<uint8_t> &, const unsigned int>(),
                 py::arg("cylinder_radius"), py::arg("paths"), py::arg("diameter"), py::arg("color"), py::arg("sides")=4);

    }
}