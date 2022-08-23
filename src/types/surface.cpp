/** \file surface.cpp
 *  \brief Implementation file for Surface class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <geos/geom/Coordinate.h>
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/CoordinateSequence.h"
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/linearref/LengthIndexedLine.h>

#include <pybind11/stl.h>

#include "common.h"
#include "surface.h"
#include "path.h"
#include "pathgroup.h"
#include "../log.h"

/** \brief Shorthand for geos::geom::Coordinate class. */
using GEOSCoordinate = geos::geom::Coordinate;
/** \brief Shorthand for geos::geom::LineString class. */
using GEOSLineString = geos::geom::LineString;
/** \brief Shorthand for geos::geom::LinearRing class. */
using GEOSLinearRing = geos::geom::LinearRing;
/** \brief Shorthand for geos::geom::Polygon class. */
using GEOSPolygon = geos::geom::Polygon;
/** \brief Shorthand for geos::geom::GeometryCollection class. */
using GEOSGeometryCollection = gg::GeometryCollection;
/** \brief Shorthand for geos::geom::prep::PreparedGeometryFactory class. */
using GEOSPreparedGeometryFactory = gg::prep::PreparedGeometryFactory;
/** \brief Shorthand for geos::geom::prep::PreparedGeometry class. */
using GEOSPreparedGeometry = const gg::prep::PreparedGeometry;
/** \brief Shorthand for geos::geom::CoordinateArraySequence class. */
using GEOSCoordinateArraySequence = geos::geom::CoordinateArraySequence;
/** \brief Shorthand for geos::geom::CoordinateSequence class. */
using GEOSCoordinateSequence = geos::geom::CoordinateSequence;

namespace pygraver::types {

    /** \brief Convert Path objects to GEOS LinearRing objects.
     *  \param paths: vector of Path objects to convert.
     *  \returns a vector of converted GEOS LinearRing objects.
     */
    static std::vector<std::unique_ptr<GEOSLinearRing>> compile_rings(const std::vector<std::shared_ptr<Path>> & paths) {
        if (paths.size()==0) return {};
        PYG_LOG_V("Converting {:d} paths to GEOS LinearRing", paths.size());
        std::vector<std::unique_ptr<GEOSLinearRing>> geoms;
        geoms.reserve(paths.size());
        for (auto p: paths) {
            auto geom = p->as_closed_geos_geometry();
            if (geom->isValid())
                geoms.emplace_back(std::move(geom));
        }
        return geoms;
    }

    void Surface::initialize() {
        PYG_LOG_V("Creating surface 0x{:x}", (uint64_t)this);
        if (!gfactory.get()) {
            gg::PrecisionModel *pm = new gg::PrecisionModel();
            gfactory = gg::GeometryFactory::create(pm);
            delete pm;
        }
    }

    Surface::Surface() {
        this->initialize();
    }

    Surface::Surface(std::shared_ptr<const Path> contour) {
        this->initialize();
        this->set_contours({std::const_pointer_cast<Path>(contour)});
    }
    
    Surface::Surface(std::shared_ptr<const Path> contour, const std::vector<std::shared_ptr<Path>> & holes) {
        this->initialize();
        this->set_contours({std::const_pointer_cast<Path>(contour)});
        this->set_holes(holes);
    }

    Surface::Surface(const std::vector<std::shared_ptr<Path>> & contours) {
        this->initialize();
        this->set_contours(contours);
    }
        
    Surface::Surface(const std::vector<std::shared_ptr<Path>> & contours, const std::vector<std::shared_ptr<Path>> & holes) {
        this->initialize();
        this->set_contours(contours);
        this->set_holes(holes);
    }

    Surface::Surface(const std::shared_ptr<Surface> surface, const std::vector<std::shared_ptr<Path>> & holes) {
        this->initialize();
        auto surfs = surface->combine();
        std::vector<std::shared_ptr<Path>> contours;
        for (auto surf: surfs)
            for (auto bnd: surf->get_contours())
                contours.emplace_back(bnd);
        
        this->set_contours(contours);
        this->set_holes(holes);
    }

    Surface::Surface(const std::shared_ptr<Surface> surface, const std::shared_ptr<Surface> holes) {
        this->initialize();
        auto surfs = surface->combine();
        auto hole_surfs = holes->combine();
        std::vector<std::shared_ptr<Path>> contours;
        std::vector<std::shared_ptr<Path>> hole_paths;
        for (auto surf: surfs)
            for (auto bnd: surf->get_contours())
                contours.emplace_back(bnd);
        for (auto surf: hole_surfs)
            for (auto bnd: surf->get_contours())
                hole_paths.emplace_back(bnd);
        
        this->set_contours(contours);
        this->set_holes(hole_paths);
    }
    
