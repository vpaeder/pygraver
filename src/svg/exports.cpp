/** \file exports.cpp
 *  \brief Implementation file for Python exports for SVG submodule.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "file.h"
#include "arc.h"
#include "exports.h"

namespace pygraver::svg {
    void py_svg_exports(py::module_ & mod) {
        py::class_<File>(mod, "File")
        .def(py::init<>())
        .def(py::init<const std::string&>())
        .def("open", &File::open, py::arg("file_name"))
        .def("from_memory", &File::from_memory, py::arg("buffer"))
        .def("get_size", &File::get_size, py::return_value_policy::take_ownership)
        .def("get_paths", &File::get_paths, py::arg("layer"), py::arg("step_size"), py::return_value_policy::take_ownership)
        .def("get_points", &File::get_points, py::arg("layer"), py::return_value_policy::take_ownership);
    
        mod.def("elliptic_e", static_cast<double(*)(const double, const double, const double)>(&elliptic_e<double>));
        mod.def("inv_elliptic_e", &inv_elliptic_e<double>);
        mod.def("carlson_rf", &carlson_rf<double>);
        mod.def("carlson_rd", &carlson_rd<double>);
        
    }
}

