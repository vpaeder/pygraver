/** \file surface.h
 *  \brief Header file for Surface class and associated enums.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <geos/geom/Geometry.h>
#include <pybind11/pybind11.h>

#include "point.h"

namespace py = pybind11;

/** \brief Shorthand for geos::geom::Geometry class. */
using GEOSGeometry = geos::geom::Geometry;

namespace pygraver::types {

    class Path;
    class PathGroup;

    /** \brief Definition of boolean operation types. */
    enum class BooleanOperation : uint8_t {
        Union = 0, /**< union */
        Difference = 1, /**< difference */
        SymmetricDifference = 2, /**< symmetric difference */
        Intersection = 3 /**< intersection */
    };

    /** \brief Class representing a surface composed of one or more contours. */
    class Surface {
    private:
        /** \brief Surface contours. */
        std::vector<std::shared_ptr<Path>> contours;

        /** \brief Surface holes.
         * 
         *  This is the contours of holes (a.k.a. inner contours).
         */
        std::vector<std::shared_ptr<Path>> holes;

        /** \brief Initialize instance. */
        void initialize();
    
    public:
        /** \brief Default constructor. */
        Surface();

        /** \brief Constructor with single contour.
         *  \param contour: a path representing the surface contour.
         */
        Surface(std::shared_ptr<const Path> contour);

        /** \brief Constructor with single contour and holes.
         *  \param contour: a path representing the surface contour.
         *  \param holes: a collection of inner contours.
         */
        Surface(std::shared_ptr<const Path> contour, const std::vector<std::shared_ptr<Path>> & holes);

        /** \brief Constructor with multiple contours.
         *  \param contours: a collection of paths representing the surface contours.
         */
        Surface(const std::vector<std::shared_ptr<Path>> & contours);

        /** \brief Constructor with multiple contours and holes.
         *  \param contours: a collection of paths representing the surface contours.
         *  \param holes: a collection of inner contours.
         */
        Surface(const std::vector<std::shared_ptr<Path>> & contours, const std::vector<std::shared_ptr<Path>> & holes);

        /** \brief Constructor with surface contour and holes.
         *  \param surface: a Surface object used as surface contour.
         *  \param holes: a collection of inner contours.
         */
        Surface(const std::shared_ptr<Surface> surface, const std::vector<std::shared_ptr<Path>> & holes);

        /** \brief Constructor with surface as contours and holes.
         *  \param surface: a Surface object used as surface contour.
         *  \param holes: a Surface object used as inner holes.
         */
        Surface(const std::shared_ptr<Surface> surface, const std::shared_ptr<Surface> holes);

        /** \brief Destructor. */
        ~Surface();
        
        /** \brief Get surface contours.
         *  \returns the collection of paths representing the surface contours.
         */
        const std::vector<std::shared_ptr<Path>> & get_contours() const;

        /** \brief Get surface inner contours.
         *  \returns the collection of paths representing the surface inner contours.
         */
        const std::vector<std::shared_ptr<Path>> & get_holes() const;

        /** \brief Set surface contours.
         *  \param contours: a collection of paths representing the surface contours.
         */
        void set_contours(const std::vector<std::shared_ptr<Path>> & contours);

        /** \brief Set surface inner contours.
         *  \param holes: a collection of inner contours.
         */
        void set_holes(const std::vector<std::shared_ptr<Path>> & holes);

        /** \brief Compute path necessary to mill surface with given parameters.
         * 
         *  Paths are computed incrementally from centroid point with given increment.
         *  Surfaces with concave features with radii smaller than tool_size/2 will
         *  be smoothed.
         * 
         *  \param tool_size: diameter of endmill.
         *  \param increment: increment between consecutive paths (>0).
         *  \returns a collection of milling paths.
         */
        std::vector<std::shared_ptr<Path>> get_milling_paths(const double tool_size, const double increment) const;

        /** \brief Compute surface milled with given parameters.
         *  \param tool_size: diameter of endmill.
         *  \param increment: increment between consecutive paths (>0).
         *  \returns a collection of milled surfaces.
         */
        std::vector<std::shared_ptr<Surface>> get_milled_surface(const double tool_size, const double increment) const;