    Surface::~Surface() {
        PYG_LOG_V("Deleting surface 0x{:x}", (uint64_t)this);
    }
    
    const std::vector<std::shared_ptr<Path>> & Surface::get_contours() const {
        return this->contours;
    }

    const std::vector<std::shared_ptr<Path>> & Surface::get_holes() const {
        return this->holes;
    }

    void Surface::set_contours(const std::vector<std::shared_ptr<Path>> & contours) {
        auto n = contours.size();
        PYG_LOG_D("Setting {} boundar{} for 0x{:x}", n, (n>1 ? "ies" : "y"), (uint64_t)this);
        this->contours.clear();
        this->contours.reserve(n);
        for (auto bnd: contours)
            this->contours.emplace_back(std::move(bnd));
    }

    void Surface::set_holes(const std::vector<std::shared_ptr<Path>> & holes) {
        auto n = holes.size();
        PYG_LOG_D("Setting {} hole{} for 0x{:x}", n, (n>1 ? "s" : ""), (uint64_t)this);
        this->holes.clear();
        this->holes.reserve(n);
        for (auto h: holes)
            this->holes.emplace_back(std::move(h));
    }
        
    bool Surface::contains(std::shared_ptr<const Point> p) const {
        PYG_LOG_V("Checking if surface 0x{:x} contains point 0x{:x}", (uint64_t)this, (uint64_t)p.get());
        bool is_inside = false;
        auto point = p->as_geos_geometry();
        auto geom_holes = compile_rings(this->holes);
        for (auto bnd: this->contours) {
            auto contour = bnd->as_closed_geos_geometry();
            auto poly = gfactory->createPolygon(std::move(contour), std::forward<std::vector<std::unique_ptr<GEOSLinearRing> > >(geom_holes));
            is_inside = poly->contains(point.get()) || is_inside;
        }
        return is_inside;
    }
    
    std::vector<std::shared_ptr<Path>> Surface::get_milling_paths(const double tool_size, const double increment) const {
        PYG_LOG_V("Computing milling paths for surface 0x{:x}", (uint64_t)this);
        if(increment<=0)
			throw std::out_of_range("Increment must be larger than 0.");
        
        std::vector<std::shared_ptr<Path>> paths;
        for (auto bnd: this->contours) {
            auto cartesian = bnd->to_cartesian();
            double reduction = tool_size/2.0;
            for (;;) {
                auto new_path = cartesian->buffer(-reduction);
                if (new_path->size()==0) break;
                paths.emplace_back(new_path);
                reduction += increment;
            }
            // this is to avoid adding a milling point if the tool is too big
            if (reduction > tool_size/2.0)
                paths.emplace_back(std::make_shared<Path>(bnd->get_centroid()));
        }
        std::reverse(paths.begin(), paths.end());
        return paths;
    }
    
    std::vector<std::shared_ptr<Surface>> Surface::get_milled_surface(const double tool_size, const double increment) const {
        PYG_LOG_V("Computing milled surface for surface 0x{:x}", (uint64_t)this);
        // we take milling paths and generate surface considering tool size
        auto paths = this->get_milling_paths(tool_size, increment);
        std::vector<std::shared_ptr<Path>> new_paths;
        new_paths.reserve(paths.size());
        for (auto p: paths)
            if (p->size()>=4)
                new_paths.emplace_back(p->buffer(tool_size/2.0));
        
        return Surface(new_paths).combine();
    }

    std::unique_ptr<GEOSGeometry> Surface::as_geos_geometry() const {
        PYG_LOG_V("Converting surface 0x{:x} to GEOS geometry", (uint64_t)this);
        // create GEOS collection containing surface contours
        // gather holes
        auto geom_holes = compile_rings(this->holes);
        // gather contours
        std::vector<std::unique_ptr<GEOSGeometry>> geos_contours;
        geos_contours.reserve(this->contours.size());
        for (auto bnd: this->contours) {
            std::unique_ptr<GEOSLinearRing> contour = bnd->as_closed_geos_geometry();
            if (contour->isValid()) {
                std::unique_ptr<GEOSPolygon> polygon;
                polygon = std::unique_ptr<GEOSPolygon>(gfactory->createPolygon(std::move(contour), std::forward<decltype(geom_holes)>(geom_holes)));
                geos_contours.emplace_back(polygon->buffer(0));
            }
        }
        auto collection = gfactory->createGeometryCollection(std::forward<decltype(geos_contours)>(geos_contours));
        auto merged = collection->Union();
        return merged;
    }

