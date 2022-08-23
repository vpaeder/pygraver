/** \file path.h
 *  \brief Header file for Path class and associated enums.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/operation/buffer/BufferParameters.h>
#include <pybind11/numpy.h>
#include <vector>

/** \brief Shorthand for geos::geom::Geometry class. */
using GEOSGeometry = geos::geom::Geometry;
/** \brief Shorthand for geos::geom::LineString class. */
using GEOSLineString = geos::geom::LineString;
/** \brief Shorthand for geos::geom::LinearRing class. */
using GEOSLinearRing = geos::geom::LinearRing;

namespace gob = geos::operation::buffer;
namespace py = pybind11;

namespace pygraver::types {

    class Point;

    /** \brief Definition of ramp directions. */
    enum class RampDirection : uint8_t {
        Forward = 0, /**< forward direction */
        Backward = 1, /**< backward direction */
        Both = 2 /**< both directions */
    };
    
    /** \brief Definition of divergence tensor components. */
    enum class DivComponent : uint8_t {
        DxDx = 0, /**< dx/dx */
        DxDy = 1, /**< dx/dy */
        DxDz = 2, /**< dx/dz */
        DyDx = 3, /**< dy/dx */
        DyDy = 4, /**< dy/dy */
        DyDz = 5, /**< dy/dz */
        DzDx = 6, /**< dz/dx */
        DzDy = 7, /**< dz/dy */
        DzDz = 8  /**< dz/dz */
    };

    /** \brief Class representing a path in 3+1-dimensional space.
     *
     *  It is composed of Point objects. For details, see Point docs.
     */
    class Path {
    private:
        /** \brief Path points. */
        std::vector<std::shared_ptr<Point>> pts;

        /** \brief Initialize instance. */
        void initialize();
    
    public:
        /** \brief Constructor with size argument.
         * 
         *  Initialize n points. If you want to only to reserve memory
         *  for n points without initializing them, use:
         *      Path p;
         *      p.reserve(n);
         * 
         *  \param n: number of points.
         */
        Path(const size_t n);

        /** \brief Constructor with single point.
         * 
         *  This makes a 1-point path.
         * 
         *  \param p: point to construct path from.
         */
        Path(std::shared_ptr<const Point> p);

        /** \brief Constructor with 2D data array.
         * 
         *  Array 1st dimension is path length.
         *  Array 2nd dimension must be of size 4.
         * 
         *  \param v: data array.
         */
        Path(const std::vector<std::vector<double>> & v);
        
        /** \brief Constructor with NumPy arrays.
         * 
         *  Arrays must either have no size or have a common length.
         * 
         *  \param xs: coordinates for 1st dimension.
         *  \param ys: coordinates for 2nd dimension.
         *  \param zs: coordinates for 3rd dimension.
         *  \param cs: coordinates for 4th dimension.
         */
        Path(const py::array_t<double> & xs = py::array_t<double>(),
             const py::array_t<double> & ys = py::array_t<double>(),
             const py::array_t<double> & zs = py::array_t<double>(),
             const py::array_t<double> & cs = py::array_t<double>());

        /** \brief Constructor with vector of Point objects.
         *  \param points: vector of points.
         */
        Path(const std::vector<std::shared_ptr<Point>> & points);

        /** \brief Destructor. */
        ~Path();

        /** \brief Copy constructor.
         *  \param p: path to copy.
         */
        Path(const Path & p);

        /** \brief Copy-assignment operator.
         *  \param p: path to copy.
         *  \returns created path.
         */
        Path & operator=(const Path & p);

        /** \brief Move constructor.
         *  \param p: path to move.
         */
        Path(Path && p);

        /** \brief Move-assignment operator.
         *  \param p: path to move.
         *  \returns created path.
         */
        Path & operator=(Path && p);

        /** \brief Create a copy.
         *  \returns a copy of the Path object.
         */
        std::shared_ptr<Path> copy() const;

        /** \brief Reserve memory for given number of points.
         *  \param n: number of points.
         */
        void reserve(const size_t n);

