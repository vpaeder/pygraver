/** \file path.cpp
 *  \brief Implementation file for Path class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <geos/geom/Coordinate.h>
#include <geos/geom/Polygon.h>
#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/CoordinateSequence.h"
#include <geos/operation/buffer/BufferParameters.h>
#include <geos/operation/buffer/BufferBuilder.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/linearref/LengthIndexedLine.h>

#include <pybind11/stl.h>

#include "common.h"
#include "path.h"
#include "point.h"
#include "../log.h"

/** \brief Shorthand for geos::geom::Coordinate class. */
using GEOSCoordinate = geos::geom::Coordinate;
/** \brief Shorthand for geos::geom::Polygon class. */
using GEOSPolygon = geos::geom::Polygon;
/** \brief Shorthand for geos::geom::CoordinateArraySequence class. */
using GEOSCoordinateArraySequence = geos::geom::CoordinateArraySequence;
/** \brief Shorthand for geos::geom::CoordinateSequence class. */
using GEOSCoordinateSequence = geos::geom::CoordinateSequence;
/** \brief Shorthand for geos::operation::buffer::BufferParameters class. */
using GEOSBufferParameters = gob::BufferParameters;
/** \brief Shorthand for geos::operation::buffer::BufferBuilder class. */
using GEOSBufferBuilder = gob::BufferBuilder;

namespace pygraver::types {

    void Path::initialize() {
        if (!gfactory.get()) {
            auto pm = std::make_unique<gg::PrecisionModel>();
            gfactory = gg::GeometryFactory::create(pm.get());
        }
        PYG_LOG_V("Creating path 0x{:x}", (uint64_t)this);
    }

    Path::Path(const Path & p) {
        PYG_LOG_V("Copy-constructing path 0x{:x} to 0x{:x}", (uint64_t)&p, (uint64_t)this);
        this->reserve(p.size());
        std::copy(p.begin(), p.end(), std::back_inserter(this->pts));
    }

    Path & Path::operator=(const Path & p) {
        PYG_LOG_V("Copy-assigning path 0x{:x} to 0x{:x}", (uint64_t)&p, (uint64_t)this);
        this->reserve(p.size());
        std::copy(p.begin(), p.end(), std::back_inserter(this->pts));
        return *this;
    }

    Path::Path(Path && p) {
        PYG_LOG_V("Move-constructing path 0x{:x} to 0x{:x}", (uint64_t)&p, (uint64_t)this);
        this->reserve(p.size());
        std::move(p.begin(), p.end(), std::back_inserter(this->pts));
    }

    Path & Path::operator=(Path && p) {
        PYG_LOG_V("Move-assigning path 0x{:x} to 0x{:x}", (uint64_t)&p, (uint64_t)this);
        this->reserve(p.size());
        std::move(p.begin(), p.end(), std::back_inserter(this->pts));
        return *this;
    }

    Path::Path(const size_t n) {
        this->initialize();
        if (n==0) return;
        this->resize(n);
    }

    Path::Path(std::shared_ptr<const Point> p) {
        PYG_LOG_V("Creating path 0x{:x} with point 0x{:x}", (uint64_t)this, (uint64_t)&p);
        this->reserve(1);
        this->pts.emplace_back(std::const_pointer_cast<Point>(p));
    }
    
    Path::Path(const std::vector< std::vector<double> > &v) {
        this->initialize();

        if (v.size()==0)
            return;

        this->reserve(v.size());
        auto nd = v[0].size();
        for (auto & el : v) {
            if (nd>=4) {
                this->pts.emplace_back(std::make_shared<Point>(el[0], el[1], el[2], el[3]));
            } else if (nd==3) {
                this->pts.emplace_back(std::make_shared<Point>(el[0], el[1], el[2], 0));
            } else if (nd==2) {
                this->pts.emplace_back(std::make_shared<Point>(el[0], el[1], 0, 0));
            } else if (nd==1) {
                this->pts.emplace_back(std::make_shared<Point>(el[0], 0, 0, 0));
            }
        }
    }

    Path::Path(const py::array_t<double> & py_xs, const py::array_t<double> & py_ys, const py::array_t<double> & py_zs, const py::array_t<double> & py_cs) {
        this->initialize();
        
        if (py_xs.ndim()==0 && py_xs.ndim()==0 && py_xs.ndim()==0 && py_xs.ndim()==0)
            return;
        
        if (py_xs.ndim() > 1) {
            PyErr_SetString(PyExc_ValueError, "xs must be an array of dimension 1.");
            throw py::error_already_set();
        }
        if (py_ys.ndim() > 1) {
            PyErr_SetString(PyExc_ValueError, "ys must be an array of dimension 1.");
            throw py::error_already_set();
        }
        if (py_zs.ndim() > 1) {
            PyErr_SetString(PyExc_ValueError, "zs must be an array of dimension 1.");
            throw py::error_already_set();
        }
        if (py_cs.ndim() > 1) {
            PyErr_SetString(PyExc_ValueError, "cs must be an array of dimension 1.");
            throw py::error_already_set();
        }

        auto nx = (py_xs.ndim() == 1) ? py_xs.shape(0) : 0;
        auto ny = (py_ys.ndim() == 1) ? py_ys.shape(0) : 0;
        auto nz = (py_zs.ndim() == 1) ? py_zs.shape(0) : 0;
        auto nc = (py_cs.ndim() == 1) ? py_cs.shape(0) : 0;
        auto n = std::max({nx, ny, nz, nc});

        if (n==0) return;

        std::unique_ptr<double[]> zeros;
        if (nx == 0 || ny == 0 || nz == 0 || nc == 0)
            zeros = std::make_unique<double[]>(n);

        auto xs = (nx == 0) ? zeros.get() : static_cast<double*>(py_xs.request().ptr);
        auto ys = (ny == 0) ? zeros.get() : static_cast<double*>(py_ys.request().ptr);
        auto zs = (nz == 0) ? zeros.get() : static_cast<double*>(py_zs.request().ptr);
        auto cs = (nc == 0) ? zeros.get() : static_cast<double*>(py_cs.request().ptr);
        this->reserve(n);
        for (auto i=0; i<n; i++)
            this->pts.emplace_back(std::make_shared<Point>(xs[i], ys[i], zs[i], cs[i]));
    }
    
    Path::Path(const std::vector<std::shared_ptr<Point>> & points) {
        unsigned int n = points.size();
        if (n==0) return;
        this->initialize();
        this->reserve(n);
        std::copy(points.begin(), points.end(), std::back_inserter(this->pts));
    }
    
    Path::~Path() {
        PYG_LOG_V("Deleting path 0x{:x}", (uint64_t)this);
    }

    std::shared_ptr<Path> Path::copy() const {
        PYG_LOG_V("Copying path 0x{:x}", (uint64_t)this);
        auto new_path = std::make_shared<Path>(0);
        new_path->reserve(this->pts.size());
        for (auto p: this->pts)
            new_path->emplace_back(p->copy());
        return new_path;
    }

