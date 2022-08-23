/** \file point.cpp
 *  \brief Implementation file for Point class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <geos/geom/Coordinate.h>

#include "point.h"
#include "common.h"
#include "../log.h"

/** \brief Shorthand for geos::geom::Coordinate class. */
using GEOSCoordinate = geos::geom::Coordinate;

namespace pygraver::types {

    Point::Point(const double x, const double y, const double z, const double c) : x(x), y(y), z(z), c(c) {
        create_geometry_factory();
        PYG_LOG_V("Creating point 0x{:x} with values: {},{},{},{}", (uint64_t)this, x, y, z, c);
    }
    
    Point::Point(const Point & p) {
        PYG_LOG_V("Copy-constructing point 0x{:x} with values: {},{},{},{}", (uint64_t)&p, p.x, p.y, p.z, p.c);
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
        this->c = p.c;
    }

    Point & Point::operator=(const Point & p) {
        PYG_LOG_V("Copy-assigning point 0x{:x} with values: {},{},{},{}", (uint64_t)&p, p.x, p.y, p.z, p.c);
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
        this->c = p.c;
        return *this;
    }
    
    Point::Point(Point && p) {
        PYG_LOG_V("Move-constructing point 0x{:x} with values: {},{},{},{}", (uint64_t)&p, p.x, p.y, p.z, p.c);
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
        this->c = p.c;
    }

    Point & Point::operator=(Point && p) {
        PYG_LOG_V("Move-assigning point 0x{:x} with values: {},{},{},{}", (uint64_t)&p, p.x, p.y, p.z, p.c);
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
        this->c = p.c;
        return *this;
    }

    Point::~Point() {
        PYG_LOG_V("Deleting point 0x{:x} with values: {},{},{},{}", (uint64_t)this, this->x, this->y, this->z, this->c);
    }

    std::shared_ptr<Point> Point::copy() const {
        PYG_LOG_V("Copying point 0x{:x}", (uint64_t)this);
        return std::make_shared<Point>(this->x, this->y, this->z, this->c);
    }

    std::unique_ptr<GEOSPoint> Point::as_geos_geometry() const {
        PYG_LOG_V("Creating GEOS point from point 0x{:x}", (uint64_t)this);
        double r = sqrt(pow(this->x,2) + pow(this->y, 2));
        double t = this->angle()/180*M_PI;
        const GEOSCoordinate c(r*cos(t), r*sin(t), this->z);
        return std::unique_ptr<GEOSPoint>(gfactory->createPoint(c));
    }

    double Point::radius() const {
        return sqrt(pow(this->x,2) + pow(this->y, 2) + pow(this->z, 2));
    }

    double Point::angle(const bool radians) const {
        if (radians)
            return this->c/180*M_PI + atan2(this->y, this->x);
        else
            return this->c + atan2(this->y, this->x)/M_PI*180;
    }

    double Point::elevation(const bool radians) const {
        auto proj = sqrt(pow(this->x,2) + pow(this->y, 2));
        if (radians)
            return atan2(this->z, proj);
        else
            return atan2(this->z, proj)/M_PI*180;
    }
    
    double Point::distance_to(std::shared_ptr<const Point> p) const {
        double x1, x2, y1, y2;
        double c = cos(this->c/180*M_PI), s = sin(this->c/180*M_PI);
        x1 = this->x*c - this->y*s;
        y1 = this->y*c + this->x*s;
        c = cos(p->c/180*M_PI);
        s = sin(p->c/180*M_PI);
        x2 = p->x*c - p->y*s;
        y2 = p->y*c + p->x*s;
        return sqrt(pow(x1-x2, 2) + pow(y1-y2, 2) + pow(this->z-p->z, 2));
    }
    
    std::shared_ptr<Point> Point::to_cartesian() const {
        double c = cos(this->c/180*M_PI), s = sin(this->c/180*M_PI);
        return std::make_shared<Point>(this->x*c - this->y*s, this->y*c + this->x*s, this->z, 0);
    }
    
    std::shared_ptr<Point> Point::to_polar() const {
        return std::make_shared<Point>(sqrt(pow(this->x, 2) + pow(this->y, 2)), 0, this->z, this->angle());
    }

    std::shared_ptr<Point> Point::to_cylindrical(const double radius) const {
        double c = cos(this->c/180*M_PI), s = sin(this->c/180*M_PI);
        return std::make_shared<Point>(this->x, radius*c + this->y, radius*s + this->z, 0);
    }