        /** \brief Resize path.
         *  \param n: new path size.
         */
        void resize(const size_t n);

        /** \brief Emplace given points in the end of the path.
         *  \tparam Args: point classes (must be Point).
         *  \param args: points to append.
         */
        template <typename... Args> void emplace_back(Args&&... args) {
            this->pts.emplace_back(args...);
        }

        /** \brief Append point to path.
         *  \param pt: point to append.
         */
        void push_back(std::shared_ptr<const Point> pt) {
            this->pts.push_back(std::const_pointer_cast<Point>(pt));
        }

        /** \brief Get path's number of points.
         *  \returns number of points.
         */
        size_t size() const;

        /** \brief Get iterator to path beginning.
         *  \returns iterator to path beginning.
         */
        auto begin() { return this->pts.begin(); }
        /** \brief Get iterator to path beginning (const version).
         *  \returns iterator to path beginning.
         */
        auto begin() const { return this->pts.begin(); }

        /** \brief Get iterator to path end.
         *  \returns iterator to path end.
         */
        auto end() { return this->pts.end(); }
        /** \brief Get iterator to path end (const version).
         *  \returns iterator to path end.
         */
        auto end() const { return this->pts.end(); }

        /** \brief Get reverse iterator to path beginning.
         *  \returns reverse iterator to path beginning.
         */
        auto rbegin() { return this->pts.rbegin(); }
        /** \brief Get reverse iterator to path beginning (const version).
         *  \returns reverse iterator to path beginning.
         */
        auto rbegin() const { return this->pts.rbegin(); }

        /** \brief Get reverse iterator to path end.
         *  \returns reverse iterator to path end.
         */
        auto rend() { return this->pts.rend(); }
        /** \brief Get reverse iterator to path end (const version).
         *  \returns reverse iterator to path end.
         */
        auto rend() const { return this->pts.rend(); }

        /** \brief Subscript operator.
         *  \param idx: point index to access.
         *  \returns reference to point.
         */
        std::shared_ptr<Point> operator[] (unsigned int idx);
        /** \brief Subscript operator (const version).
         *  \param idx: point index to access.
         *  \returns const reference to point.
         */
        const std::shared_ptr<Point> operator[] (unsigned int idx) const;
        
        /****************************************/
        /*    Getters for various properties    */ 
        /****************************************/

        /** \brief Get largest radius (i.e. largest distance to path centroid).
         *  \returns largest radius.
         */
        double get_largest_radius() const;

        /** \brief Get path length.
         * 
         *  This integrates over the path to compute the length.
         *  This is not the number of points (use size() for this).
         *  
         *  \returns path length.
         */
        double get_length() const;

        /** \brief Get radii (i.e. distance of each point to centroid).
         *  \returns vector of radii.
         */
        std::vector<double> get_radii() const;

        /** \brief Get angles (i.e. orientation of point wrt x axis in xy plane).
         *  \param radians: if true, result is in radians. If false, result is in degrees.
         *  \returns vector of angles.
         */
        std::vector<double> get_angles(const bool radians = false) const;

        /** \brief Get elevations (i.e. orientation of point wrt xy plane in z direction).
         *  \param radians: if true, result is in radians. If false, result is in degrees.
         *  \returns vector of angles.
         */
        std::vector<double> get_elevations(const bool radians = false) const;

        /** \brief Get path centroid.
         *  \returns centroid point.
         */
        std::shared_ptr<Point> get_centroid() const;

        /** \brief Tell if path is counter-clockwise.
         *  \returns true if path evolves in a counter-clockwise manner.
         */
        bool is_ccw() const;

        /** \brief Tell if path is closed.
         *  \returns true if path is closed.
         */
        bool is_closed() const;
    
        /*****************************************/
        /*    Conversion methods of all kinds    */ 
        /*****************************************/

        /** \brief Convert path to an open GEOS geometry (LineString).
         *  \returns Pointer to a GEOS LineString object.
         */
        std::unique_ptr<GEOSLineString> as_open_geos_geometry() const;