    void Path::resize(const size_t n) {
        PYG_LOG_V("Resizing path 0x{:x} to size {:d}", (uint64_t)this, n);
        this->pts.resize(n);
    }

    void Path::reserve(const size_t n) {
        this->pts.reserve(n);
    }

    size_t Path::size() const {
        return this->pts.size();
    }

    std::shared_ptr<Point> Path::operator[] (unsigned int idx) {
        if(idx>=this->size())
			throw std::out_of_range("Index out of range.");

        return this->pts[idx];
    }
    const std::shared_ptr<Point> Path::operator[] (unsigned int idx) const {
        return this->pts[idx];
    }

    std::unique_ptr<GEOSLineString> Path::as_open_geos_geometry() const {
        PYG_LOG_V("Creating open GEOS geometry for path 0x{:x}", (uint64_t)this);
        std::unique_ptr<GEOSCoordinateArraySequence> cs = std::make_unique<GEOSCoordinateArraySequence>(this->size(), 3);
        double x, y, c, s;
        for (auto i=0; i<this->size(); i++) {
            c = cos(this->pts[i]->c/180*M_PI);
            s = sin(this->pts[i]->c/180*M_PI);
            x = this->pts[i]->x*c - this->pts[i]->y*s;
            y = this->pts[i]->x*s + this->pts[i]->y*c;
            cs->setAt(GEOSCoordinate(x, y, this->pts[i]->z), i);
        }
        std::unique_ptr<GEOSLineString> ls = gfactory->createLineString(std::move(cs));
        return ls;
    }

    std::unique_ptr<GEOSLinearRing> Path::as_closed_geos_geometry() const {
        PYG_LOG_V("Creating closed GEOS geometry for path 0x{:x}", (uint64_t)this);
        double x, y, c, s;
        size_t max_n = this->is_closed() ? this->size()-1 : this->size();
        std::unique_ptr<GEOSCoordinateSequence> cs = std::make_unique<GEOSCoordinateArraySequence>(max_n+1, 3);
        for (auto i=0; i<max_n; i++) {
            c = cos(this->pts[i]->c/180*M_PI);
            s = sin(this->pts[i]->c/180*M_PI);
            x = this->pts[i]->x*c - this->pts[i]->y*s;
            y = this->pts[i]->x*s + this->pts[i]->y*c;
            cs->setAt(GEOSCoordinate(x, y, this->pts[i]->z), i);
            if (i==0)
                cs->setAt(GEOSCoordinate(x, y, this->pts[i]->z), max_n);
        }
        std::unique_ptr<GEOSLinearRing> lr = gfactory->createLinearRing(std::move(cs));
        PYG_LOG_D("Geometry is {}valid", lr->isValid() ? "" : "in");
        PYG_LOG_D("Geometry is {}empty", lr->isEmpty() ? "" : "not ");
        PYG_LOG_D("Geometry is {}simple", lr->isSimple() ? "" : "not ");
        PYG_LOG_D("Number of points for geometry: {}", lr->getNumPoints());
        PYG_LOG_D("Geometry length: {}", lr->getLength());
        return lr;
    }

    double Path::get_largest_radius() const {
        PYG_LOG_V("Computing radius for path 0x{:x}", (uint64_t)this);
        auto centroid = this->get_centroid();
        double ri, rmax = 0;
        for (auto p: this->pts) {
            ri = sqrt(pow(p->x - centroid->x, 2) + pow(p->y - centroid->y, 2) + pow(p->z - centroid->z, 2));
            rmax = (ri>rmax) ? ri : rmax;
        }
        return rmax;
    }
    
    double Path::get_length() const {
        auto ls = this->as_open_geos_geometry();
        return ls->getLength();
    }

    std::vector<double> Path::get_radii() const {
        PYG_LOG_V("Computing radii for path 0x{:x}", (uint64_t)this);
        std::vector<double> radii;
        auto n = this->size();
        radii.reserve(n);
        auto centroid = this->get_centroid();
        for (auto p: this->pts)
            radii.emplace_back(sqrt(pow(p->x - centroid->x, 2) + pow(p->y - centroid->y, 2) + pow(p->z - centroid->z, 2)));
        
        return radii;
    }

    std::vector<double> Path::get_angles(const bool radians) const {
        PYG_LOG_V("Computing angles for path 0x{:x}", (uint64_t)this);
        std::vector<double> angles;
        auto n = this->size();
        angles.reserve(n);
        auto factor = radians ? M_PI/180 : 1;
        auto corr_fct = radians ? angle_norm_rad : angle_norm;
        angles.emplace_back(this->pts[0]->angle(radians));
        for (auto i=1; i<n; i++) {
            angles.emplace_back(this->pts[i]->angle(radians));
            angles[i] = angles[i-1] + corr_fct(angles[i] - angles[i-1]);
        }
        return angles;
    }

    std::vector<double> Path::get_elevations(const bool radians) const {
        PYG_LOG_V("Computing elevations for path 0x{:x}", (uint64_t)this);
        std::vector<double> angles;
        auto n = this->size();
        angles.reserve(n);
        auto factor = radians ? M_PI/180 : 1;
        auto corr_fct = radians ? angle_norm_rad : angle_norm;
        for (auto pt: this->pts)
            angles.emplace_back(pt->elevation());
        return angles;
    }

    std::shared_ptr<Point> Path::get_centroid() const {
        PYG_LOG_V("Computing centroid for path 0x{:x}", (uint64_t)this);
        if (this->size()>2) {
            // creates GEOS polygon for path
            auto linear_ring = this->as_closed_geos_geometry();
            if (!linear_ring->isRing())
                return std::make_shared<Point>();
            auto path_polygon = gfactory->createPolygon(std::move(linear_ring));
            auto centroid_point = path_polygon->getCentroid();
            // GEOS algorithm for 2D only; for Z, compute average
            double z = 0;
            auto max_n = this->is_closed() ? this->size()-1 : this->size();
            for (auto pt: this->pts)
                z += pt->z/max_n;
            return std::make_shared<Point>(centroid_point->getX(), centroid_point->getY(), z);
    
        } else if (this->size()==1 || this->size()==2) {
            // centroid is only valid for an area => if less than 3 points, returns average
            auto avg = std::make_shared<Point>(0,0,0,0);
            auto npts = this->pts.size();
            for (auto pt: this->pts) {
                auto t = pt->c/180*M_PI;
                auto c = cos(t);
                auto s = sin(t);
                avg->x += (c*pt->x - s*pt->y)/npts;
                avg->y += (s*pt->x + c*pt->y)/npts;
                avg->z += pt->z/npts;
            }
            return avg;
        } else {
            return std::make_shared<Point>();
        }
    }

