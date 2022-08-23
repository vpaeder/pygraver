/** \file cylinder.h
 *  \brief Header file for Cylinder class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "model.h"
#include "wire.h"

namespace pygraver::render {

    using namespace pygraver::types;

    /** \brief A shape representing a cylinder in 3D. */
    class Cylinder : public Shape3D {
    protected:
        /** \brief Position to color mapping function.
         *  \param pos: position.
         *  \returns index along color scale.
         */
        double color_mapping_function(const double pos[3]) override;
    
    public:
        /** \brief Constructor with arguments.
         *  \param radius: cylinder radius.
         *  \param height: cylinder height.
         *  \param center: shape center.
         *  \param axis: alignment axis.
         *  \param color: RGB or RGBA color.
         */
        Cylinder(const double radius,
                 const double height,
                 std::shared_ptr<Point> center,
                 std::shared_ptr<Point> axis,
                 const std::vector<uint8_t> & color);
        
        ~Cylinder();

    };


    /** \fn void py_cylinder_exports(py::module_ & mod)
     *  \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_cylinder_exports(py::module_ & mod);

}