        /** \brief Convert path to a closed GEOS geometry (LinearRing).
         *  \returns Pointer to a GEOS LinearRing object.
         */
        std::unique_ptr<GEOSLinearRing> as_closed_geos_geometry() const;
    
        /** \brief Shift path by given distance.
         *  \param p: amount to shift by.
         *  \returns shifted path.
         */
        std::shared_ptr<Path> shift(std::shared_ptr<const Point> p) const;

        /** \brief Scale path.
         *  \param factor: scaling factor.
         *  \param ct: center point.
         *  \returns scaled path.
         */
        std::shared_ptr<Path> scale(const double factor, std::shared_ptr<const Point> ct) const;

        /** \brief Mirror path.
         *  \param along_x: if true, mirror along x axis
         *  \param along_y: if true, mirror along y axis
         *  \param along_z: if true, mirror along z axis
         *  \returns mirrored path.
         */
        std::shared_ptr<Path> mirror(const bool along_x, const bool along_y, const bool along_z) const;

        /** \brief Rotate path.
         *  \param yaw_angle: amount of rotation around z axis.
         *  \param pitch_angle: amount of rotation around y axis.
         *  \param roll_angle: amount of rotation around x axis.
         *  \param radians: if true, angle is in radians. If false, angle is in degrees.
         *  \returns rotated path.
         */
        std::shared_ptr<Path> rotate(const double yaw_angle, const double pitch_angle, const double roll_angle, const bool radians=false) const;

        /** \brief Matrix transform.
         *  \param components: matrix components as double vector with 16 elements.
         *  \returns transformed path.
         */
        std::shared_ptr<Path> matrix_transform(const std::vector<double> & components) const;

        /** \brief Matrix transform.
         *  \param matrix: transform matrix as 4x4 double array.
         *  \returns transformed path.
         */
        std::shared_ptr<Path> matrix_transform(const std::vector<std::vector<double>> & matrix) const;

        /** \brief Inflate path.
         * 
         *  If path largest distance to centroid is r, returns path
         *  with largest distance of r+amount.
         * 
         *  \param amount: amount to inflate.
         *  \returns inflated path.
         */
        std::shared_ptr<Path> inflate(const double amount) const;

        /** \brief Buffer path.
         * 
         *  This is an inflation/deflation algorithm provided by GEOS.
         * 
         *  \param amount: amount to buffer.
         *  \param cap_style: style for end caps (round, flat or square).
         *  \param join_style: style for joins (round, mitre or bevel).
         *  \param mitre_limit: mitre limit.
         *  \returns buffered path.
         */
        std::shared_ptr<Path> buffer(const double amount,
                                     const gob::BufferParameters::EndCapStyle cap_style = gob::BufferParameters::CAP_ROUND,
                                     const gob::BufferParameters::JoinStyle join_style = gob::BufferParameters::JOIN_ROUND,
                                     const double mitre_limit = 1.0) const;

        /** \brief Close an open path.
         *  \returns closed path.
         */
        std::shared_ptr<Path> close() const;

        /** \brief Compute convex hull.
         *  \returns convex hull.
         */
        std::vector<std::shared_ptr<Path>> convex_hull() const;

        /** \brief Simplify path.
         *  \param tolerance: simplification tolerance.
         *  \returns simplified path.
         */
        std::shared_ptr<Path> simplify(const double tolerance) const;

        /** \brief Interpolate path.
         *  \param dl: distance between interpolated points.
         *  \returns interpolated path.
         */
        std::shared_ptr<Path> interpolate(const double dl) const;
    
        /** \brief Get cartesian representation of path.
         *  \returns path in cartesian coordinates (xyz).
         */
        std::shared_ptr<Path> to_cartesian() const;

        /** \brief Get polar representation of path.
         *  
         *  In the x-y plane, this converts the representation
         *  to r-theta. Stores r in x component and theta in c.
         *  
         *  \returns path in polar coordinates.
         */
        std::shared_ptr<Path> to_polar() const;