    std::shared_ptr<Path> Path::shift(std::shared_ptr<const Point> p) const {
        PYG_LOG_V("Shifting path 0x{:x} by point 0x{:x}", (uint64_t)this, (uint64_t)p.get());
        auto new_path = std::make_shared<Path>(0);
        new_path->reserve(this->size());
        double x, y, c, s;
        for (auto q: this->pts) {
            c = cos(q->c/180*M_PI);
            s = sin(q->c/180*M_PI);
            x = c*q->x - s*q->y + p->x;
            y = s*q->x + c*q->y + p->y;
            new_path->emplace_back(
                std::make_shared<Point>(x*c + y*s, -x*s + y*c, q->z + p->z, q->c + p->c)
            );
        }
        return new_path;
    }

    std::shared_ptr<Path> Path::scale(const double factor, std::shared_ptr<const Point> ct) const {
        PYG_LOG_V("Scaling path 0x{:x} by factor {:f}", (uint64_t)this, factor);
        auto new_path = this->copy();
        for (auto pt: new_path->pts) {
            auto t = pt->c/180*M_PI;
            auto c = cos(t);
            auto s = sin(t);
            auto xb = (c*pt->x - s*pt->y)*factor;
            auto yb = (c*pt->y + s*pt->x)*factor;
            pt->x = c*xb + s*yb;
            pt->y = c*yb - s*xb;
            pt->z *= factor;
        }
        auto scaled_path = new_path->shift(-ct*factor);
        return scaled_path;
    }
    
    std::shared_ptr<Path> Path::mirror(const bool along_x, const bool along_y, const bool along_z) const {
        PYG_LOG_V("Mirroring path 0x{:x}", (uint64_t)this);
        if (!along_x && !along_y && !along_z)
            return this->copy();
        
        auto new_path = this->copy();
        for (auto pt: *new_path) {
            if (along_x)
                pt->x = -pt->x;
            if (along_y)
                pt->y = -pt->y;
            if (along_z)
                pt->z = -pt->z;
        }
        return new_path;
    }
    
    std::shared_ptr<Path> Path::rotate(const double yaw_angle, const double pitch_angle, const double roll_angle, const bool radians) const {
        PYG_LOG_V("Rotating path 0x{:x}", (uint64_t)this);
        auto new_path = std::make_shared<Path>(0);
        auto factor = radians ? 1 : M_PI/180;
        new_path->reserve(this->size());
        auto yaw_c = cos(yaw_angle*factor);
        auto yaw_s = sin(yaw_angle*factor);
        auto pitch_c = cos(pitch_angle*factor);
        auto pitch_s = sin(pitch_angle*factor);
        auto roll_c = cos(roll_angle*factor);
        auto roll_s = sin(roll_angle*factor);
        for (auto p: this->pts)
            new_path->emplace_back(std::make_shared<Point>(
                yaw_c*pitch_c*p->x + (yaw_c*pitch_s*roll_s - yaw_s*roll_c)*p->y + (yaw_c*pitch_s*roll_c + yaw_s*roll_s)*p->z,
                yaw_s*pitch_c*p->x + (yaw_s*pitch_s*roll_s + yaw_c*roll_c)*p->y + (yaw_s*pitch_s*roll_c - yaw_c*roll_s)*p->z,
                -pitch_s*p->x + pitch_c*roll_s*p->y + pitch_c*roll_c*p->z,
                p->c
            ));
        return new_path;
    }
    
    std::shared_ptr<Path> Path::matrix_transform(const std::vector<double> & components) const {
        PYG_LOG_V("Matrix transform for path 0x{:x}", (uint64_t)this);
        auto new_path = this->copy();
        for (auto pt: new_path->pts) {
            auto t = pt->c/180*M_PI;
            auto c = cos(t);
            auto s = sin(t);
            auto xa = c*pt->x - s*pt->y;
            auto ya = c*pt->y + s*pt->x;
            auto xb = components[0]*xa + components[1]*ya + components[2]*pt->z + components[3];
            auto yb = components[4]*xa + components[5]*ya + components[6]*pt->z + components[7];
            pt->z = components[8]*xa + components[9]*ya + components[10]*pt->z + components[11];
            pt->x = c*xb + s*yb;
            pt->y = c*yb - s*xb;
        }
        return new_path;
    }

    std::shared_ptr<Path> Path::matrix_transform(const std::vector<std::vector<double>> & matrix) const {
        if (matrix.size() != 4 || matrix[0].size() != 4)
            throw std::invalid_argument("Transform matrix must be 4x4.");
        std::vector<double> components;
        components.reserve(9);
        components.insert(components.end(), matrix[0].begin(), matrix[0].end());
        components.insert(components.end(), matrix[1].begin(), matrix[1].end());
        components.insert(components.end(), matrix[2].begin(), matrix[2].end());
        components.insert(components.end(), matrix[3].begin(), matrix[3].end());
        return this->matrix_transform(components);
    }

    std::shared_ptr<Path> Path::inflate(const double amount) const {
        PYG_LOG_V("Inflating path 0x{:x}", (uint64_t)this);
        double rmax = this->get_largest_radius();
        return this->scale((rmax + amount)/rmax, this->get_centroid());
    }
    
    std::shared_ptr<Path> Path::buffer(const double amount,
                      const gob::BufferParameters::EndCapStyle cap_style,
                      const gob::BufferParameters::JoinStyle join_style,
                      const double mitre_limit) const {
        PYG_LOG_V("Buffering path 0x{:x}", (uint64_t)this);
        auto boundary = this->as_closed_geos_geometry();
        // this is the advanced way of buffering: create buffer parameters with cap and join styles,
        // create a buffer builder, build buffer for geometry.
        auto buffer_params = GEOSBufferParameters(16, cap_style, join_style, mitre_limit);
        auto buffer_builder = std::make_unique<GEOSBufferBuilder>(buffer_params);
        auto polygon = std::unique_ptr<GEOSPolygon>(gfactory->createPolygon(std::move(boundary)));
        auto bpoly = std::unique_ptr<GEOSGeometry>(buffer_builder->buffer(polygon.get(), amount));
        // that would be the simpler buffering method, but this doesn't let one choose join style (only cap style)
        //std::unique_ptr<GEOSGeometry> bpoly = polygon->buffer(amount, 16, geos::operation::buffer::BufferOp::CAP_SQUARE);
        return make_path(bpoly->getBoundary().get());
    }
    
    std::shared_ptr<Path> Path::close() const {
        PYG_LOG_V("Closing path 0x{:x}", (uint64_t)this);
        auto new_path = this->copy();
        if (!this->is_closed())
            new_path->emplace_back(this->pts[0]);
        return new_path;
    }

    std::vector<std::shared_ptr<Path>> Path::convex_hull() const {
        PYG_LOG_V("Computing convex hull for path 0x{:x}", (uint64_t)this);
        auto boundary = this->as_closed_geos_geometry();
        auto hull = boundary->convexHull();
        std::vector<std::shared_ptr<Path>> hulls;
        auto n = hull->getNumGeometries();
        hulls.reserve(n);
        for (auto i=0; i<n; i++) {
            auto env_i = hull->getGeometryN(i);
            if (env_i->getGeometryTypeId() == geos::geom::GEOS_POLYGON) {
                auto poly_i = dynamic_cast<const GEOSPolygon*>(env_i);
                auto bnd_i = poly_i->getExteriorRing();
                hulls.emplace_back(make_path(bnd_i));
            }
        }
        return hulls;
    }

