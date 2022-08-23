/** \file exports.cpp
 *  \brief Definition of Python exports for PyGraver module.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <pybind11/pybind11.h>

#include "types/common.h"
#include "types/point.h"
#include "types/path.h"
#include "types/surface.h"
#include "types/pathgroup.h"
#include "svg/exports.h"
#include "render/exports.h"

PYBIND11_MODULE(core, m) {
    using namespace pygraver;
    using namespace pygraver::types;
    using namespace pygraver::svg;
    using namespace pygraver::render;
    namespace py = pybind11;

    m.doc() = "PyGraver";

    auto m_types = m.def_submodule("types", "Data types");

    types::py_point_exports(m_types);
    types::py_path_exports(m_types);
    types::py_surface_exports(m_types);
    types::py_pathgroup_exports(m_types);

    auto m_svg = m.def_submodule("svg", "SVG parsing routines");
    py_svg_exports(m_svg);

    
    auto m_render = m.def_submodule("render", "Rendering aids");
    py_render_exports(m_render);
    
}
