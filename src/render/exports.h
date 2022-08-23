/** \file exports.h
 *  \brief Header file for Python exports for rendering submodule.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <pybind11/pybind11.h>

namespace py = pybind11;

/** \namespace pygraver::render
 *  \brief Contains rendering routines.
 */
namespace pygraver::render {

    /** \fn void py_render_exports(py::module_ & mod)
     *  \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_render_exports(py::module_ & mod);

}