    std::shared_ptr<Path> Path::simplify(const double tolerance) const {
        PYG_LOG_V("Simplifying path 0x{:x}", (uint64_t)this);
        auto ls = this->as_open_geos_geometry();
        auto simplifier = geos::simplify::TopologyPreservingSimplifier(ls.get());
        simplifier.setDistanceTolerance(tolerance);
        auto result = simplifier.getResultGeometry();
        return make_path(result.get());
    }

    std::shared_ptr<Path> Path::interpolate(double dl) const{
        PYG_LOG_V("Interpolating path 0x{:x} with step {:f}", (uint64_t)this, dl);
        auto ls = this->as_open_geos_geometry();
        auto linref = std::make_unique<geos::linearref::LengthIndexedLine>(ls.get());
        auto length = ls->getLength();
        auto new_path = std::make_shared<Path>(0);
        auto Np = std::ceil(length/dl);
        new_path->reserve(Np);
        dl = length / Np;
        
        for (auto np = 0; np <= Np; np++) {
            auto coords = linref->extractPoint(dl*np);
            new_path->emplace_back(std::make_shared<Point>(coords.x, coords.y, coords.z));
        }
        return new_path;
    }
    
    std::shared_ptr<Path> Path::to_cartesian() const {
        PYG_LOG_V("Converting path 0x{:x} to cartesian coordinates", (uint64_t)this);
        // converts path to cartesian coordinates
        auto n = this->size();
        auto new_path = std::make_shared<Path>(0);
        auto rs = this->get_radii();
        auto ts = this->get_angles(true);
        new_path->reserve(n);

        for (auto pt: this->pts) {
            auto theta = pt->angle(true);
            auto t = pt->c/180*M_PI;
            auto c = cos(t);
            auto s = sin(t);
            new_path->emplace_back(std::make_shared<Point>(
                pt->x*cos(t) - pt->y*sin(t),
                pt->y*cos(t) + pt->x*sin(t),
                pt->z));
        }
        return new_path;
    }

    std::shared_ptr<Path> Path::to_polar() const {
        PYG_LOG_V("Converting path 0x{:x} to polar coordinates", (uint64_t)this);
        // converts path to polar coordinates (plane perpendicular to z)
        auto n = this->size();
        auto new_path = std::make_shared<Path>(0);
        new_path->reserve(n);
        auto rs = this->get_radii();
        auto ts = this->get_angles(false);
        for (auto i=0; i<n; i++)
            new_path->emplace_back(std::make_shared<Point>(rs[i], 0, this->pts[i]->z, ts[i]));
        
        return new_path;
    }

    std::shared_ptr<Path> Path::to_cylindrical(const double radius) const {
        PYG_LOG_V("Converting path 0x{:x} to cylindrical coordinates", (uint64_t)this);
        // converts path to cylindrical coordinates (cylinder along x)
        auto n = this->size();
        auto new_path = std::make_shared<Path>(0);
        new_path->reserve(n);
        for (auto p: this->pts) {
            double c = cos(p->c/180*M_PI), s = sin(p->c/180*M_PI);
            new_path->emplace_back(std::make_shared<Point>(p->x, radius*c + p->y, radius*s + p->z));
        }
        return new_path;
    }
    
    std::vector<double> Path::divergence(const DivComponent cmp) const {
        PYG_LOG_V("Computing divergence for path 0x{:x}", (uint64_t)this);
        if (this->size()<=1)
			throw std::out_of_range("Path length must be larger than 1.");

        if (cmp == DivComponent::DxDx || cmp == DivComponent::DyDy || cmp == DivComponent::DzDz) {
            std::vector<double> div;
            div.resize(this->size());
            std::fill(div.begin(), div.end(), 1);
            return div;
        }
        std::vector<double> us, ds;
        us.reserve(this->size());
        ds.reserve(this->size());
        auto xy = this->to_cartesian();
        for (auto p: xy->pts) {
            if (cmp == DivComponent::DxDy) {
                us.emplace_back(p->x);
                ds.emplace_back(p->y);
            } else if (cmp == DivComponent::DxDz) {
                us.emplace_back(p->x);
                ds.emplace_back(p->z);
            } else if (cmp == DivComponent::DyDx) {
                us.emplace_back(p->y);
                ds.emplace_back(p->x);
            } else if (cmp == DivComponent::DyDz) {
                us.emplace_back(p->y);
                ds.emplace_back(p->z);
            } else if (cmp == DivComponent::DzDx) {
                us.emplace_back(p->z);
                ds.emplace_back(p->x);
            } else if (cmp == DivComponent::DzDy) {
                us.emplace_back(p->z);
                ds.emplace_back(p->y);
            }
        }
        std::vector<double> div;
        div.reserve(this->size());
        div.emplace_back((us[1] - us[0])/(ds[1] - ds[0]));
        for (auto i=1; i<this->size()-1; i++) {
            auto hs = ds[i] - ds[i-1];
            auto hd = ds[i+1] - ds[i];
            auto hs2 = pow(hs,2);
            auto hd2 = pow(hd,2);
            div.emplace_back((hs2*us[i+1] + (hd2-hs2)*us[i] - hd2*us[i-1])/(hs*hd*(hs+hd)));
        }
        div.emplace_back((us.back() - us[us.size()-2])/(ds.back() - ds[ds.size()-2]));
        return div;
    }
    
    std::vector<double> Path::tangent_angle(const bool radians) const {
        PYG_LOG_V("Computing tangent angles for path 0x{:x}", (uint64_t)this);
        if (this->size()<=1)
			throw std::out_of_range("Path length must be larger than 1.");
        
        std::vector<double> grd;
        grd.reserve(this->size());
        auto xy = this->to_cartesian();
        double factor = radians ? 1 : 180/M_PI;
        // compute tangent angle for each point
        grd.emplace_back(atan2(xy->pts[1]->y-xy->pts[0]->y, xy->pts[1]->x-xy->pts[0]->x)*factor);
        for (auto i=1; i<this->size()-1; i++) {
            auto hs = xy->pts[i]->x - xy->pts[i-1]->x;
            auto hd = xy->pts[i+1]->x - xy->pts[i]->x;
            auto hs2 = pow(hs,2);
            auto hd2 = pow(hd,2);
            grd.emplace_back(atan2((xy->pts[i]->y - xy->pts[i-1]->y)*hd2 + (xy->pts[i+1]->y - xy->pts[i]->y)*hs2, (hd+hs)*hd*hs)*factor);
        }
        grd.emplace_back(atan2(xy->pts.back()->y-xy->pts[this->size()-2]->y, xy->pts.back()->x-xy->pts[this->size()-2]->x)*factor);
        return grd;
    }
    