    std::vector<std::shared_ptr<Surface>> Surface::combine() const {
        PYG_LOG_V("Combining surface 0x{:x}", (uint64_t)this);
        auto merged = this->as_geos_geometry();
        std::vector<std::shared_ptr<Surface>> surfaces;
        auto n = merged->getNumGeometries();
        PYG_LOG_D("Number of geometries: {}", n);
        surfaces.reserve(n);
        for (auto i=0; i<n; i++) {
            auto env_i = merged->getGeometryN(i);
            PYG_LOG_D("Geometry {} type: {}", i, env_i->getGeometryTypeId());
            if (env_i->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
                auto poly_i = dynamic_cast<const GEOSPolygon*>(env_i);
                auto new_path = make_path(poly_i->getExteriorRing());
                auto nh = poly_i->getNumInteriorRing();
                PYG_LOG_D("Number of holes for geometry {}: {}", i, nh);
                if (nh>0) {
                    std::vector<std::shared_ptr<Path>> new_holes;
                    new_holes.reserve(nh);
                    for (auto ih=0; ih<nh; ih++) {
                        auto hole_i = poly_i->getInteriorRingN(ih);
                        auto new_hole = make_path(poly_i->getInteriorRingN(ih));
                        new_holes.emplace_back(new_hole);
                    }
                    surfaces.emplace_back(std::make_shared<Surface>(new_path, new_holes));
                } else {
                    surfaces.emplace_back(std::make_shared<Surface>(new_path));
                }
            }
        }
        return surfaces;
    }

    std::shared_ptr<Point> Surface::get_centroid() const {
        PYG_LOG_V("Computing centroid for surface 0x{:x}", (uint64_t)this);
        // this takes only contours into account, not holes
        // => GEOS centroid algorithm would give a different result if holes
        // are present.
        double x = 0, y = 0, z = 0;
        for (auto p: this->contours) {
            auto centroid = p->get_centroid();
            x += centroid->x;
            y += centroid->y;
            z += centroid->z;
        }
        auto nbnds = this->contours.size();
        return std::make_shared<Point>(x/nbnds, y/nbnds, z/nbnds);
    }

    std::vector<std::shared_ptr<Surface>> Surface::boolean_operation(std::shared_ptr<const Surface> other, const BooleanOperation operation_type) const {
        PYG_LOG_V("Computing boolean operation for surface 0x{:x}", (uint64_t)this);
        auto this_merged = this->as_geos_geometry();
        auto other_merged = other->as_geos_geometry();
        std::vector<std::shared_ptr<Surface>> new_surfaces;
        auto ng = this_merged->getNumGeometries();
        new_surfaces.reserve(ng);
        for (auto i=0; i<ng; i++) {
            auto env_i = this_merged->getGeometryN(i);
            std::unique_ptr<GEOSGeometry> diff_i;
            if (operation_type == BooleanOperation::Union) {
                diff_i = env_i->Union(other_merged.get());
            } else if (operation_type == BooleanOperation::Difference) {
                diff_i = env_i->difference(other_merged.get());
            } else if (operation_type == BooleanOperation::SymmetricDifference) {
                diff_i = env_i->symDifference(other_merged.get());
            } else if (operation_type == BooleanOperation::Intersection) {
                diff_i = env_i->intersection(other_merged.get());
            }
            auto ndiff = diff_i->getNumGeometries();
            for (auto j=0; j<ndiff; j++) {
                auto env_j = diff_i->getGeometryN(j);
                auto new_path = make_path(env_j->getBoundary().get());
                new_surfaces.emplace_back(std::make_shared<Surface>(new_path));
            }
        }
        return new_surfaces;
    }
    
    std::vector<std::shared_ptr<Surface>> operator+(std::shared_ptr<const Surface> s1, std::shared_ptr<const Surface> s2) {
        return s1->boolean_operation(s2, BooleanOperation::Union);
    }

    std::vector<std::shared_ptr<Surface>> operator-(std::shared_ptr<const Surface> s1, std::shared_ptr<const Surface> s2) {
        return s1->boolean_operation(s2, BooleanOperation::Difference);
    }

    std::vector<std::shared_ptr<Surface>> operator*(std::shared_ptr<const Surface> s1, std::shared_ptr<const Surface> s2) {
        return s1->boolean_operation(s2, BooleanOperation::Intersection);
    }

