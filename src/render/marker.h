/** \file marker.h
 *  \brief Header file for Marker classes.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include "../types/point.h"
#include "extrusion.h"

namespace pygraver::render {

    using namespace pygraver::types;

    /** \brief A shape representing a marker in 3D. */
    class Marker : public Extrusion {
    public:
        /** \brief Constructor with arguments.
         *  \param glyph: ASCII code for glyph used as marker.
         *  \param position: marker position.
         *  \param size_x: size in x direction.
         *  \param size_y: size in y direction.
         *  \param thickness: size in z direction.
         *  \param orientation: extrusion direction.
         *  \param color: marker color (RGB or RGBA).
        */
        Marker(const char glyph,
               std::shared_ptr<const Point> position,
               const double size_x,
               const double size_y,
               const double thickness,
               std::shared_ptr<const Point> orientation,
               const std::vector<uint8_t> & color);
        
        ~Marker();

        /** \brief Get actors that can be interactive (clickable, hoverable, ...).
         *  \returns vector of actors and associated messages.
        */
        virtual std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>> get_interactive() override;
    };


    /** \brief A shape representing a collection of markers in 3D. */
    class MarkerCollection : public Extrusion {
    public:
        /** \brief Constructor with arguments.
         *  \param glyph: ASCII code for glyph used as marker.
         *  \param positions: list of positions.
         *  \param size_x: size in x direction.
         *  \param size_y: size in y direction.
         *  \param thickness: size in z direction.
         *  \param orientation: extrusion direction.
         *  \param color: marker color (RGB or RGBA).
        */
        MarkerCollection(const char glyph,
                         const std::vector<std::shared_ptr<Point>> & positions,
                         const double size_x,
                         const double size_y,
                         const double thickness,
                         std::shared_ptr<const Point> orientation,
                         const std::vector<uint8_t> & color);
        
        ~MarkerCollection();

        /** \brief Get actors that can be interactive (clickable, hoverable, ...).
         *  \returns vector of actors and associated messages.
        */
         virtual std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>> get_interactive() override;
   };

    /** \fn void py_marker_exports(py::module_ & mod)
     *  \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_marker_exports(py::module_ & mod);

}