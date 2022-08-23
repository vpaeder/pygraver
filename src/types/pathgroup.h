/** \file pathgroup.h
 *  \brief Header file for PathGroup class and associated enums.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <vector>

#include "path.h"

namespace py = pybind11;

namespace pygraver::types {

    class Path;
    class Surface;
    class Point;

    /** \brief Definition of sorting strategies for sort_paths method. */
    enum class SortPredicate : uint8_t {
        StartToStart = 0, /**< start point to start point */
        EndToStart = 1, /**< end point to start point */
        EndToEnd = 2 /**< end point to end point */
    };

    /** \brief Class representing a group of paths. */
    class PathGroup {
    private:
        /** \brief Paths. */
        std::vector<std::shared_ptr<Path>> paths;

        /** \brief Initialize instance. */
        void initialize();

    public:

        /** \brief Default constructor. */
        PathGroup();

        /** \brief Constructor with paths.
         * 
         *  Create a path group with given paths.
         * 
         *  \param paths: paths to initialize group with.
         */
        PathGroup(const std::vector<std::shared_ptr<Path>> & paths);

        /** \brief Destructor. */
        ~PathGroup();

        /** \brief Subscript operator.
         *  \param idx: path index to access.
         *  \returns reference to path.
         */
        std::shared_ptr<Path> operator[] (unsigned int idx);

        /** \brief Subscript operator (const version).
         *  \param idx: path index to access.
         *  \returns const reference to path.
         */
        const std::shared_ptr<Path> operator[] (unsigned int idx) const;
    
        /** \brief Get iterator to path group beginning.
         *  \returns iterator to path group beginning.
         */
        auto begin() { return this->paths.begin(); }
        /** \brief Get iterator to path group beginning (const version).
         *  \returns iterator to path group beginning.
         */
        auto begin() const { return this->paths.begin(); }

        /** \brief Get iterator to path group end.
         *  \returns iterator to path group end.
         */
        auto end() { return this->paths.end(); }
        /** \brief Get iterator to path group end (const version).
         *  \returns iterator to path group end.
         */
        auto end() const { return this->paths.end(); }

        /** \brief Get path group size (i.e. number of paths in group).
         *  \returns number of paths in path group.
         */
        size_t size() const;

        /** \brief Reserve memory for given number of paths.
         *  \param n: number of paths.
         */
        void reserve(const size_t n);

        /** \brief Resize path group.
         *  \param n: new path group size.
         */
        void resize(const size_t n);

        /** \tparam Args: path classes (must be Path).
         *  \param args: paths to append.
         */
        template <class... Args> void emplace_back(Args&&... args) {
            this->paths.emplace_back(args...);
        }

        /** \brief Append path to path group.
         *  \param p: path to append.
         */
        void push_back(std::shared_ptr<Path> p) {
            this->paths.push_back(p);
        }

        /** \brief Create a copy.
         *  \returns a copy of the PathGroup object.
         */
        std::shared_ptr<PathGroup> copy() const;

        /** \brief Get cartesian representation of paths.
         *  \returns path group with paths in cartesian coordinates (xyz).
         */
        std::shared_ptr<PathGroup> to_cartesian() const;

        /** \brief Get polar representation of paths.
         *  
         *  In the x-y plane, this converts the representation
         *  to r-theta. Stores r in x component and theta in c.
         *  
         *  \returns path group with paths in polar coordinates.
         */
        std::shared_ptr<PathGroup> to_polar() const;

        /** \brief Get cylindrical representation of paths.
         *  
         *  This maps the paths around a cylinder of given radius.
         *  The cylinder axis is y.
         * 
         *  \param radius: cylinder radius.
         *  \returns path group with paths in cylindrical coordinates.
         */
        std::shared_ptr<PathGroup> to_cylindrical(const double radius) const;
    
        /** \brief Get path group envelope.
         *  \returns the paths representing the envelope of the path group.
         */
        std::vector<std::shared_ptr<Path>> get_envelope() const;

        /** \brief Get relative displacements between paths.
         *  \returns a collection of points representing the relative displacements between paths.
         */
        std::vector<std::shared_ptr<Point>> get_steps() const;

        /** \brief Set relative displacements between paths.
         *  \param steps: a collection of points representing the relative displacements between paths.
         */
        void set_steps(const std::vector<std::shared_ptr<Point>> & steps);

        /** \brief Get radius of path group (i.e. the largest distance to group centroid).
         *  \returns the radius of the path group.
         */
        double get_radius() const;

        /** \brief Get path centroid.
         *  \returns centroid point.
         */
        std::shared_ptr<Point> get_centroid() const;

        /** \brief Shift path group by given distance.
         *  \param pt: amount to shift by.
         *  \returns shifted path group.
         */
        std::shared_ptr<PathGroup> shift(std::shared_ptr<const Point> pt) const;

        /** \brief Scale path group.
         *  \param factor: scaling factor.
         *  \param ct: centre point.
         *  \returns scaled path group.
         */
        std::shared_ptr<PathGroup> scale(const double factor, std::shared_ptr<Point> ct) const;

        /** \brief Scale path group to given size.
         *  \param target_size: target size.
         *  \param ct: centre point.
         *  \returns scaled path group.
         */
        std::shared_ptr<PathGroup> scale_to_size(const double target_size, std::shared_ptr<Point> ct) const;
        
        /** \brief Mirror path group.
         *  \param along_x: if true, mirror along x axis
         *  \param along_y: if true, mirror along y axis
         *  \param along_z: if true, mirror along z axis
         *  \returns mirrored path group.
         */
        std::shared_ptr<PathGroup> mirror(const bool along_x, const bool along_y, const bool along_z) const;

        /** \brief Rotate path group.
         *  \param yaw_angle: amount of rotation around z axis.
         *  \param pitch_angle: amount of rotation around y axis.
         *  \param roll_angle: amount of rotation around x axis.
         *  \param radians: if true, angle is in radians. If false, angle is in degrees.
         *  \returns rotated path group.
         */
        std::shared_ptr<PathGroup> rotate(const double yaw_angle, const double pitch_angle, const double roll_angle, const bool radians=false) const;

        /** \brief Matrix transform.
         *  \param components: matrix components as double vector with 16 elements.
         *  \returns transformed path group.
         */
        std::shared_ptr<PathGroup> matrix_transform(const std::vector<double> & components) const;

        /** \brief Inflate path group.
         * 
         *  \param amount: amount to inflate.
         *  \returns inflated path group.
         */
        std::shared_ptr<PathGroup> inflate(const double amount) const;

        /** \brief Buffer path group.
         * 
         *  \param amount: amount to buffer.
         *  \param cap_style: style for end caps (round, flat or square).
         *  \param join_style: style for joins (round, mitre or bevel).
         *  \param mitre_limit: mitre limit.
         *  \returns buffered path group.
         */
        std::shared_ptr<PathGroup> buffer(const double amount,
                                     const gob::BufferParameters::EndCapStyle cap_style = gob::BufferParameters::CAP_ROUND,
                                     const gob::BufferParameters::JoinStyle join_style = gob::BufferParameters::JOIN_ROUND,
                                     const double mitre_limit = 1.0) const;

        /** \brief Simplify path group.
         *  \param tolerance: simplification tolerance.
         *  \returns simplified path group.
         */
        std::shared_ptr<PathGroup> simplify(const double tolerance) const;

        /** \brief Interpolate path group.
         *  \param dl: distance between interpolated points.
         *  \returns interpolated path group.
         */
        std::shared_ptr<PathGroup> interpolate(const double dl) const;

        /** \brief Flip path group.
         *  \returns flipped path group.
         */
        std::shared_ptr<PathGroup> flip() const;

        /** \brief Simplify path group above given height.
         *  
         *  Points above given height are removed.
         *  
         *  \param height: threshold height.
         *  \returns simplified path group.
         */
        std::shared_ptr<PathGroup> simplify_above(const double height) const;

        /** \brief Split paths in pathgroup above given height.
         *  \param height: threshold height.
         *  \returns path group containing split paths.
         */
        std::shared_ptr<PathGroup> split_above(const double height) const;

        /** \param limit_height: threshold height. Crossing this height is considered
         *  a discontinuity.
         *  \param ramp_height: height of ramps.
         *  \param ramp_length: length of ramps.
         *  \param direction: ramp direction.
         *  \returns path group with ramps.
         */
        std::shared_ptr<PathGroup> create_ramps(const double limit_height, const double ramp_height, const double ramp_length, const RampDirection direction) const;

        /** \brief Create downward ramps near discontinuities.
         *  \param limit_height: threshold height. Crossing this height is considered
         *  a discontinuity.
         *  \param ramp_height: height of ramps.
         *  \param ramp_length: length of ramps.
         *  \returns path group with ramps.
         */
        std::shared_ptr<PathGroup> create_backward_ramps(const double limit_height, const double ramp_height, const double ramp_length) const;

        /** \brief Create upward ramps near discontinuities.
         *  \param limit_height: threshold height. Crossing this height is considered
         *  a discontinuity.
         *  \param ramp_height: height of ramps.
         *  \param ramp_length: length of ramps.
         *  \returns path group with ramps.
         */
        std::shared_ptr<PathGroup> create_forward_ramps(const double limit_height, const double ramp_height, const double ramp_length) const;

        /** \brief Sort paths in order to reduce distance between them.
         *  \param ref_point: reference point.
         *  \param predicate: sorting predicate.
         *  \returns path group containing sorted paths.
         */
        std::shared_ptr<PathGroup> sort_paths(std::shared_ptr<Point> ref_point, const SortPredicate predicate=SortPredicate::EndToStart) const;

        /** \brief Rearrange paths in such a way that it starts at a discontinuity.
         * 
         *  This version takes the first path point of each path as reference.
         * 
         *  \param limit_height: threshold height. Crossing this height is considered
         *  a discontinuity.
         *  \returns path group containing re-arranged paths.
         */
        std::shared_ptr<PathGroup> rearrange(const double limit_height) const;

        /** \brief Sort paths according to given order.
         *  \param order: a vector of indices giving new path group order.
         *  \returns path group containing reordered paths.
         */
        std::shared_ptr<PathGroup> reorder(const std::vector<unsigned int> & order) const;
    
        /** \brief Get paths.
         *  \returns the paths contained in the path group.
         */
        const std::vector<std::shared_ptr<Path>> & get_paths() const;

        /** \brief Set paths.
         *  \param paths: the paths to assign to the path group.
         */
        void set_paths(const std::vector<std::shared_ptr<Path>> & paths);

        /** \brief Wrapper for subscript operator.
         *  \param idx: path index to access.
         *  \returns reference to path.
         */
        std::shared_ptr<Path> py_get_item(int idx) const;

        /** \brief Get path group slice.
         *  \param slice: Python slice object.
         *  \returns copy of path group slice.
         */
        std::vector<std::shared_ptr<Path>> py_get_item_slice(const py::slice & slice) const;

        /** \brief Set path at given index.
         *  \param idx: path index.
         *  \param path: path.
         */
        void py_set_item(const int idx, std::shared_ptr<Path> path);

        /** \brief Set paths in given slice.
         *  \param slice: Python slice object.
         *  \param ps: path values.
         */
        void py_set_item_slice(const py::slice & slice, const std::vector<std::shared_ptr<Path>> & ps);
        
    };

    /** \brief Addition operator.
     *  \param p: first path group.
     *  \param q: second path group.
     *  \returns a concatenation of path groups p and q.
     */
    std::shared_ptr<PathGroup> operator+ (std::shared_ptr<const PathGroup> p, std::shared_ptr<const PathGroup> q);

    /** \brief Addition operator.
     *  \param p: path group.
     *  \param q: path.
     *  \returns the p path group with appended q path.
     */
    std::shared_ptr<PathGroup> operator+ (std::shared_ptr<const PathGroup> p, std::shared_ptr<Path> q);

    /** \brief Right multiplication operator.
     *  \param p: path group.
     *  \param n: number of copies.
     *  \returns path group consisting of n copies of path group p.
     */
    std::shared_ptr<PathGroup> operator* (std::shared_ptr<const PathGroup> p, const unsigned int n);

    /** \brief Left multiplication operator.
     *  \param n: number of copies.
     *  \param p: path group.
     *  \returns path group consisting of n copies of path group p.
     */
    std::shared_ptr<PathGroup> operator*(const unsigned int n, std::shared_ptr<const PathGroup> p);

    /** \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_pathgroup_exports(py::module_ & mod);

}