    std::vector<std::shared_ptr<Path>> Surface::correct_height(const std::vector<std::shared_ptr<Path>> & paths, const double clearance, const double safe_height, const bool outside, const bool fix_contours) const {
        PYG_LOG_V("Correcting {:d} paths using surface 0x{:x} as mask", paths.size(), (uint64_t)this);
        // build a prepared geometry
        auto merged = this->as_geos_geometry()->buffer(clearance);
        auto prepared = GEOSPreparedGeometryFactory::prepare(merged.get());
        auto contour = merged->getBoundary();
        auto prep_bnd = GEOSPreparedGeometryFactory::prepare(contour.get());
        // loop over paths
        std::vector<std::shared_ptr<Path>> new_paths;
        new_paths.reserve(paths.size());
    
        for (auto path: paths) {
            if (path->size() == 0) {
                new_paths.emplace_back(std::make_shared<Path>(0));
                continue;
            }
            std::shared_ptr<Path> new_path;
            if (fix_contours) {
                new_path = std::make_shared<Path>(0);
                std::shared_ptr<Point> point0, point1;
                // find if path intersects with surface contours, and if so, add points on contours
                // to avoid losing accuracy; otherwise, make a direct copy of path
                for (auto it_point=1; it_point<path->size(); it_point++) {
                    // add 1st segment point to new path
                    new_path->emplace_back((*path)[it_point-1]);
        
                    // find if there's an intersection point to be added
                    point0 = (*path)[it_point-1]->to_cartesian();
                    point1 = (*path)[it_point]->to_cartesian();
                    // segment of current path to be compared with surface contour
                    auto segment = std::make_unique<GEOSCoordinateArraySequence>(2, 3);
                    segment->setAt(GEOSCoordinate(point0->x, point0->y, point0->z), 0);
                    segment->setAt(GEOSCoordinate(point1->x, point1->y, point1->z), 1);
                    auto seg_ls = gfactory->createLineString(std::move(segment));
        
                    if (prep_bnd->intersects(seg_ls.get())) {
                        // extract intersections from GEOS and sort position along segment
                        auto isec = contour->intersection(seg_ls.get());
                        std::size_t npoints = isec->getNumGeometries();
                        new_path->reserve(new_path->size() + 3*npoints);
        
                        // use a LengthIndexedLine to get position along segment
                        auto lil_segment = std::make_unique<geos::linearref::LengthIndexedLine>(seg_ls.get());
                    
                        // create 3 points per intersection to make sure we have
                        // at least one point on each side of the contour
                        std::vector<double> pts_positions;
                        std::vector<GEOSCoordinate> pts_coords;
                        pts_positions.reserve(3*npoints);
                        pts_coords.reserve(3*npoints);
                        for (auto isec_i = 0; isec_i < npoints; isec_i++) {
                            auto isec_pt = isec->getGeometryN(isec_i);
                            double curr_pos = lil_segment->indexOf(*isec_pt->getCoordinate());
                            pts_positions.emplace_back(std::min(curr_pos, std::max(curr_pos-1e-3, 1e-3)));
                            pts_positions.emplace_back(curr_pos);
                            pts_positions.emplace_back(std::max(curr_pos, std::min(curr_pos+1e-3,seg_ls->getLength()-1e-3)));
                            pts_coords.emplace_back(lil_segment->extractPoint(pts_positions[isec_i*3]));
                            pts_coords.emplace_back(lil_segment->extractPoint(pts_positions[isec_i*3+1]));
                            pts_coords.emplace_back(lil_segment->extractPoint(pts_positions[isec_i*3+2]));
                        }
        
                        // sort positions (dumb algorithm as arrays are small)
                        for (auto isec_i = 0; isec_i < 3*npoints; isec_i++) {
                            for (auto isec_j = isec_i; isec_j < 3*npoints; isec_j++) {
                                if (pts_positions[isec_i] > pts_positions[isec_j]) {
                                    std::swap(pts_positions[isec_i], pts_positions[isec_j]);
                                    std::swap(pts_coords[isec_i], pts_coords[isec_j]);
                                }
                            }
                        }
        
                        for (auto ipt = 0; ipt < 3*npoints; ipt++) {
                            // point given by GEOS is in cartesian coordinates;
                            // if original path is in mixed coordinates (i.e. c is not zero),
                            // we need to find where the new point is along segment,
                            // and then we need to interpolate c value as well
                            auto lx = point1->x - point0->x;
                            auto ly = point1->y - point0->y;
                            auto lz = point1->z - point0->z;
                            auto dlx = pts_coords[ipt].x - point0->x;
                            auto dly = pts_coords[ipt].y - point0->y;
                            auto dlz = pts_coords[ipt].z - point0->z;
                            auto seg_length2 = pow(lx, 2) + pow(ly, 2) + pow(lz, 2);
                            auto dseg_length2 = pow(dlx, 2) + pow(dly, 2) + pow(dlz, 2);
                            double rel_pos = 0;
                                if (seg_length2 > 0) rel_pos = sqrt(dseg_length2 / seg_length2);
                            // interpolate points using values in original coordinates
                            auto p0_orig = (*path)[it_point-1];
                            auto p1_orig = (*path)[it_point];
                            new_path->emplace_back(p0_orig + rel_pos*(p1_orig - p0_orig));
                        }
                    }
                    // 2nd segment point becomes next segment's 1st point
                    point0 = point1;
                }
                // add last point to new path
                auto it_point = path->size() - 1;
                new_path->emplace_back((*path)[it_point]);
            } else {
                new_path = path->copy();
            }

            // check whether path points are inside or outside surface
            std::vector<std::shared_ptr<Point>> is_inside, is_outside;
            for (auto pt: *new_path) {
                auto point = pt->as_geos_geometry();
                if (prepared->intersects(point.get())) {
                    is_inside.emplace_back(pt);
                } else {
                    is_outside.emplace_back(pt);
                }
            }
        
            if (outside) {
                // set every point outside surface to safe height
                for (auto pt: is_outside)
                    pt->z = safe_height;
            } else {
                // set every point inside surface to safe height
                for (auto pt: is_inside)
                    pt->z = safe_height;
            }
           new_paths.emplace_back(new_path);
        }
        return new_paths;
    }