    bool Path::is_ccw() const {
        PYG_LOG_V("Checking path 0x{:x} winding", (uint64_t)this);
        // compute normal vector to determine its orientation;
        // if positive, curve is CCW; if negative, curve is CW.
        if (this->size()<3) return false;
        auto cart = this->to_cartesian();
        auto centroid = this->get_centroid();
        auto normal = std::make_shared<Point>();
        auto last_it = this->is_closed() ? cart->end()-3 : cart->end()-2;
        for (auto it = cart->begin(); it != last_it; it++) {
            auto pt1 = *(it+1) - *it;
            std::shared_ptr<Point> pt2;
            if (it != last_it)
                pt2 = *(it+2) - *(it+1);
            else
                pt2 = *cart->begin() - *(it+1);
            normal->x += pt1->y*pt2->z - pt2->y*pt1->z;
            normal->y += pt1->z*pt2->x - pt2->z*pt1->x;
            normal->z += pt1->x*pt2->y - pt2->x*pt1->y;
        }
        PYG_LOG_V("Path normal: {:f}, {:f}, {:f}", normal->x, normal->y, normal->z);
        normal = normal*(1/normal->radius());
        // we define winding wrt coordinate axes, with z as dominant axis;
        // if normal->z>0, polygon is ccw; if normal->z<0, it is cw;
        // if abs(normal->z)<1e-6, we use y component instead, with
        // the same criteria to define winding;
        // if abs(normal->y)<1e-6, we use x component instead.
        // Note that we could also give the possibility to provide
        // a reference axis and define winding with dot(normal, ref)>0
        if (!almost_equal(normal->z, 0.0, 6)) {
            return normal->z>0;
        } else if (!almost_equal(normal->y, 0.0, 6)) {
            return normal->y>0;
        } else {
            return normal->x>0;
        }
    }
    
    bool Path::is_closed() const {
        PYG_LOG_V("Checking if path 0x{:x} is closed", (uint64_t)this);
        if (this->size()<=2) return false; // can't have a closed geometry with less than 3 points
        auto c0 = this->pts.front()->to_cartesian();
        auto c1 = this->pts.back()->to_cartesian();
        return (
            almost_equal<double>(c0->x, c1->x, 6)
            && almost_equal<double>(c0->y, c1->y, 6)
            && almost_equal<double>(c0->z, c1->z, 6));
    }

    std::shared_ptr<Path> Path::flip() const {
        PYG_LOG_V("Flipping path 0x{:x}", (uint64_t)this);
        auto new_path = std::make_shared<Path>(0);
        new_path->reserve(this->size());
        std::reverse_copy(this->begin(), this->end(), std::back_inserter(new_path->pts));
        return new_path;
    }

    std::shared_ptr<Path> Path::simplify_above(const double height) const {
        PYG_LOG_V("Simplifying path 0x{:x} above {:f}", (uint64_t)this, height);
        auto n = this->size();
        if (n==0) return this->copy();
        std::vector<unsigned int> ih; // indices for which the z component is above 'height'
        std::vector<unsigned int> ikeep; // indices that will be kept
        std::vector<unsigned int> irem; // indices that will be removed
        double c_correction = 0; // correction factor for c parameter, if 2 points are separated by more than 360 degrees
        // pick elements above height
        for (auto i=0; i<n; i++) {
            if (this->pts[i]->z>height)
                ih.emplace_back(i);
            else
                ikeep.emplace_back(i);
        }

        auto new_path = std::make_shared<Path>(0);
        auto last_c = this->pts[0]->c;
        bool is_above = this->pts[0]->z>height;
        for (auto i=0; i<n-1; i++) {
            if (this->pts[i]->z<=height) {
                new_path->emplace_back(this->pts[i]->copy());
                new_path->pts.back()->c += c_correction;
                if (this->pts[i+1]->z>height && !is_above) {
                    // adjust c correction if current c value is more than 360 apart from previous one
                    c_correction -= std::round((this->pts[i]->c - last_c)/360)*360;
                    last_c = this->pts[i]->c;
                    is_above = true;
                    new_path->emplace_back(
                        std::make_shared<Point>(this->pts[i]->x, this->pts[i]->y, this->pts[i+1]->z, this->pts[i]->c + c_correction)
                    );
                }
            } else {
                if (this->pts[i+1]->z<=height && is_above) {
                    c_correction -= std::round((this->pts[i+1]->c - last_c)/360)*360;
                    last_c = this->pts[i+1]->c;
                    is_above = false;
                    new_path->emplace_back(
                        std::make_shared<Point>(this->pts[i+1]->x, this->pts[i+1]->y, this->pts[i]->z, this->pts[i+1]->c + c_correction)
                    );
                }
            }
        }
        if (this->pts.back()->z<=height) {
            new_path->emplace_back(this->pts.back()->copy());
            new_path->pts.back()->c += c_correction;
        }
        return new_path;
    }
    
    std::vector<std::shared_ptr<Path>> Path::split_above(const double height) const {
        PYG_LOG_V("Splitting path 0x{:x} above {:f}", (uint64_t)this, height);
        if (this->pts.size()==0) return {};
        std::vector<std::shared_ptr<Path>> new_paths;
        auto new_path = std::make_shared<Path>(0);
        double c_correction = 0; // correction factor for c parameter, if 2 zones are separated by more than 360 degrees
        bool create_new = false;
        auto last_c = this->pts[0]->c;
        for (auto p: this->pts) {
            if (p->z<=height) {
                if (create_new) {
                    c_correction -= std::round((p->c - last_c)/360)*360;
                    last_c = p->c;
                    new_path = std::make_shared<Path>(0);
                    create_new = false;
                }
                new_path->emplace_back(p->copy());
                new_path->pts.back()->c += c_correction;
            } else {
                if (!create_new) {
                    if (new_path->size()>=1)
                        new_paths.emplace_back(new_path);
                    create_new = true;
                }
            }
        }
        if (!create_new && new_path->size()>=1)
            new_paths.emplace_back(new_path);
        return new_paths;
    }

