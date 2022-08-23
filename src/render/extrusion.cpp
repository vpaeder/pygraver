/** \file extrusion.cpp
 *  \brief Implementation file for Extrusion class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <vtkAppendPolyData.h>
#include <vtkLinearExtrusionFilter.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkDelaunay2D.h>
#include <vtkTriangle.h>

#include <pybind11/stl.h>

#include <geos/geom/Coordinate.h>

#include "../types/common.h"
#include "extrusion.h"

namespace pygraver::render {

    /** \brief Shorthand for geos::geom::Coordinate class. */
    using GEOSCoordinate = geos::geom::Coordinate;

    vtkSmartPointer<vtkPolyData> make_polydata(std::shared_ptr<Surface> surf) {
        auto nbnd = surf->get_contours().size();
        auto nh = surf->get_holes().size();
        PYG_LOG_I("Creating PolyData from {} boundar{} and {} hole{}.", nbnd, nbnd>1 ? "ies" : "y", nh, nh ? "s" : "");
        if (nbnd == 0)
            return vtkSmartPointer<vtkPolyData>::New();
        
        // compute total number of points; this is needed to allocate memory
        // for points (faster to do it in one go)
        size_t npts = 0;
        for (auto p: surf->get_contours())
            npts += p->size() - p->is_closed();
        for (auto p: surf->get_holes())
            npts += p->size() - p->is_closed();
        // create containers for cells
        auto cells_bnd = vtkSmartPointer<vtkCellArray>::New();
        auto cells_h = vtkSmartPointer<vtkCellArray>::New();
        // create container for points
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetNumberOfPoints(npts);
        size_t offset = 0; // point index offset
        // fill points for boundaries
        for (auto bnd: surf->get_contours()) {
            auto p = bnd->is_ccw() ? bnd : bnd->flip();
            auto poly_bnd = vtkSmartPointer<vtkPolygon>::New();
            auto n = p->size() - p->is_closed();
            PYG_LOG_D("Adding {} boundary points to PolyData (total={}).", n, offset+n);
            poly_bnd->GetPointIds()->SetNumberOfIds(n);
            for (auto i=0; i<n; i++) {
                PYG_LOG_D("Adding point {}: {},{},{}", offset+i, (*p)[i]->x, (*p)[i]->y, (*p)[i]->z);
                points->SetPoint(offset+i, (*p)[i]->x, (*p)[i]->y, (*p)[i]->z);
                poly_bnd->GetPointIds()->SetId(i, offset+i);
            }
            offset += n;
            // each boundary makes one polygon that makes one PolyData cell
            cells_bnd->InsertNextCell(poly_bnd);
        }
        // fill points for holes
        for (auto h: surf->get_holes()) {
            auto p = !h->is_ccw() ? h : h->flip();
            auto poly_h = vtkSmartPointer<vtkPolygon>::New();
            auto n = p->size() - p->is_closed();
            PYG_LOG_D("Adding {} hole points to PolyData (total={}).", n, offset+n);
            poly_h->GetPointIds()->SetNumberOfIds(n);
            for (auto i=0; i<n; i++) {
                PYG_LOG_D("Adding point {}: {},{},{}", offset+i, (*p)[i]->x, (*p)[i]->y, (*p)[i]->z);
                points->SetPoint(offset+i, (*p)[i]->x, (*p)[i]->y, (*p)[i]->z);
                poly_h->GetPointIds()->SetId(i, offset+i);
            }
            offset += n;
            // each hole makes one polygon that makes one PolyData cell
            cells_h->InsertNextCell(poly_h);
        }
        // create PolyData sets for boundaries and holes
        auto poly_bnd = vtkSmartPointer<vtkPolyData>::New();
        auto poly_h = vtkSmartPointer<vtkPolyData>::New();
        poly_bnd->SetPoints(points);
        poly_h->SetPoints(points);
        poly_bnd->SetPolys(cells_bnd);
        poly_h->SetPolys(cells_h);
        // perform constrained Delaunay triangulation
        auto delaunay = vtkSmartPointer<vtkDelaunay2D>::New();
        delaunay->SetInputData(poly_bnd);
        delaunay->SetSourceData(poly_h);
        delaunay->Update();
        auto deldata = delaunay->GetOutput();
        // Delaunay triangulation produces unwanted triangles if outer shape is concave;
        // therefore, we need to exclude those triangles; we pick the centre point of
        // each of them and probe if they are in the geometry with GEOS.
        auto dcells = deldata->GetPolys();
        auto dpoints = deldata->GetPoints();
        // container for triangles that are kept
        auto newpoly = vtkSmartPointer<vtkPolyData>::New();
        auto newcells = vtkSmartPointer<vtkCellArray>::New();
        const vtkIdType *pts;
        vtkIdType tripts = 3;
        auto geos_surf = surf->as_geos_geometry();
        for (auto i=0; i<deldata->GetNumberOfPolys(); i++) {
            auto dpol = dcells->GetNextCell(tripts, pts);
            if (dpol != 1) continue;
            double x=0, y=0, z=0;
            for (auto j=0; j<3; j++) {
                auto pt = dpoints->GetPoint(pts[j]);
                x += pt[0];
                y += pt[1];
                z += pt[2];
            }
            // take centroid point of triangle
            const GEOSCoordinate c(x/3, y/3, z/3);
            auto tript = std::unique_ptr<GEOSPoint>(gfactory->createPoint(c));
            // test if point is within surface
            if (geos_surf->contains(tript.get())) {
                auto triangle = vtkSmartPointer<vtkTriangle>::New();
                triangle->GetPointIds()->SetId(0, pts[0]);
                triangle->GetPointIds()->SetId(1, pts[1]);
                triangle->GetPointIds()->SetId(2, pts[2]);
                newcells->InsertNextCell(triangle);
            }
        }
        newpoly->SetPoints(dpoints);
        newpoly->SetPolys(newcells);
        return newpoly;
    }


    vtkSmartPointer<vtkPolyData> make_polydata(std::shared_ptr<Path> path) {
        auto points = vtkSmartPointer<vtkPoints>::New();
        auto polygon = vtkSmartPointer<vtkPolygon>::New();
        auto n = path->size();
        PYG_LOG_I("Creating PolyData with {} points.", n);

        points->SetNumberOfPoints(n);
        polygon->GetPointIds()->SetNumberOfIds(n);
        for (auto i=0; i<n; i++) {
            points->SetPoint(i, (*path)[i]->x, (*path)[i]->y, (*path)[i]->z);
            polygon->GetPointIds()->SetId(i, i);
        }        
        auto cells = vtkSmartPointer<vtkCellArray>::New();
        cells->InsertNextCell(polygon);
        auto polydata = vtkSmartPointer<vtkPolyData>::New();
        polydata->SetPoints(points);
        polydata->SetPolys(cells);
        return polydata;
    }


    vtkSmartPointer<vtkPolyData> extrude(vtkSmartPointer<vtkPolyData> data,
                                         std::shared_ptr<Point> axis,
                                         std::shared_ptr<Point> centre,
                                         const double length) {
        PYG_LOG_I("Extruding vtkPolyData 0x{:x}", (uint64_t)data.GetPointer());
        auto extr_filter = vtkSmartPointer<vtkLinearExtrusionFilter>::New();
        extr_filter->SetInputData(data);
        extr_filter->SetExtrusionTypeToNormalExtrusion();
        extr_filter->SetCapping(1);
        extr_filter->SetVector(axis->x, axis->y, axis->z);
        extr_filter->SetExtrusionPoint(centre->x, centre->y, centre->z);
        extr_filter->SetScaleFactor(length);
        extr_filter->Update();
        return extr_filter->GetOutput();
    }

    Extrusion::Extrusion() {
        PYG_LOG_V("Creating extrusion 0x{:x}", (uint64_t)this);
    }

    Extrusion::~Extrusion() {
        PYG_LOG_V("Deleting extrusion 0x{:x}", (uint64_t)this);
    }

    double Extrusion::color_mapping_function(const double pos[3]) {
        return axis[0]*pos[0] + axis[1]*pos[1] + axis[2]*pos[2];
    }

    Extrusion::Extrusion(std::shared_ptr<Surface> contour,
                         const double length,
                         std::shared_ptr<Point> axis,
                         const std::vector<uint8_t> & color) : Shape3D() {
        PYG_LOG_V("Creating extrusion 0x{:x}", (uint64_t)this);
        this->set_shape(contour, length, axis, color);
    }

    Extrusion::Extrusion(std::shared_ptr<Path> contour,
                         const double length,
                         std::shared_ptr<Point> axis,
                         const std::vector<uint8_t> & color) : Shape3D() {
        PYG_LOG_V("Creating extrusion 0x{:x}", (uint64_t)this);
        this->set_shape(contour, length, axis, color);
    }


    void Extrusion::set_shape(std::shared_ptr<Surface> contour,
                              const double length,
                              std::shared_ptr<Point> axis,
                              const std::vector<uint8_t> & color) {
        PYG_LOG_V("Setting contour for extrusion 0x{:x} from surface 0x{:x}", (uint64_t)this, (uint64_t)contour.get());
        auto surfaces = contour->combine();
        if (surfaces.size()==0)
            throw std::invalid_argument("Surface must contain at least one contour.");
        // normalize axis
        double raxis = axis->radius();
        if (raxis==0)
            throw std::invalid_argument("Axis must have non-zero length.");
        this->axis[0] = axis->x/raxis;
        this->axis[1] = axis->y/raxis;
        this->axis[2] = axis->z/raxis;
        // limits for color LUT
        double zmin = std::numeric_limits<double>::max();
        double zmax = -std::numeric_limits<double>::max();
        // process each sub-surface separately
        auto append = vtkSmartPointer<vtkAppendPolyData>::New();
        for (auto s: surfaces) {
            if (s->get_contours().size() == 0)
                continue;

            auto centroid = s->get_centroid();
            if (centroid->z<zmin) zmin = centroid->z;
            if (centroid->z>zmax) zmax = centroid->z;
            auto shape = extrude(make_polydata(s), axis, centroid, length);
            append->SetInputData(shape);
        }
        append->Update();
        // create actor for shape
        this->set_item(0, append->GetOutput());
        // set base colors
        this->set_base_color(color);
        this->set_highlight_color(Shape3D::make_highlight_color(color));
    }

    void Extrusion::set_shape(std::shared_ptr<Path> contour,
                              const double length,
                              std::shared_ptr<Point> axis,
                              const std::vector<uint8_t> & color) {
        PYG_LOG_V("Setting contour for extrusion 0x{:x} from path 0x{:x}", (uint64_t)this, (uint64_t)contour.get());
        if (contour->size()<4 || !contour->is_closed())
            throw std::invalid_argument("Contour must be closed.");
        // normalize axis
        double raxis = axis->radius();
        if (raxis==0)
            throw std::invalid_argument("Axis must have non-zero length.");
        this->axis[0] = axis->x/raxis;
        this->axis[1] = axis->y/raxis;
        this->axis[2] = axis->z/raxis;
        // extrude base surface
        auto xy = contour->to_cartesian();
        auto centroid = contour->get_centroid();
        // set base colors
        this->set_base_color(color);
        this->set_highlight_color(Shape3D::make_highlight_color(color));
        // create actor for shape
        this->set_item(0, extrude(make_polydata(xy), axis, centroid, length));
    }

    std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>> Extrusion::get_interactive() {
        return {};
    }


    void py_extrusion_exports(py::module_ & mod) {
        void (Extrusion::*set_shape_surface)(std::shared_ptr<Surface>, const double, std::shared_ptr<Point>, const std::vector<uint8_t> &) = &Extrusion::set_shape;
        void (Extrusion::*set_shape_path)(std::shared_ptr<Path>, const double, std::shared_ptr<Point>, const std::vector<uint8_t> &) = &Extrusion::set_shape;
        auto cls = py::class_<Extrusion, std::shared_ptr<Extrusion>, Shape3D>(mod, "Extrusion")
            .def(py::init<std::shared_ptr<Surface>, const double, std::shared_ptr<Point>, const std::vector<uint8_t> &>(), py::arg("contour"), py::arg("length"), py::arg("axis"), py::arg("color"))
            .def(py::init<std::shared_ptr<Path>, const double, std::shared_ptr<Point>, const std::vector<uint8_t> &>(), py::arg("contour"), py::arg("length"), py::arg("axis"), py::arg("color"))
            .def("set_shape", set_shape_surface, py::arg("contour"), py::arg("length"), py::arg("axis"), py::arg("color"))
            .def("set_shape", set_shape_path, py::arg("contour"), py::arg("length"), py::arg("axis"), py::arg("color"))
            ;
    }

}
