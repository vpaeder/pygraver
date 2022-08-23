/** \file point.h
 *  \brief Header file for Point class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <geos/geom/Point.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

/** \brief Shorthand for geos::geom::Point class. */
using GEOSPoint = geos::geom::Point;

namespace pygraver::types {

    /** \brief Class representing a point in 3+1-dimensional space.
     *
     *  The representation corresponds to a 4-axis setup, with 3 degrees of freedom
     *  Typically, the 3 first axes are linear axes, in x, y and z directions,
     *  and the 4th one (c) controls the rotation in one plane. Depending on setup,
     *  axis c is bound with axes x and y or x and z.
     */
    class Point {
    public:
        /** \brief Position along x axis. */
        double x = 0;

        /** \brief Position along y axis. */
        double y = 0;

        /** \brief Position along z axis. */
        double z = 0;

        /** \brief Position of c axis. */
        double c = 0;

        /** \brief Creates a point with given coordinates.
         *  \param x : value for x coordinate.
         *  \param y : value for y coordinate.
         *  \param z : value for z coordinate.
         *  \param c : value for c coordinate.
         */
        Point(const double x = 0, const double y = 0, const double z = 0, const double c = 0);
        
        /** \brief Copy constructor.
         *  \param p: point to copy.
         */
        Point(const Point & p);

        /** \brief Copy-assignment operator.
         *  \param p: point to copy.
         *  \returns created point.
         */
        Point & operator=(const Point & p);

        /** \brief Move constructor.
         *  \param p: point to move.
         */
        Point(Point && p);

        /** \brief Move-assignment operator.
         *  \param p: point to move.
         *  \returns created point.
         */
        Point & operator=(Point && p);

        /** \brief Destructor. */
        ~Point();

        /** \brief Create a 3D GEOSPoint object.
         *  \returns a pointer to the new GEOSPoint object.
         */
        std::unique_ptr<GEOSPoint> as_geos_geometry() const;
        
        /** \brief Create a copy of the object and returns a pointer to it.
         *  \returns a pointer to the new object.
         */
        std::shared_ptr<Point> copy() const;
        
        /** \brief Compute the distance from the centre in the x-y plane (i.e. radius).
         *  \returns the calculated radius.
         */
        double radius() const;
        
        /** \brief Compute the angle in the x-y plane with respect to the x axis.
         *  \param radians: if true, result is in radians. If false, result is in degrees.
         *  \returns the calculated angle.
         */
        double angle(const bool radians=false) const;

        /** \brief Compute the angle in the z direction with respect to the x-y plane.
         *  \param radians: if true, result is in radians. If false, result is in degrees.
         *  \returns the calculated angle.
         */
        double elevation(const bool radians=false) const;

        /** \brief Compute the distance to the given point.
         *  \param p : pointer to the point to compute the distance from.
         *  \returns the calculated distance.
         */
        double distance_to(std::shared_ptr<const Point> p) const;
        
        /** \brief Construct a copy of the object with a cartesian projection of the c component.
         *  \returns a pointer to the new object.
         */
        std::shared_ptr<Point> to_cartesian() const;
        
        /** \brief Construct a copy of the object with a polar projection of the y component.
         *  \returns a pointer to the new object.
         */
        std::shared_ptr<Point> to_polar() const;
        
        /** \brief Construct a copy of the object projected on a cylinder along x axis.
         *  \param radius : the radius of the cylinder.
         *  \returns a pointer to the new object.
         */
        std::shared_ptr<Point> to_cylindrical(const double radius) const;
    };
    
    /** \brief Insertion operator for Point objects.
     *  \param os : output stream.
     *  \param obj : pointer to Point object.
     *  \returns the output stream with inserted data from the Point object.
     */
    std::ostream & operator<<(std::ostream & os, std::shared_ptr<const Point> obj);

    /** \brief Insertion operator for Point objects.
     *  \param os : output stream.
     *  \param obj : Point object.
     *  \returns the output stream with inserted data from the Point object.
     */
    std::ostream & operator<<(std::ostream & os, const Point & obj);
    
