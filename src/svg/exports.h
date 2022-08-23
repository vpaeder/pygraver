/** \file exports.h
 *  \brief Header file for Python exports for SVG submodule.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <pybind11/pybind11.h>
namespace py = pybind11;

namespace pygraver::svg {

    /** \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_svg_exports(py::module_ & mod);

}
