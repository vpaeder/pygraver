/** \file exports.cpp
 *  \brief Implementation file for Python exports for rendering submodule.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <pybind11/pybind11.h>

#include "cylinder.h"
#include "shape3d.h"
#include "extrusion.h"
#include "marker.h"
#include "wire.h"
#include "model.h"
#include "exports.h"

namespace pygraver::render {
    void py_render_exports(py::module_ & mod) {
        py_shape3d_exports(mod);
        py_extrusion_exports(mod);
        py_cylinder_exports(mod);
        py_marker_exports(mod);
        py_wire_exports(mod);

        py_model_exports(mod);
    }
}