        /** \brief Get cylindrical representation of path.
         *  
         *  This maps the path around a cylinder of given radius.
         *  The cylinder axis is y.
         * 
         *  \param radius: cylinder radius.
         *  \returns path in cylindrical coordinates.
         */
        std::shared_ptr<Path> to_cylindrical(const double radius) const;

        /** \brief Get component of divergence tensor.
         *  \param cmp: tensor component to get.
         *  \returns divergence tensor component as vector.
         */
        std::vector<double> divergence(const DivComponent cmp) const;

        /** \brief Get tangent angles.
         * 
         *  This computes, for each point, the angle that the tangent to
         *  the path makes with origin.
         * 
         *  \param radians: if true, angles are in radians. If false, angles are in degrees.
         *  \returns tangent angles.
         */
        std::vector<double> tangent_angle(const bool radians = false) const;
    
        /** \brief Flip path.
         * 
         *  The result looks exactly the same, only with the points
         *  in reverse order. 
         * 
         *  \returns flipped path.
         */
        std::shared_ptr<Path> flip() const;

        /** \brief Simplify path above given height.
         *  
         *  Points above given height are removed.
         *  
         *  \param height: threshold height.
         *  \returns simplified path.
         */
        std::shared_ptr<Path> simplify_above(const double height) const;

        /** \brief Split path above given height.
         *  \param height: threshold height.
         *  \returns collection of path chunks.
         */
        std::vector<std::shared_ptr<Path>> split_above(const double height) const;

        /** \brief Create ramps near discontinuities.
         *  \param limit_height: threshold height. Crossing this height is considered
         *  a discontinuity.
         *  \param increase: height of ramps.
         *  \param ramp_length: length of ramps.
         *  \param direction: ramp direction.
         *  \returns path with ramps.
         */
        std::shared_ptr<Path> create_ramps(const double limit_height, const double increase,
                          const double ramp_length, const RampDirection direction) const;

        /** \brief Create downward ramps near discontinuities.
         *  \param limit_height: threshold height. Crossing this height is considered
         *  a discontinuity.
         *  \param increase: height of ramps.
         *  \param ramp_length: length of ramps.
         *  \returns path with ramps.
         */
        std::shared_ptr<Path> create_backward_ramps(const double limit_height, const double increase, const double ramp_length) const;

        /** \brief Create upward ramps near discontinuities.
         *  \param limit_height: threshold height. Crossing this height is considered
         *  a discontinuity.
         *  \param increase: height of ramps.
         *  \param ramp_length: length of ramps.
         *  \returns path with ramps.
         */
        std::shared_ptr<Path> create_forward_ramps(const double limit_height, const double increase, const double ramp_length) const;

        /** \brief Rearrange path in such a way that it starts at a discontinuity.
         *  \param limit_height: threshold height. Crossing this height is considered
         *  a discontinuity.
         *  \param ref_point: searches the discontinuity from this point in forward direction.
         *  \returns re-arranged path.
         */
        std::shared_ptr<Path> rearrange(const double limit_height, std::shared_ptr<const Point> ref_point) const;

        /** \brief Rearrange path in such a way that it starts at a discontinuity.
         * 
         *  This version takes the first path point as reference.
         * 
         *  \param limit_height: threshold height. Crossing this height is considered
         *  a discontinuity.
         *  \returns re-arranged path.
         */
        std::shared_ptr<Path> rearrange(const double limit_height) const;

        /********************************************/
        /*    Methods specific to Python wrapper    */ 
        /********************************************/

        /** \brief Wrapper for subscript operator.
         *  \param idx: point index to access.
         *  \returns reference to point.
         */
        const std::shared_ptr<Point> py_get_item(int idx) const;

        /** \brief Get path slice.
         *  \param slice: Python slice object.
         *  \returns copy of path slice.
         */
        std::vector<std::shared_ptr<Point>> py_get_item_slice(const py::slice & slice) const;

        /** \fn void py_set_item(int idx, std::shared_ptr<Point> pt)
         *  \brief Set point at given index.
         *  \param idx: point index.
         *  \param pt: point value.
         */
        void py_set_item(int idx, std::shared_ptr<Point> pt);

