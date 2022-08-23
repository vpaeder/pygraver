/** \file pathgroup.cpp
 *  \brief Implementation file for PathGroup class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <pybind11/stl.h>

#include "common.h"
#include "pathgroup.h"
#include "path.h"
#include "surface.h"
#include "point.h"
#include "../log.h"

namespace pygraver::types {

    void PathGroup::initialize() {
        PYG_LOG_V("Creating path group 0x{:x}", (uint64_t)this);
        if (!gfactory.get()) {
            gg::PrecisionModel *pm = new gg::PrecisionModel();
            gfactory = gg::GeometryFactory::create(pm);
            delete pm;
        }
    }

    PathGroup::PathGroup() {
        this->initialize();
    }

    PathGroup::PathGroup(const std::vector<std::shared_ptr<Path>> & paths) {
        this->initialize();
        this->paths = paths;
    }

    PathGroup::~PathGroup() {
        PYG_LOG_V("Deleting path group 0x{:x}", (uint64_t)this);
    }

    size_t PathGroup::size() const {
        return this->paths.size();
    }

    std::shared_ptr<Path> PathGroup::operator[] (unsigned int idx) {
        if(idx>=this->size())
			throw std::out_of_range("Index out of range.");
        return this->paths[idx];
    }
    const std::shared_ptr<Path> PathGroup::operator[] (unsigned int idx) const {
        return this->paths[idx];
    }

    std::shared_ptr<PathGroup> PathGroup::copy() const {
        PYG_LOG_V("Copying pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->emplace_back(std::make_shared<Path>(*p));
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::to_cartesian() const {
        PYG_LOG_V("Converting pathgroup 0x{:x} to cartesian coordinates", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->paths.emplace_back(p->to_cartesian());
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::to_polar() const {
        PYG_LOG_V("Converting pathgroup 0x{:x} to polar coordinates", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->paths.emplace_back(p->to_polar());
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::to_cylindrical(const double radius) const {
        PYG_LOG_V("Converting pathgroup 0x{:x} to cylindrical coordinates", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->paths.emplace_back(p->to_cylindrical(radius));
        
        return new_group;
    }

    void PathGroup::reserve(const size_t n) {
        this->paths.reserve(n);
    }

    void PathGroup::resize(const size_t n) {
        this->paths.resize(n);
    }

    std::shared_ptr<PathGroup> operator+ (std::shared_ptr<const PathGroup> p, std::shared_ptr<const PathGroup> q) {
        PYG_LOG_V("Adding pathgroups 0x{:x} and 0x{:x}", (uint64_t)p.get(), (uint64_t)q.get());
        auto np = p->size(), nq = q->size();
        auto new_group = p->copy();
        new_group->reserve(np+nq);
        for (auto path: *q)
            new_group->emplace_back(path);
        return new_group;
    }

    std::shared_ptr<PathGroup> operator+ (std::shared_ptr<const PathGroup> p, std::shared_ptr<Path> q) {
        PYG_LOG_V("Adding path 0x{:x} to pathgroup 0x{:x}", (uint64_t)q.get(), (uint64_t)p.get());
        auto new_group = p->copy();
        new_group->emplace_back(q);
        return new_group;
    }

    std::shared_ptr<PathGroup> operator* (std::shared_ptr<const PathGroup> p, const unsigned int n) {
        PYG_LOG_V("Replicating {:d} times pathgroup 0x{:x}", n, (uint64_t)p.get());
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(n*p->size());
        for (auto i=0; i<n; i++)
            for (auto path: *p)
                new_group->emplace_back(path);
        
        return new_group;
    }

    std::shared_ptr<PathGroup> operator* (const unsigned int n, std::shared_ptr<const PathGroup> p) {
        return p*n;
    }

    std::vector<std::shared_ptr<Path>> PathGroup::get_envelope() const {
        PYG_LOG_V("Computing envelope for pathgroup 0x{:x}", (uint64_t)this);
        std::vector<std::shared_ptr<Path>> paths;
        for (auto s: Surface(this->paths).combine()) {
            auto new_paths = s->get_contours();
            paths.reserve(paths.size() + new_paths.size());
            paths.insert(paths.end(), new_paths.begin(), new_paths.end());
        }
        return paths;
    }

    std::vector<std::shared_ptr<Point>> PathGroup::get_steps() const {
        PYG_LOG_V("Getting steps for pathgroup 0x{:x}", (uint64_t)this);
        std::vector<std::shared_ptr<Point>> steps;
        steps.reserve(this->paths.size()-1);
        for (auto i=1; i<this->size(); i++)
            steps.emplace_back((*this->paths[i])[0] - (*this->paths[i-1])[0]);
        
        return steps;
    }

    void PathGroup::set_steps(const std::vector<std::shared_ptr<Point>> & steps) {
        PYG_LOG_V("Setting steps for pathgroup 0x{:x}", (uint64_t)this);
        if (this->size()<2 || steps.size() == 0) return;
        std::shared_ptr<Point> dp;
        auto ns = std::min(this->size()-1, steps.size());
        for (auto i=1; i<=ns; i++) {
            if (this->paths[i]->size()==0 || this->paths[i-1]->size()==0) continue;
            dp = (*this->paths[i])[0] - (*this->paths[i-1])[0];
            for (auto j=0; j<this->paths[i]->size(); j++)
                (*this->paths[i])[j] += steps[i-1] - dp;
        }
    }

    double PathGroup::get_radius() const {
        PYG_LOG_V("Getting radius for pathgroup 0x{:x}", (uint64_t)this);
        // return max radius
        double rmax = 0;
        for (auto p: this->paths) {
            auto rs = p->get_radii();
            rmax = std::max(rmax, *std::max_element(rs.begin(), rs.end()));
        }
        return rmax;
    }

    std::shared_ptr<Point> PathGroup::get_centroid() const {
        PYG_LOG_V("Getting centroid for pathgroup 0x{:x}", (uint64_t)this);
        return Surface(this->paths).get_centroid();
    }


    std::shared_ptr<PathGroup> PathGroup::shift(std::shared_ptr<const Point> pt) const {
        PYG_LOG_V("Shifting pathgroup 0x{:x} by point 0x{:x}", (uint64_t)this, (uint64_t)pt.get());
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->emplace_back(p->shift(pt));
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::scale(const double factor, std::shared_ptr<Point> ct) const {
        PYG_LOG_V("Scaling pathgroup 0x{:x} by factor {:f}", (uint64_t)this, factor);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        // scale each path
        for (auto p: this->paths)
            new_group->emplace_back(p->scale(factor, ct));
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::scale_to_size(const double target_size, std::shared_ptr<Point> ct) const {
        // scale relative to max radius
        return this->scale(target_size/this->get_radius(), ct);
    }
    
    std::shared_ptr<PathGroup> PathGroup::mirror(const bool along_x, const bool along_y, const bool along_z) const {
        PYG_LOG_V("Mirroring pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        // scale each path
        for (auto p: this->paths)
            new_group->emplace_back(p->mirror(along_x, along_y, along_z));
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::rotate(const double yaw_angle, const double pitch_angle, const double roll_angle, const bool radians) const {
        PYG_LOG_V("Rotating pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->emplace_back(p->rotate(yaw_angle, pitch_angle, roll_angle, radians));
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::matrix_transform(const std::vector<double> & components) const {
        PYG_LOG_V("Matrix transform for pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->emplace_back(p->matrix_transform(components));
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::inflate(const double amount) const {
        PYG_LOG_V("Inflating pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->emplace_back(p->inflate(amount));
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::buffer(const double amount,
                      const gob::BufferParameters::EndCapStyle cap_style,
                      const gob::BufferParameters::JoinStyle join_style,
                      const double mitre_limit) const {
        PYG_LOG_V("Buffering pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->emplace_back(p->buffer(amount, cap_style, join_style, mitre_limit));
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::simplify(const double tolerance) const {
        PYG_LOG_V("Simplifying pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->emplace_back(p->simplify(tolerance));
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::interpolate(double dl) const{
        PYG_LOG_V("Interpolating pathgroup 0x{:x} with step {:f}", (uint64_t)this, dl);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->emplace_back(p->interpolate(dl));
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::flip() const {
        PYG_LOG_V("Flipping pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->emplace_back(p->flip());
        
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::simplify_above(const double height) const {
        PYG_LOG_V("Simplifying pathgroup 0x{:x} above {:f}", (uint64_t)this, height);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths) {
            auto new_path = p->simplify_above(height);
            // check that the new path contains points below height, discard it otherwise
            auto above_height = new_path->size();
            for (auto pt: *p)
                if (pt->z<=height)
                    above_height--;
            
            if (above_height<new_path->size())
                new_group->emplace_back(new_path);
        }
        new_group->reserve(new_group->size());
        return new_group;
    }

    std::shared_ptr<PathGroup> PathGroup::split_above(const double height) const {
        PYG_LOG_V("Splitting pathgroup 0x{:x} above {:f}", (uint64_t)this, height);
        auto new_group = std::make_shared<PathGroup>();
        for (auto p: this->paths) {
            auto new_paths = p->split_above(height);
            if (new_paths.size()) {
                new_group->reserve(new_group->size() + new_paths.size());
                std::copy(new_paths.begin(), new_paths.end(), std::back_inserter(new_group->paths));
            }
        }
        return new_group;
    }
    
    std::shared_ptr<PathGroup> PathGroup::create_ramps(const double limit_height, const double ramp_height, const double ramp_length, const RampDirection direction) const {
        PYG_LOG_V("Creating ramps for pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        for (auto p: this->paths)
            new_group->emplace_back(p->create_ramps(limit_height, ramp_height, ramp_length, direction));
        return new_group;
    }
    
    std::shared_ptr<PathGroup> PathGroup::create_forward_ramps(const double limit_height, const double ramp_height, const double ramp_length) const {
        return this->create_ramps(limit_height, ramp_height, ramp_length, RampDirection::Forward);
    }
    
    std::shared_ptr<PathGroup> PathGroup::create_backward_ramps(const double limit_height, const double ramp_height, const double ramp_length) const {
        return this->create_ramps(limit_height, ramp_height, ramp_length, RampDirection::Backward);
    }
    
    std::shared_ptr<PathGroup> PathGroup::sort_paths(std::shared_ptr<Point> ref_point, const SortPredicate predicate) const {
        PYG_LOG_V("Sorting paths for pathgroup 0x{:x}", (uint64_t)this);
        if (this->size()==0) return std::make_shared<PathGroup>();
        std::vector<std::shared_ptr<Point>> starts, ends;
        std::vector<std::shared_ptr<Path>> unassigned;
        auto new_group = std::make_shared<PathGroup>();
        auto n = this->size();
        new_group->reserve(n);
        unassigned.reserve(n-1);
        // find path closest to reference point
        double min_dist = std::numeric_limits<double>::max(), cur_dist;
        std::shared_ptr<Path> p0 = this->paths[0];
        for (auto p: this->paths) {
            cur_dist = (*p)[0]->distance_to(ref_point);
            if (cur_dist < min_dist) {
                min_dist = cur_dist;
                p0 = p;
                if (min_dist==0) break;
            }
        }
        // pick start and end points
        for (auto p: this->paths) {
            if (p0 != p) {
                unassigned.emplace_back(p);
            } else {
                new_group->emplace_back(p0->copy());
            }
        }
        // sort paths by distance to each other
        std::vector<std::shared_ptr<Path>>::iterator pmin = unassigned.end();
        for (auto i=1; i<n; i++) {
            min_dist = std::numeric_limits<double>::max();
            for (auto it = unassigned.begin(); it!=unassigned.end(); ++it) {
                // take distance between start and start points, end and start
                // points, or end and end points, depending on settings
                if (predicate == SortPredicate::StartToStart) {
                    cur_dist = (*p0)[0]->distance_to((**it)[0]);
                } else if (predicate == SortPredicate::EndToStart) {
                    cur_dist = (*p0)[p0->size()-1]->distance_to((**it)[0]);
                } else if (predicate == SortPredicate::EndToEnd) {
                    cur_dist = (*p0)[p0->size()-1]->distance_to((**it)[(*it)->size()-1]);
                }
                if (cur_dist<=min_dist) {
                    min_dist = cur_dist;
                    pmin = it;
                    if (min_dist==0) break;
                }

            }
            if (pmin != unassigned.end()) {
                p0 = *pmin;
                new_group->emplace_back(p0->copy());
                unassigned.erase(pmin);
            }
        }
        // adjust paths angles in such a way that the starting angles are as close as possible from each other
        double dangle;
        for (auto i=1; i<n; i++) {
            dangle = (*(*new_group)[i-1])[0]->c - (*(*new_group)[i])[0]->c + angle_norm((*new_group->paths[i])[0]->c - (*new_group->paths[i-1])[0]->c);
            for (auto pt: *(*new_group)[i])
                pt->c += dangle;
        }
        return new_group;
    }
    
    std::shared_ptr<PathGroup> PathGroup::rearrange(const double limit_height) const {
        PYG_LOG_V("Rearranging pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        auto ref_point = (*this->paths[0])[0];
        for (auto p: this->paths) {
            new_group->emplace_back(p->rearrange(limit_height, ref_point));
            ref_point = (*(*new_group)[new_group->size()-1])[0];
        }
        return new_group;
    }
    
    std::shared_ptr<PathGroup> PathGroup::reorder(const std::vector<unsigned int> & order) const {
        PYG_LOG_V("Reordering pathgroup 0x{:x}", (uint64_t)this);
        auto new_group = std::make_shared<PathGroup>();
        new_group->reserve(this->size());
        auto n = this->size();
        for (auto i: order) {
            if (i>=n)
                throw std::invalid_argument("Indices cannot exceed <group size - 1>.");
            new_group->emplace_back(this->paths[i]);
        }
        
        return new_group;
    }
    
    const std::vector<std::shared_ptr<Path>> & PathGroup::get_paths() const {
        return this->paths;
    }

    void PathGroup::set_paths(const std::vector<std::shared_ptr<Path>> & paths) {
        this->paths = paths;
    }

    std::shared_ptr<Path> PathGroup::py_get_item(int idx) const {
        if (idx<0)
            idx = this->paths.size() + idx;
    
        if (idx<0 || idx>=this->paths.size()) {
            PyErr_SetString(PyExc_ValueError, "Index out of bounds.");
            throw py::error_already_set();
        }
        return this->paths[idx];
    }

    std::vector<std::shared_ptr<Path>> PathGroup::py_get_item_slice(const py::slice & slice) const {
        ssize_t start, stop, step;
        PySlice_Unpack(slice.ptr(), &start, &stop, &step);
        std::vector<std::shared_ptr<Path>> res;
        res.reserve((stop-start+step-1)/step);
        for (auto it = this->paths.begin()+start; it<this->paths.begin()+stop; std::advance(it,step))
            res.emplace_back(*it);
        
        return res;
    }

    void PathGroup::py_set_item(int idx, std::shared_ptr<Path> path) {
        this->paths[idx] = path;
    }

    void PathGroup::py_set_item_slice(const py::slice & slice, const std::vector<std::shared_ptr<Path>> & ps) {
        auto [start, stop, step] = convert_slice(slice, this->size());
        if ((stop-start+step-1)/step != ps.size()) {
            PyErr_SetString(PyExc_ValueError, "Values and slice are of different sizes.");
            throw py::error_already_set();
        }
        auto p_it = ps.begin();
        for (auto it = this->paths.begin()+start; it<this->paths.begin()+stop; std::advance(it,step))
            *it = *p_it++;
    }

    void py_pathgroup_exports(py::module_ & mod) {
        py::enum_<SortPredicate>(mod, "SortPredicate")
        .value("StartToStart", SortPredicate::StartToStart)
        .value("EndToStart", SortPredicate::EndToStart)
        .value("EndToEnd", SortPredicate::EndToEnd);

        py::class_<PathGroup, std::shared_ptr<PathGroup>>(mod, "PathGroup")
        .def(py::init<>())
        .def(py::init<const std::vector<std::shared_ptr<Path>>&>())
        .def("__add__", &add_objects<PathGroup>, py::is_operator())
        .def("__mul__", &mul_object<PathGroup, unsigned int>, py::is_operator())
        .def("__rmul__", &mul_object<PathGroup, unsigned int>, py::is_operator())
        .def("__len__", &PathGroup::size)
        .def("__iter__", [](std::shared_ptr<PathGroup> p){return py::make_iterator(p->begin(), p->end());}
                       , py::keep_alive<0, 1>())
        .def("append", &PathGroup::push_back, py::arg("path"))
        .def("cylindrical", &PathGroup::to_cylindrical, py::arg("radius"))
        .def("shift", &PathGroup::shift, py::arg("vector"))
        .def("scale", &PathGroup::scale, py::arg("factor"), py::arg("center"))
        .def("scale_to_size", &PathGroup::scale_to_size, py::arg("radius"), py::arg("center"))
        .def("mirror", &PathGroup::mirror, py::arg("along_x"), py::arg("along_y"), py::arg("along_z"))
        .def("rotate", &PathGroup::rotate, py::arg("yaw_angle"), py::arg("pitch_angle"), py::arg("roll_angle"), py::arg("radians")=false)
        .def("matrix_transform", &PathGroup::matrix_transform, py::arg("components"))
        .def("inflate", &PathGroup::inflate, py::arg("amount"))
        .def("buffer", &PathGroup::buffer, py::arg("amount"), py::arg("cap_style")=gob::BufferParameters::CAP_ROUND, py::arg("join_style")=gob::BufferParameters::JOIN_ROUND, py::arg("mitre_limit")=1.0)
        .def("simplify", &PathGroup::simplify, py::arg("tolerance"))
        .def("interpolate", &PathGroup::interpolate, py::arg("step_size"))
        .def("flip", &PathGroup::flip)
        .def("simplify_above", &PathGroup::simplify_above, py::arg("limit_height"))
        .def("split_above", &PathGroup::split_above, py::arg("limit_height"))
        .def("create_ramps", &PathGroup::create_ramps, py::arg("limit_height"), py::arg("ramp_height"), py::arg("ramp_length"), py::arg("ramp_direction"))
        .def("create_backward_ramps", &PathGroup::create_backward_ramps, py::arg("limit_height"), py::arg("ramp_height"), py::arg("ramp_length"))
        .def("create_forward_ramps", &PathGroup::create_forward_ramps, py::arg("limit_height"), py::arg("ramp_height"), py::arg("ramp_length"))
        .def("sort_paths", &PathGroup::sort_paths, py::arg("ref_point"), py::arg("predicate")=SortPredicate::EndToStart)
        .def("rearrange", &PathGroup::rearrange, py::arg("limit_height"))
        .def("reorder", &PathGroup::reorder, py::arg("order"))
        .def("__getitem__", &PathGroup::py_get_item)
        .def("__getitem__", &PathGroup::py_get_item_slice)
        .def("__setitem__", &PathGroup::py_set_item)
        .def("__setitem__", &PathGroup::py_set_item_slice)
        .def_property("paths", &PathGroup::get_paths, &PathGroup::set_paths)
        .def_property_readonly("cartesian", &PathGroup::to_cartesian)
        .def_property_readonly("polar", &PathGroup::to_polar)
        .def_property_readonly("radius", &PathGroup::get_radius)
        .def_property_readonly("centroid", &PathGroup::get_centroid)
        .def_property_readonly("envelope", &PathGroup::get_envelope)
        .def_property("steps", &PathGroup::get_steps, &PathGroup::set_steps)
        ;
    }

}