    std::shared_ptr<Path> Path::create_ramps(const double limit_height,
                                             const double ramp_height,
                                             const double ramp_length,
                                             const RampDirection direction) const {
        PYG_LOG_V("Creating ramps for path 0x{:x}", (uint64_t)this);
        auto n = this->size();
        auto new_path = this->copy();
        std::vector<size_t> disconts;
        // add ramp in the beginning or in the end if necessary
        // down ramp if 1st point is below limit and path is open
        if (!this->is_closed() && this->pts[0]->z<=limit_height) {
            PYG_LOG_D("Found discontinuity at index 0 (backward).");
            disconts.emplace_back(0);
        }
    
        for (size_t i=0; i<n-1; i++) {
            if (this->pts[i+1]->z<=limit_height && this->pts[i]->z>limit_height) {
                // backward ramp
                PYG_LOG_D("Found discontinuity at index {} (backward).", i+1);
                disconts.emplace_back(i+1);
            } else if (this->pts[i+1]->z>limit_height && this->pts[i]->z<=limit_height) {
                // forward ramp
                disconts.emplace_back(i);
                PYG_LOG_D("Found discontinuity at index {} (forward).", i);
            }
        }
    
        // up ramp if last point is above limit and path is open
        if (!this->is_closed() && this->pts.back()->z<=limit_height) {
            PYG_LOG_D("Found discontinuity at index {} (forward).", n-1);
            disconts.emplace_back(n-1);
        }
        
        // Loop over discontinuities. From each point, one computes compensations
        // for the next points until ramp length is reached.
        PYG_LOG_D("Found {} discontinuities.", disconts.size());
        for (size_t i=0; i<disconts.size(); i++) {
            int d_idx = disconts[i];
            // preliminary checks: test if discontinuity is of the right kind;
            // if not, skip
            RampDirection disc_type = RampDirection::Both;
            if (d_idx == 0) {
                disc_type = RampDirection::Backward;
            } else if (this->pts[d_idx-1]->z>limit_height) {
                disc_type = RampDirection::Backward;
            } else if (d_idx == this->pts.size()-1) {
                disc_type = RampDirection::Forward;
            } else if (this->pts[d_idx+1]->z>limit_height) {
                disc_type = RampDirection::Forward;
            }
            // skip if discontinuity type is incorrect
            if (direction != disc_type && direction != RampDirection::Both)
                continue;
            
            int next_idx = d_idx; // index of previous/next discontinuity (we don't want to step over)
            if (disc_type == RampDirection::Forward) {
                // forward ramp -> next discontinuity sits before ramp start
                // (we build forward ramp backward from discontinuity to stop point)
                next_idx = (i>0) ? disconts[i-1] : 0;
                // check that next discontinuity is of the right kind (backward); if not, skip
                if (next_idx>0 && this->pts[next_idx-1]->z<=limit_height)
                    continue;
            } else { // backward ramp -> next discontinuity sits after ramp start
                next_idx = (i<disconts.size()-1) ? disconts[i+1] : this->pts.size()-1;
                // check that next discontinuity is of the right kind (forward); if not, skip
                if (next_idx<this->pts.size()-1 && this->pts[next_idx+1]->z<=limit_height)
                    continue;
            }
            // skip single point ramps
            if (next_idx == d_idx) continue;

            GEOSCoordinateArraySequence cs;
            auto ls = gfactory->createLineString(&cs);
            double delta = 0; // length of segment from discontinuity to current point
            // raise points until end point
            int increment = (disc_type == RampDirection::Forward) ? -1 : 1;
            PYG_LOG_D("Creating {}ward ramp between indices {} and {}.",(disc_type == RampDirection::Forward) ? "for":"back", d_idx, next_idx);
            for (;;) {
                PYG_LOG_D("Ramp point #{}.", d_idx);
                auto p_cart = this->pts[d_idx]->to_cartesian();
                cs.add(GEOSCoordinate(p_cart->x, p_cart->y, p_cart->z));
                delta = ls->getLength();
                if (delta>=ramp_length) break;
                new_path->pts[d_idx]->z += ramp_height*(1-delta/ramp_length);
                new_path->pts[d_idx]->z = std::min(new_path->pts[d_idx]->z, ramp_height);
                d_idx += increment;
                if (increment<0 && d_idx<next_idx) break;
                if (increment>0 && d_idx>next_idx) break;
            }
        }
        return new_path;
    }

    std::shared_ptr<Path> Path::create_backward_ramps(const double limit_height, const double ramp_height, const double ramp_length) const {
        return this->create_ramps(limit_height, ramp_height, ramp_length, RampDirection::Backward);
    }

    std::shared_ptr<Path> Path::create_forward_ramps(const double limit_height, const double ramp_height, const double ramp_length) const {
        return this->create_ramps(limit_height, ramp_height, ramp_length, RampDirection::Forward);
    }
    
    std::shared_ptr<Path> Path::rearrange(const double limit_height, std::shared_ptr<const Point> ref_point) const {
        PYG_LOG_V("Rearranging path 0x{:x}", (uint64_t)this);
        // this function tries to find another spot to start from (i.e. at a discontinuity)
        // if no better point is found, returns current path, otherwise rearranges path so that
        // it starts from a true discontinuity
        auto n = this->size();
        size_t discont = 0;
        double cur_dist = 0, min_dist = std::numeric_limits<double>::max();
        auto is_closed = this->is_closed();
        auto new_path = this->copy();
        // remove last point if path is closed
        if (is_closed)
            new_path->pts.pop_back();
        
        for (auto i=0; i<n-1; i++) {
            if (this->pts[i]->z>=limit_height && this->pts[i+1]->z<limit_height) {
                cur_dist = this->pts[i]->distance_to(ref_point);
                if (cur_dist<min_dist) {
                    min_dist = cur_dist;
                    discont = i;
                }
            }
        }
        if (discont>0) {
            std::rotate(new_path->pts.begin(), new_path->pts.begin()+discont, new_path->pts.end());
            // unwrap angles
            for (int i=1; i<new_path->size(); i++)
                new_path->pts[i]->c = new_path->pts[i-1]->c + angle_norm(new_path->pts[i]->c - new_path->pts[i-1]->c);
        }
        if (is_closed)
            new_path->pts.emplace_back(new_path->pts[0]);
        
        return new_path;
    }
    
    std::shared_ptr<Path> Path::rearrange(const double limit_height) const {
        return this->rearrange(limit_height, this->pts[0]);
    }
    
    const std::shared_ptr<Point> Path::py_get_item(int idx) const {
        if (idx<0)
            idx = this->size() + idx;
    
        if (idx>=this->size()) {
            PyErr_SetString(PyExc_ValueError, "Index out of bounds.");
            throw py::error_already_set();
        }
        return this->pts[idx];
    }

    std::vector<std::shared_ptr<Point>> Path::py_get_item_slice(const py::slice & slice) const {
        ssize_t start, stop, step;
        PySlice_Unpack(slice.ptr(), &start, &stop, &step);
        std::vector<std::shared_ptr<Point>> points;
        points.reserve((stop-start+step-1)/step);
        for (auto it = this->pts.begin()+start; it<this->pts.begin()+stop; std::advance(it,step))
            points.emplace_back(*it);
        return points;
    }

    void Path::py_set_item(int idx, std::shared_ptr<Point> pt) {
        if (idx<0)
            idx = this->size() - idx;

        if (idx>=this->size()) {
            PyErr_SetString(PyExc_ValueError, "Index out of bounds.");
            throw py::error_already_set();
        }
        this->pts[idx] = pt;
    }

    void Path::py_set_item_slice(const py::slice & slice, const std::vector<std::shared_ptr<Point>> & pts) {
        auto [start, stop, step] = convert_slice(slice, this->size());
        if ((stop-start+step-1)/step != pts.size()) {
            PyErr_SetString(PyExc_ValueError, "Values and slice are of different sizes.");
            throw py::error_already_set();
        }
        auto pt_it = pts.begin();
        for (auto it = this->pts.begin()+start; it<this->pts.begin()+stop; std::advance(it,step))
            *it = *pt_it++;
    }

