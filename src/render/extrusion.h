/** \file extrusion.h
 *  \brief Header file for Extrusion class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vector>

#include "../types/surface.h"
#include "../types/path.h"
#include "../types/point.h"
#include "shape3d.h"
#include "../log.h"

namespace pygraver::render {

    using namespace pygraver::types;

    /** \brief Create a vtkPolyData object from a Surface object.
     *  \param surf: pointer to the surface object to convert.
     *  \returns pointer to a vtkPolyData object.
     */
    vtkSmartPointer<vtkPolyData> make_polydata(std::shared_ptr<Surface> surf);

    /** \brief Create a vtkPolyData object from a Path object.
     *  \param path: pointer to the path object to convert.
     *  \returns pointer to a vtkPolyData object.
     */
    vtkSmartPointer<vtkPolyData> make_polydata(std::shared_ptr<Path> path);

    /** \brief Extrude a vtkPolyData object with given parameters.
     *  \param data: pointer to the vtkPolyData object to extrude.
     *  \param axis: extrusion axis.
     *  \param centre: extrusion centre.
     *  \param length: extrusion length.
     *  \returns pointer to a vtkPolyData object.
     */
    vtkSmartPointer<vtkPolyData> extrude(vtkSmartPointer<vtkPolyData> data,
                                         std::shared_ptr<Point> axis,
                                         std::shared_ptr<Point> centre,
                                         const double length);

    /** \brief A shape representing the extrusion of an arbitrary contour. */
    class Extrusion : public Shape3D {
    protected:
        /** \brief Extrusion axis. */
        double axis[3];

        /** \brief Position to color mapping function.
         *  \param pos: position.
         *  \returns index along color scale.
         */
        virtual double color_mapping_function(const double pos[3]) override;
        
    public:
        /** \brief Default constructor. */
        Extrusion();

        /** \brief Default destructor. */
        ~Extrusion();
        
        /** \brief Constuctor with surface as contour.
         *  \param contour: pointer to a surface object.
         *  \param length: extrusion length.
         *  \param axis: extrusion axis.
         *  \param color: RGB or RGBA color.
         */
        Extrusion(std::shared_ptr<Surface> contour,
                  const double length,
                  std::shared_ptr<Point> axis,
                  const std::vector<uint8_t> & color);
        
        /** \brief Constuctor with path as contour.
         *  \param contour: pointer to a path object.
         *  \param length: extrusion length.
         *  \param axis: extrusion axis.
         *  \param color: RGB or RGBA color.
         */
        Extrusion(std::shared_ptr<Path> contour,
                  const double length,
                  std::shared_ptr<Point> axis,
                  const std::vector<uint8_t> & color);

        /** \brief Set extruded contour.
         *  \param contour: pointer to a surface object.
         *  \param length: extrusion length.
         *  \param axis: extrusion axis.
         *  \param color: RGB or RGBA color.
         */
        void set_shape(std::shared_ptr<Surface> contour,
                       const double length,
                       std::shared_ptr<Point> axis,
                       const std::vector<uint8_t> & color);
        
        /** \brief Set extruded contour.
         *  \param contour: pointer to a path object.
         *  \param length: extrusion length.
         *  \param axis: extrusion axis.
         *  \param color: RGB or RGBA color.
         */
        void set_shape(std::shared_ptr<Path> contour,
                       const double length,
                       std::shared_ptr<Point> axis,
                       const std::vector<uint8_t> & color);

        /** \brief Get actors that can be interactive (clickable, hoverable, ...).
         *  \returns vector of actors and associated messages.
        */
        virtual std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>> get_interactive() override;
    };


    /** \fn void py_extrusion_exports(py::module_ & mod)
     *  \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_extrusion_exports(py::module_ & mod);
}