        /** \brief Tell if given point is inside surface.
         *  \param p: point to test for.
         *  \returns true if point is inside surface, false otherwise.
         */
        bool contains(std::shared_ptr<const Point> p) const;

        /** \brief Convert surface to a GEOS Geometry object.
         *  \returns a GEOS Geometry object containing surface data.
         */
        std::unique_ptr<GEOSGeometry> as_geos_geometry() const;

        /** \brief Combine contours and produce well-defined surfaces.
         * 
         *  This splits surfaces into single continuous ones with their
         *  respective inner contours.
         * 
         *  \returns a collection of surfaces.
         */
        std::vector<std::shared_ptr<Surface>> combine() const;

        /** \brief Get surface centroid.
         *  \returns centroid point.
         */
        std::shared_ptr<Point> get_centroid() const;

        /** \brief Apply a boolean operation between two surfaces.
         *  \param other: second surface to perform operation with.
         *  \param operation_type: the type of boolean operation to apply.
         *  \returns a collection of surfaces resulting from the boolean operation.
         */
        std::vector<std::shared_ptr<Surface>> boolean_operation(std::shared_ptr<const Surface> other, const BooleanOperation operation_type) const;

        /** \brief Correct height of given paths.
         *  
         *  I wrote this function with fabrication in mind. For a number of operations
         *  (milling, engraving, turning, ...), we need enough clearance to pass
         *  the tool; besides, it needs to be made to follow the surface.
         *  This function adapts the height of the path points for this purpose, based
         *  on whether they are inside or outside the surface.
         * 
         *  \param paths: a collection of paths to correct.
         *  \param clearance: clearance near a contour.
         *  \param safe_height: height in exclusion area (outside or inside).
         *  \param outside: if true, exclusion area is outside surface; if false, exclusion area is inside.
         *  \param fix_contours: if true, adds supplementary points on contours to increase accuracy.
         *  \returns a collection of corrected paths.
         */
        std::vector<std::shared_ptr<Path>> correct_height(const std::vector<std::shared_ptr<Path>> & paths, const double clearance,
                                                          const double safe_height, const bool outside=true,
                                                          const bool fix_contours=false) const;

        /** \brief Correct height of given paths.
         *  \param pg: path group containing paths to correct.
         *  \param clearance: clearance near a contour.
         *  \param safe_height: height in exclusion area (outside or inside).
         *  \param outside: if true, exclusion area is outside surface; if false, exclusion area is inside.
         *  \param fix_contours: if true, adds supplementary points on contours to increase accuracy.
         *  \returns a path group containing corrected paths.
         */
        std::shared_ptr<PathGroup> correct_height(std::shared_ptr<const PathGroup> pg, const double clearance,
                                                  const double safe_height, const bool outside=true,
                                                  const bool fix_contours=false) const;

    };

    /** \brief Addition operator. This computes the boolean union.
     *  \param s1: first surface.
     *  \param s2: second surface.
     *  \returns the surfaces resulting from the union operation.
     */
    std::vector<std::shared_ptr<Surface>> operator+(std::shared_ptr<const Surface> s1, std::shared_ptr<const Surface> s2);

    /** \brief Subtraction operator. This computes the boolean difference s1-s2.
     *  \param s1: first surface.
     *  \param s2: second surface.
     *  \returns the surfaces resulting from the difference operation.
     */
    std::vector<std::shared_ptr<Surface>> operator-(std::shared_ptr<const Surface> s1, std::shared_ptr<const Surface> s2);

    /** \brief Multiplication operator. This computes the boolean intersection s1*s2.
     *  \param s1: first surface.
     *  \param s2: second surface.
     *  \returns the surfaces resulting from the intersection operation.
     */
    std::vector<std::shared_ptr<Surface>> operator*(std::shared_ptr<const Surface> s1, std::shared_ptr<const Surface> s2);

    /** \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_surface_exports(py::module_ & mod);

}