    /** \brief Addition operator for Point objects.
     *  \param p : pointer to first Point object.
     *  \param q : pointer to second Point object.
     *  \returns the sum of the two objects (p+q).
     */
    std::shared_ptr<Point> operator+(std::shared_ptr<const Point> p, std::shared_ptr<const Point> q);

    /** \brief Addition operator for Point objects.
     *  \param p : first Point object.
     *  \param q : second Point object.
     *  \returns the sum of the two objects (p+q).
     */
    Point operator+(const Point & p, const Point & q);

    /** \brief Addition assignment operator for Point objects.
     *  \param p : pointer to first Point object.
     *  \param q : pointer to second Point object.
     *  \returns the sum of the two objects (p+q) stored in p.
     */
    std::shared_ptr<Point> operator+=(std::shared_ptr<Point> p, std::shared_ptr<const Point> q);

    /** \brief Addition assignment operator for Point objects.
     *  \param p : first Point object.
     *  \param q : second Point object.
     *  \returns the sum of the two objects (p+q) stored in p.
     */
    Point & operator+=(Point & p, const Point & q);

    /** \brief Subtraction operator for Point objects.
     *  \param p : pointer to first Point object.
     *  \param q : pointer to second Point object.
     *  \returns the subtraction of object q to object p (p-q).
     */
    std::shared_ptr<Point> operator-(std::shared_ptr<const Point> p, std::shared_ptr<const Point> q);

    /** \brief Subtraction operator for Point objects.
     *  \param p : first Point object.
     *  \param q : second Point object.
     *  \returns the subtraction of object q to object p (p-q).
     */
    Point operator-(const Point & p, const Point & q);

    /** \brief Right multiplication operator for Point objects.
     *  \param p : pointer to Point object.
     *  \param v : multiplier.
     *  \returns the Point object with multiplied coordinates.
     */
    std::shared_ptr<Point> operator*(std::shared_ptr<const Point> p, const double v);

    /** \brief Right multiplication operator for Point objects.
     *  \param p : Point object.
     *  \param v : multiplier.
     *  \returns the Point object with multiplied coordinates.
     */
    Point operator*(const Point & p, const double v);

    /** \brief Left multiplication operator for Point objects.
     *  \param v : multiplier.
     *  \param p : pointer to Point object.
     *  \returns the Point object with multiplied coordinates.
     */
    std::shared_ptr<Point> operator*(const double v, std::shared_ptr<const Point> p);

    /** \brief Left multiplication operator for Point objects.
     *  \param v : multiplier.
     *  \param p : Point object.
     *  \returns the Point object with multiplied coordinates.
     */
    Point operator*(const double v, const Point & p);

    /** \brief Negation operator for Point objects.
     *  \param p : pointer to the Point object to negate.
     *  \returns the negation of the object.
     */
    std::shared_ptr<Point> operator-(std::shared_ptr<const Point> p);

    /** \brief Negation operator for Point objects.
     *  \param p : Point object to negate.
     *  \returns the negation of the object.
     */
    Point operator-(const Point & p);

    /** \brief Comparison operator.
     *  \param p: pointer to first point.
     *  \param q: pointer to second point.
     *  \returns true if points are equal, false otherwise.
     */
    bool operator==(std::shared_ptr<const Point> p, std::shared_ptr<const Point> q);

    /** \brief Comparison operator.
     *  \param p: first point.
     *  \param q: second point.
     *  \returns true if points are equal, false otherwise.
     */
    bool operator==(const Point & p, const Point & q);

    /** \brief Negative comparison operator.
     *  \param p: pointer to first point.
     *  \param q: pointer to second point.
     *  \returns true if points are not equal, false otherwise.
     */
    bool operator!=(std::shared_ptr<const Point> p, std::shared_ptr<const Point> q);

    /** \brief Negative comparison operator.
     *  \param p: first point.
     *  \param q: second point.
     *  \returns true if points are not equal, false otherwise.
     */
    bool operator!=(const Point & p, const Point & q);

    /** \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_point_exports(py::module_ & mod);

}