    std::shared_ptr<PathGroup> Surface::correct_height(std::shared_ptr<const PathGroup> pg, const double clearance, const double safe_height, const bool outside, const bool fix_contours) const {
        return std::make_shared<PathGroup>(this->correct_height(pg->get_paths(), clearance, safe_height, outside, fix_contours));
    }

    void py_surface_exports(py::module_ & mod) {
        py::enum_<BooleanOperation>(mod, "BooleanOperation")
        .value("Union", BooleanOperation::Union)
        .value("Difference", BooleanOperation::Difference)
        .value("SymmetricDifference", BooleanOperation::SymmetricDifference)
        .value("Intersection", BooleanOperation::Intersection);
    
        py::class_<Surface, std::shared_ptr<Surface>>(mod, "Surface")
        .def(py::init<>())
        .def(py::init<std::shared_ptr<Path>>())
        .def(py::init<const std::vector<std::shared_ptr<Path>> &>(), py::arg("contour"))
        .def(py::init<std::shared_ptr<Path>, const std::vector<std::shared_ptr<Path>> &>(), py::arg("contour"), py::arg("holes"))
        .def(py::init<const std::vector<std::shared_ptr<Path>> &, const std::vector<std::shared_ptr<Path>> &>(), py::arg("contours"), py::arg("holes"))
        .def(py::init<const std::shared_ptr<Surface>, const std::vector<std::shared_ptr<Path>> &>(), py::arg("contours"), py::arg("holes"))
        .def(py::init<const std::shared_ptr<Surface>, const std::shared_ptr<Surface>>(), py::arg("contours"), py::arg("holes"))
        .def_property("contours", &Surface::get_contours, &Surface::set_contours, py::return_value_policy::reference)
        .def_property("holes", &Surface::get_holes, &Surface::set_holes, py::return_value_policy::reference)
        .def("get_milling_paths", &Surface::get_milling_paths, py::arg("tool_size"), py::arg("increment"))
        .def("get_milled_surface", &Surface::get_milled_surface, py::arg("tool_size"), py::arg("increment"))
        .def("contains", &Surface::contains, py::arg("point"))
        .def("combine", &Surface::combine)
        .def("boolean_operation", &Surface::boolean_operation, py::arg("other"), py::arg("operation_type"))
        .def("__add__", &add_objects<Surface>, py::is_operator()) // boolean union
        .def("__sub__", &sub_objects<Surface>, py::is_operator()) // boolean difference
        .def("__mul__", &mul_object<Surface, std::shared_ptr<Surface>>, py::is_operator()) // boolean intersection
        .def("correct_height", static_cast<std::vector<std::shared_ptr<Path>> (Surface::*)(const std::vector<std::shared_ptr<Path>>&, const double, const double, const bool, const bool) const>(&Surface::correct_height), py::arg("paths"), py::arg("clearance"), py::arg("safe_height"), py::arg("outside")=true, py::arg("fix_contours")=false)
        .def("correct_height", static_cast<std::shared_ptr<PathGroup> (Surface::*)(std::shared_ptr<const PathGroup>, const double, const double, const bool, const bool) const>(&Surface::correct_height), py::arg("pathgroup"), py::arg("clearance"), py::arg("safe_height"), py::arg("outside")=true, py::arg("fix_contours")=false)
            ;
    }

}