    std::ostream & operator<<(std::ostream & os, std::shared_ptr<const Point> obj) {
        return os << *obj;
    }
    std::ostream & operator<<(std::ostream & os, const Point & obj) {
        os << "x=" << obj.x << ", y=" << obj.y << ", z=" << obj.z << ", c=" << obj.c;
        return os;
    }

    std::shared_ptr<Point> operator+(std::shared_ptr<const Point> p, std::shared_ptr<const Point> q) {
        return std::make_shared<Point>(p->x + q->x, p->y + q->y, p->z + q->z, p->c + q->c);
    }

    Point operator+(const Point & p, const Point & q) {
        return Point(p.x + q.x, p.y + q.y, p.z + q.z, p.c + q.c);
    }

    std::shared_ptr<Point> operator+=(std::shared_ptr<Point> p, std::shared_ptr<const Point> q) {
        p->x += q->x;
        p->y += q->y;
        p->z += q->z;
        p->c += q->c;
        return p;
    }

    Point & operator+=(Point & p, const Point & q) {
        p.x += q.x;
        p.y += q.y;
        p.z += q.z;
        p.c += q.c;
        return p;
    }

    std::shared_ptr<Point> operator-(std::shared_ptr<const Point> p, std::shared_ptr<const Point> q) {
        return std::make_shared<Point>(p->x - q->x, p->y - q->y, p->z - q->z, p->c - q->c);
    }

    Point operator-(const Point & p, const Point & q) {
        return Point(p.x - q.x, p.y - q.y, p.z - q.z, p.c - q.c);
    }

    std::shared_ptr<Point> operator-(std::shared_ptr<const Point> p) {
        return std::make_shared<Point>(-p->x, -p->y, -p->z, -p->c);
    }

    Point operator-(const Point & p) {
        return Point(-p.x, -p.y, -p.z, -p.c);
    }

    std::shared_ptr<Point> operator*(std::shared_ptr<const Point> p, const double v) {
        return std::make_shared<Point>(p->x*v, p->y*v, p->z*v, p->c*v);
    }

    Point operator*(const Point & p, const double v) {
        return Point(p.x*v, p.y*v, p.z*v, p.c*v);
    }

    std::shared_ptr<Point> operator*(const double v, std::shared_ptr<const Point> p) {
        return p*v;
    }

    Point operator*(const double v, const Point & p) {
        return p*v;
    }

    bool operator==(std::shared_ptr<const Point> p, std::shared_ptr<const Point> q) {
        return *p == *q;
    }

    bool operator==(const Point & p, const Point & q) {
        return almost_equal(p.x, q.x, 6) && almost_equal(p.y, q.y, 6)
            && almost_equal(p.z, q.z, 6) && almost_equal(p.c, q.c, 6);
    }

    bool operator!=(std::shared_ptr<const Point> p, std::shared_ptr<const Point> q) {
        return *p != *q;
    }

    bool operator!=(const Point & p, const Point & q) {
        return !almost_equal(p.x, q.x, 6) || !almost_equal(p.y, q.y, 6)
            || !almost_equal(p.z, q.z, 6) || !almost_equal(p.c, q.c, 6);
    }

    void py_point_exports(py::module_ & mod) {
        py::class_<Point, std::shared_ptr<Point>>(mod, "Point")
            .def(py::init<float, float, float, float>(), py::arg("x")=0, py::arg("y")=0, py::arg("z")=0, py::arg("c")=0)
            .def("__add__", &add_objects<Point>, py::is_operator())
            .def("__sub__", &sub_objects<Point>, py::is_operator())
            .def("__neg__", &neg_object<Point>, py::is_operator())
            .def("__mul__", &mul_object<Point, float>, py::is_operator())
            .def("__rmul__", &mul_object<Point, float>, py::is_operator())
            .def("__eq__", &eq_objects<Point>, py::is_operator())
            .def("__neq__", &neq_objects<Point>, py::is_operator())
            .def("copy", &Point::copy)
            .def("distance_to", &Point::distance_to)
            .def("angle", &Point::angle, py::arg("radians")=false)
            .def("elevation", &Point::elevation, py::arg("radians")=false)
            .def_property_readonly("radius", &Point::radius)
            .def_property_readonly("cartesian", &Point::to_cartesian)
            .def_property_readonly("polar", &Point::to_polar)
            .def("cylindrical", &Point::to_cylindrical, py::arg("radius"))
            .def_readwrite("x", &Point::x)
            .def_readwrite("y", &Point::y)
            .def_readwrite("z", &Point::z)
            .def_readwrite("c", &Point::c)
            .def("__repr__", [](const std::shared_ptr<const Point> p) {
                std::ostringstream os;
                os << "<Point " << (p.get()) << ": " << p << ">";
                return os.str();
            });
    }
}