    py::array_t<double> Path::py_get_xs() const {
        auto py_res = py::array_t<double>(this->size());
        auto res = static_cast<double *>(py_res.request().ptr);
        for (auto i=0; i<this->size(); i++)
            res[i] = this->pts[i]->x;
        return py_res;
    }

    void Path::py_set_xs(const py::array_t<double> & py_xs) {
        if (py_xs.ndim() != 1) {
            PyErr_SetString(PyExc_ValueError, "Array must be one-dimensional.");
            throw py::error_already_set();
        }
        auto npts = std::min(static_cast<size_t>(py_xs.shape(0)), this->size());
        auto *xs = static_cast<double *>(py_xs.request().ptr);
        for (auto i=0; i<npts; i++)
            this->pts[i]->x = xs[i];
    }

    py::array_t<double> Path::py_get_ys() const {
        auto py_res = py::array_t<double>(this->size());
        auto res = static_cast<double *>(py_res.request().ptr);
        for (auto i=0; i<this->size(); i++)
            res[i] = this->pts[i]->y;
        return py_res;
    }

    void Path::py_set_ys(const py::array_t<double> & py_ys) {
        if (py_ys.ndim() != 1) {
            PyErr_SetString(PyExc_ValueError, "Array must be one-dimensional.");
            throw py::error_already_set();
        }
        auto npts = std::min(static_cast<size_t>(py_ys.shape(0)), this->size());
        auto *ys = static_cast<double *>(py_ys.request().ptr);
        for (auto i=0; i<npts; i++)
            this->pts[i]->y = ys[i];
    }

    py::array_t<double> Path::py_get_zs() const {
        auto py_res = py::array_t<double>(this->size());
        auto res = static_cast<double *>(py_res.request().ptr);
        for (auto i=0; i<this->size(); i++)
            res[i] = this->pts[i]->z;
        return py_res;
    }

    void Path::py_set_zs(const py::array_t<double> & py_zs) {
        if (py_zs.ndim() != 1) {
            PyErr_SetString(PyExc_ValueError, "Array must be one-dimensional.");
            throw py::error_already_set();
        }
        auto npts = std::min(static_cast<size_t>(py_zs.shape(0)), this->size());
        auto *zs = static_cast<double *>(py_zs.request().ptr);
        for (auto i=0; i<npts; i++)
            this->pts[i]->z = zs[i];
    }

    py::array_t<double> Path::py_get_cs() const {
        auto py_res = py::array_t<double>(this->size());
        auto res = static_cast<double *>(py_res.request().ptr);
        for (auto i=0; i<this->size(); i++)
            res[i] = this->pts[i]->c;
        return py_res;
    }

    void Path::py_set_cs(const py::array_t<double> & py_cs) {
        if (py_cs.ndim() != 1) {
            PyErr_SetString(PyExc_ValueError, "Array must be one-dimensional.");
            throw py::error_already_set();
        }
        auto npts = std::min(static_cast<size_t>(py_cs.shape(0)), this->size());
        auto *cs = static_cast<double *>(py_cs.request().ptr);
        for (auto i=0; i<npts; i++)
            this->pts[i]->c = cs[i];
    }

    py::array_t<double> Path::py_radii() const {
        return py::array_t<double>(this->size(), this->get_radii().data());
    }

    py::array_t<double> Path::py_angles() const {
        return py::array_t<double>(this->size(), this->get_angles(false).data());
    }

    py::array_t<double> Path::py_elevations() const {
        return py::array_t<double>(this->size(), this->get_elevations(false).data());
    }

    py::array_t<double> Path::py_xy() const {
        if (this->size() == 0) return py::array_t<double>(0);
        auto cartesian = this->to_cartesian();
        auto n = this->size();
        auto py_res = py::array_t<double>(2*n);
        auto res = static_cast<double *>(py_res.request().ptr);
        for (auto p: cartesian->pts) {
            *res++ = p->x;
            *res++ = p->y;
        }
        return py_res.reshape({(int)n,2});
    }
    
    py::array_t<double> Path::py_div(const DivComponent cmp) const {
        return py::array_t<double>(this->size(), this->divergence(cmp).data());
    }
    
    py::array_t<double> Path::py_tangent_angle(const bool radians) const {
        return py::array_t<double>(this->size(), this->tangent_angle(radians).data());
    }
    
    std::shared_ptr<Path> operator+(std::shared_ptr<const Path> p, std::shared_ptr<const Path> q) {
        PYG_LOG_V("Adding paths 0x{:x} and 0x{:x}", (uint64_t)p.get(), (uint64_t)q.get());
        auto np = p->size(), nq = q->size();
        auto new_path = p->copy();
        new_path->reserve(np+nq);
        for (auto i=0; i<nq; i++) {
            new_path->emplace_back((*q)[i]);
            // unwrap angles to make sure that new values start within +-360deg of previous values
            if (np+i>0)
                (*new_path)[np+i]->c = (*new_path)[np+i-1]->c + angle_norm((*q)[i]->c - (*new_path)[np+i-1]->c);
        }
        return new_path;
    }

    std::shared_ptr<Path> operator+(std::shared_ptr<const Path> p, std::shared_ptr<const Point> q) {
        PYG_LOG_V("Adding point 0x{:x} to path 0x{:x}", (uint64_t)q.get(), (uint64_t)p.get());
        auto new_path = p->copy();
        auto n = p->size();
        new_path->emplace_back(std::const_pointer_cast<Point>(q));
        // unwrap angles to make sure that new values start within +-360deg of previous values
        if (n>0)
            (*new_path)[n]->c = (*new_path)[n-1]->c + angle_norm(q->c - (*new_path)[n-1]->c);
        return new_path;
    }

    std::shared_ptr<Path> operator-(std::shared_ptr<const Path> p) {
        PYG_LOG_V("Negating path 0x{:x}", (uint64_t)p.get());
        auto new_path = std::make_shared<Path>(0);
        new_path->reserve(p->size());
        for (auto p: *p)
            new_path->emplace_back(std::make_shared<Point>(-p->x, -p->y, -p->z, -p->c));

        return new_path;
    }

    std::shared_ptr<Path> operator*(std::shared_ptr<const Path> p, const unsigned int n) {
        PYG_LOG_V("Replicating {:d} times path 0x{:x}", n, (uint64_t)p.get());
        auto np = p->size();
        auto new_path = std::make_shared<Path>(0);
        new_path->reserve(n*np);
        for (auto i=0; i<n; i++) {
            for (auto q: *p)
                new_path->emplace_back(q);
            if (i>0)
                for (auto ip=0; ip<np; ip++)
                    (*new_path)[i*np+ip]->c = (*new_path)[i*np+ip-1]->c + angle_norm((*p)[ip]->c - (*new_path)[i*np+ip-1]->c);
        }
        return new_path;
    }

    std::shared_ptr<Path> operator*(const unsigned int n, std::shared_ptr<const Path> p) {
        return p*n;
    }