        /** \brief Set points in given slice.
         *  \param slice: Python slice object.
         *  \param pts: point values.
         */
        void py_set_item_slice(const py::slice & slice, const std::vector<std::shared_ptr<Point>> & pts);

        /** \brief Get first coordinate of path points.
         *  \returns array of first coordinates.
         */
        py::array_t<double> py_get_xs() const;

        /** \brief Set first coordinate of path points.
         *  \param py_xs: array of first coordinates. Truncated if longer than path.
         */
        void py_set_xs(const py::array_t<double> & py_xs);

        /** \brief Get second coordinate of path points.
         *  \returns array of second coordinates.
         */
        py::array_t<double> py_get_ys() const;

        /** \brief Set second coordinate of path points.
         *  \param py_ys: array of second coordinates. Truncated if longer than path.
         */
        void py_set_ys(const py::array_t<double> & py_ys);

        /** \brief Get third coordinate of path points.
         *  \returns array of third coordinates.
         */
        py::array_t<double> py_get_zs() const;

        /** \brief Set third coordinate of path points.
         *  \param py_zs: array of third coordinates. Truncated if longer than path.
         */
        void py_set_zs(const py::array_t<double> & py_zs);

        /** \brief Get fourth coordinate of path points.
         *  \returns array of fourth coordinates.
         */
        py::array_t<double> py_get_cs() const;

        /** \brief Set fourth coordinate of path points.
         *  \param py_cs: array of fourth coordinates. Truncated if longer than path.
         */
        void py_set_cs(const py::array_t<double> & py_cs);

        /** \brief Get radii. See get_radii method for details.
         *  \returns array of radii.
         */
        py::array_t<double> py_radii() const;

        /** \brief Get angles. See get_angles method for details.
         *  \returns array of angles.
         */
        py::array_t<double> py_angles() const;

        /** \brief Get elevation angles. See get_elevations method for details.
         *  \returns array of angles.
         */
        py::array_t<double> py_elevations() const;

        /** \brief Get x and y coordinates in cartesian representation.
         *  \returns array of coordinates.
         */
        py::array_t<double> py_xy() const;

        /** \brief Get divergence tensor component.
         *  \param cmp: tensor component to get.
         *  \returns divergence tensor component as array.
         */
        py::array_t<double> py_div(const DivComponent cmp) const;

        /** \brief Get tangent angle. See tangent_angle for details.
         *  \returns array tangent angles.
         */
        py::array_t<double> py_tangent_angle(const bool radians=false) const;
    
    };

    /** \brief Addition operator.
     *  \param p: first path.
     *  \param q: second path.
     *  \returns concatenation of paths p and q.
     */
    std::shared_ptr<Path> operator+(std::shared_ptr<const Path> p, std::shared_ptr<const Path> q);

    /** \brief Addition operator.
     *  \param p: path.
     *  \param q: point.
     *  \returns concatenation of point p to path p.
     */
    std::shared_ptr<Path> operator+(std::shared_ptr<const Path> p, std::shared_ptr<const Point> q);

    /** \brief Negation operator.
     *  \param p: path.
     *  \returns path with negated coordinates.
     */
    std::shared_ptr<Path> operator-(std::shared_ptr<const Path> p);

    /** \brief Right multiplication operator.
     *  \param p: path.
     *  \param n: number of copies.
     *  \returns path consisting of n copies of path p.
     */
    std::shared_ptr<Path> operator*(std::shared_ptr<const Path> p, const unsigned int n);

    /** \brief Left multiplication operator.
     *  \param n: number of copies.
     *  \param p: path.
     *  \returns path consisting of n copies of path p.
     */
    std::shared_ptr<Path> operator*(const unsigned int n, std::shared_ptr<const Path> p);

    /** \brief Build a Path object out of a GEOS Geometry object.
     *  \param g: pointer to a GEOS Geometry object.
     *  \returns generated path.
     */
    std::shared_ptr<Path> make_path(const GEOSGeometry * g);

    /** \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_path_exports(py::module_ & mod);

}