    std::shared_ptr<Path> make_path(const GEOSGeometry * g) {
        PYG_LOG_V("Creating path from GEOS geometry 0x{:x}", (uint64_t)g);
        auto coords = g->getCoordinates();
        auto sz = coords->getSize();
        PYG_LOG_D("Extracting geometry of size {}", sz);
        auto new_path = std::make_shared<Path>(0);
        new_path->reserve(sz);
        if (coords->hasZ()) {
            for (auto i=0; i<sz; i++) {
                auto coord = coords->getAt(i);
                new_path->emplace_back(std::make_shared<Point>(coord.x, coord.y, coord.z));
            }
        } else {
            for (auto i=0; i<sz; i++)
                new_path->emplace_back(std::make_shared<Point>(coords->getX(i), coords->getY(i)));
        }
        return new_path;
    }

    void py_path_exports(py::module_ & mod) {
        py::enum_<gob::BufferParameters::EndCapStyle>(mod, "EndCapStyle")
        .value("Round", gob::BufferParameters::CAP_ROUND)
        .value("Flat", gob::BufferParameters::CAP_FLAT)
        .value("Square", gob::BufferParameters::CAP_SQUARE);
        
        py::enum_<gob::BufferParameters::JoinStyle>(mod, "JoinStyle")
        .value("Round", gob::BufferParameters::JOIN_ROUND)
        .value("Mitre", gob::BufferParameters::JOIN_MITRE)
        .value("Bevel", gob::BufferParameters::JOIN_BEVEL);
        
        py::enum_<DivComponent>(mod, "DivComponent")
        .value("DxDx", DivComponent::DxDx)
        .value("DxDy", DivComponent::DxDy)
        .value("DxDz", DivComponent::DxDz)
        .value("DyDx", DivComponent::DyDx)
        .value("DyDy", DivComponent::DyDy)
        .value("DyDz", DivComponent::DyDz)
        .value("DzDx", DivComponent::DzDx)
        .value("DzDy", DivComponent::DzDy)
        .value("DzDz", DivComponent::DzDz);

        py::enum_<RampDirection>(mod, "RampDirection")
        .value("Forward", RampDirection::Forward)
        .value("Backward", RampDirection::Backward)
        .value("Both", RampDirection::Both);

        std::shared_ptr<Path> (Path::*rearrange)(const double, std::shared_ptr<const Point>) const = &Path::rearrange;
        py::class_<Path, std::shared_ptr<Path>>(mod, "Path")
        .def(py::init<const py::array_t<double> &, const py::array_t<double> &, const py::array_t<double> &, const py::array_t<double> &>(),
            py::arg("xs")=py::array_t<double>(), py::arg("ys")=py::array_t<double>(), py::arg("zs")=py::array_t<double>(), py::arg("cs")=py::array_t<double>())
        .def(py::init<std::shared_ptr<const Point>>())
        .def(py::init<const std::vector<std::shared_ptr<Point>>&>())
        .def(py::init<const Path &>())
        .def("simplify_above", &Path::simplify_above, py::arg("limit_height"))
        .def("split_above", &Path::split_above, py::arg("limit_height"))
        .def("create_ramps", &Path::create_ramps, py::arg("limit_height"), py::arg("ramp_height"), py::arg("ramp_length"), py::arg("ramp_direction"))
        .def("create_backward_ramps", &Path::create_backward_ramps, py::arg("limit_height"), py::arg("ramp_height"), py::arg("ramp_length"))
        .def("create_forward_ramps", &Path::create_forward_ramps, py::arg("limit_height"), py::arg("ramp_height"), py::arg("ramp_length"))
        .def("rearrange", rearrange, py::arg("limit_height"), py::arg("ref_point"))
        .def("copy", &Path::copy)
        .def("shift", &Path::shift, py::arg("vector"))
        .def("scale", &Path::scale, py::arg("factor"), py::arg("center"))
        .def("mirror", &Path::mirror, py::arg("along_x"), py::arg("along_y"), py::arg("along_z"))
        .def("rotate", &Path::rotate, py::arg("yaw_angle"), py::arg("pitch_angle"), py::arg("roll_angle"), py::arg("radians")=false)
        .def("matrix_transform", static_cast<std::shared_ptr<Path> (Path::*)(const std::vector<double> &) const>(&Path::matrix_transform), py::arg("components"))
        .def("inflate", &Path::inflate, py::arg("amount"))
        .def("simplify", &Path::simplify, py::arg("tolerance"))
        .def("buffer", &Path::buffer, py::arg("amount"), py::arg("cap_style")=gob::BufferParameters::CAP_ROUND, py::arg("join_style")=gob::BufferParameters::JOIN_ROUND, py::arg("mitre_limit")=1.0)
        .def("close", &Path::close)
        .def("flip", &Path::flip)
        .def("interpolate", &Path::interpolate, py::arg("step_size"))
        .def_property_readonly("rmax", &Path::get_largest_radius)
        .def_property_readonly("length", &Path::get_length)
        .def_property_readonly("centroid", &Path::get_centroid)
        .def_property_readonly("cartesian", &Path::to_cartesian)
        .def_property_readonly("polar", &Path::to_polar)
        .def_property_readonly("convex_hull", &Path::convex_hull)
        .def_property_readonly("is_ccw", &Path::is_ccw)
        .def_property_readonly("is_closed", &Path::is_closed)
        .def("cylindrical", &Path::to_cylindrical, py::arg("radius"))
        .def("__add__", &add_objects<Path>, py::is_operator())
        .def("__add__", &add_objects<Path, Point>, py::is_operator())
        .def("__neg__", &neg_object<Path>, py::is_operator())
        .def("__mul__", &mul_object<Path, unsigned int>, py::is_operator())
        .def("__rmul__", &mul_object<Path, unsigned int>, py::is_operator())
        .def("__getitem__", &Path::py_get_item)
        .def("__getitem__", &Path::py_get_item_slice)
        .def("__setitem__", &Path::py_set_item)
        .def("__setitem__", &Path::py_set_item_slice)
        .def("__len__", &Path::size)
        .def("__iter__", [](std::shared_ptr<Path> p){return py::make_iterator(p->begin(), p->end());}
                       , py::keep_alive<0, 1>())
        .def("append", &Path::push_back, py::arg("point"))
        .def_property("xs", &Path::py_get_xs, &Path::py_set_xs)
        .def_property("ys", &Path::py_get_ys, &Path::py_set_ys)
        .def_property("zs", &Path::py_get_zs, &Path::py_set_zs)
        .def_property("cs", &Path::py_get_cs, &Path::py_set_cs)
        .def_property_readonly("radii", &Path::py_radii)
        .def_property_readonly("angles", &Path::py_angles)
        .def_property_readonly("elevations", &Path::py_elevations)
        .def_property_readonly("xy", &Path::py_xy)
        .def("divergence", &Path::py_div, py::arg("component"))
        .def("tangent_angle", &Path::py_tangent_angle, py::arg("radians")=false)
        ;
    }

}
