/** \file shape3d.cpp
 *  \brief Header file for Shape3D base class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vtkActorCollection.h>
#include <vtkActor.h>
#include <vtkLookupTable.h>

#include <vtkRenderer.h>
#include <vtkViewport.h>
#include <vtkTexture.h>
#include <vtkProperty.h>
#include <vtkPolyData.h>

#include <pybind11/pybind11.h>

#include "../types/point.h"

namespace py = pybind11;
using namespace pygraver::types;

namespace pygraver::render {

    /** \brief Base class for 3D shapes. */
    class Shape3D {
    protected:
        /** \brief Storage for shape actors.
         * 
         *  This is the actors associated with the shape items. There must be
         *  one actor for each item in shape data.
        */
        vtkSmartPointer<vtkActorCollection> actors;

        /** \brief Lookup table for color mapping in scalar color mode. */
        vtkSmartPointer<vtkLookupTable> lut;

        /** \brief Information key to access highlight state. */
        vtkSmartPointer<vtkInformationIntegerKey> highlight_key;

        /** \brief Shape label. */
        std::string label;

        /** \brief Color for default state. */
        std::vector<uint8_t> base_color = {255,255,255,255};

        /** \brief Color for highlighted state. */
        std::vector<uint8_t> highlight_color = {0,0,0,255};

        /** \brief Position to color mapping function.
         *  \param pos: position.
         *  \returns index along color scale.
         */
        virtual double color_mapping_function(const double pos[3]) {return 0;}
    
        /** \brief Compute highlight color from given color.
         * 
         *  This is used to produce a color with good contrast.
         * 
         *  \param color: RGBA color.
        */
        static std::vector<double> make_highlight_color(const double color[4]);

        /** \brief Compute highlight color from given color.
         *  \param color: RGBA color.
        */
        static std::vector<uint8_t> make_highlight_color(const std::vector<uint8_t> & color);

        /** \brief Inverse scalar colors for given data object.
         * 
         *  This is used to highlight an object when in scalar color mode.
         * 
         *  \param data: pointer to a polydata object.
        */
        static void flip_colors(vtkSmartPointer<vtkPolyData> data);

    public:

        /** \brief Default constructor. */
        Shape3D();

        /** \brief Default destructor. */
        ~Shape3D();

        /** \brief Set item at given index.
         *  \param idx: item index.
         *  \param polydata: item content.
         */
        void set_item(const size_t idx, vtkSmartPointer<vtkPolyData> polydata);

        /** \brief Getter for shape actors.
         *  \returns a pointer to the shape actors collection.
         */
        vtkSmartPointer<vtkActorCollection> get_actors();

        /** \brief Set default color.
         *  \param color: 3 or 4-valued color (RGB or RGBA).
         */
        void set_base_color(const std::vector<uint8_t> & color);

        /** \brief Set color.
         *  \param idx: index of actor.
         *  \param color: 3 or 4-valued color (RGB or RGBA).
         */
        void set_color(const size_t idx, const std::vector<uint8_t> & color);

        /** \brief Get default color.
         *  \returns default color.
         */
        std::vector<uint8_t> get_base_color() const;

        /** \brief Set highlighted color.
         *  \param color: 3 or 4-valued color (RGB or RGBA).
         */
        void set_highlight_color(const std::vector<uint8_t> & color);

        /** \brief Get highlighted color.
         *  \returns highlighted color.
         */
        std::vector<uint8_t> get_highlight_color() const;

        /** \brief Set value range for scalar color mapping.
         *  \param vmin: minimum value.
         *  \param vmax: maximum value.
         */
        void set_scalar_color_range(const double vmin, const double vmax);

        /** \brief Get value range for scalar color mapping.
         *  \returns double[2] array with min. and max. range boundaries.
         */
        double * get_scalar_color_range() const;

        /** \brief Python wrapper for get_scalar_color_range.
         *  \returns Python list object containing min. and max. range boundaries.
         */
        py::list py_get_scalar_color_range() const;

        /** \brief Set shape label.
         *  \param label: label.
         */
        void set_label(const std::string & label);

        /** \brief Get shape label.
         *  \returns shape label.
         */
        std::string get_label() const;

        /** \brief Set color mode to scalar.
         * 
         *  This will color shapes according to color mapping function.
         * 
         *  \param en: if true, enable scalar mode; if false, disable mode.
         */
        void set_scalar_color_mode(const bool en);

        /** \brief Toggle scalar color mode. */
        void toggle_scalar_color_mode();

        /** \brief Get state of scalar color mode.
         *  \returns true if scalar color mode is enabled, false otherwise.
         */
        bool get_scalar_color_mode() const;

        /** \brief Set visibility.
         *  \param en: if true, shows shape; if false, hides shape.
         */
        void set_visible(const bool en);

        /** \brief Toggle visibility. */
        void toggle_visibility();

        /** \brief Get visibility.
         *  \returns true if shape is visible, false otherwise.
         */
        bool get_visibility() const;

        /** \brief Define highlight state for given actor.
         *  \param actor: pointer to an actor amongst shape's actors.
         *  \param en: if true, highlight actor; if false, set actor in default state.
         */
        void set_highlighted(vtkSmartPointer<vtkActor> actor, const bool en);

        /** \brief Define highlight state for given actor's index.
         *  \param idx: index of actor.
         *  \param en: if true, highlight actor; if false, set actor in default state.
         */
        void set_highlighted(const unsigned int idx, bool en);

        /** \brief Define highlight state for entire shape.
         *  \param en: if true, highlight shape; if false, set shape in default state.
         */
        void set_highlighted(bool en);

        /** \brief Toggle highlight state for given actor.
         *  \param actor: pointer to an actor amongst shape's actors.
         */
        void toggle_highlighted(vtkSmartPointer<vtkActor> actor);

        /** \brief Toggle highlight state for given actor's index.
         *  \param idx: index of actor.
         */
        void toggle_highlighted(const unsigned int idx);

        /** \brief Toggle highlight state for entire shape. */
        void toggle_highlighted();

        /** \brief Get highlight state for given actor.
         *  \param actor: pointer to an actor amongst shape's actors.
         *  \returns true if actor is highlighted, false otherwise.
        */
        bool get_highlighted(vtkSmartPointer<vtkActor> actor) const;

        /** \brief Get highlight state for given actor's index.
         *  \param idx: index of actor.
         *  \returns true if actor is highlighted, false otherwise.
        */
        bool get_highlighted(const unsigned int idx) const;

        /** \brief Get shape actors.
         *  \returns pointer to shape actors collection.
        */
        vtkSmartPointer<vtkActorCollection> get_actors() const { return this->actors; }
        
        /** \brief Python wrapper for get_actors.
         *  \returns vector of pointers to shape actors.
        */
        std::vector<vtkSmartPointer<vtkActor>> py_get_actors() const;

        /** \brief Get actors that can be interactive (clickable, hoverable, ...).
         *  \returns vector of actors and associated messages.
        */
        virtual std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>> get_interactive();

        /** \brief Tell if given point is inside shape.
         *  \param point: point coordinates.
         *  \returns true if point is inside, false otherwise.
        */
        bool is_point_inside(const double point[3]) const;

        /** \brief Tell if given point is inside shape.
         *  \param point: point coordinates.
         *  \returns true if point is inside, false otherwise.
        */
        bool is_point_inside(const std::vector<double> & point) const;

        /** \brief Tell if given point is inside shape.
         *  \param point: pointer to Point object.
         *  \returns true if point is inside, false otherwise.
        */
        bool is_point_inside(std::shared_ptr<const Point> point) const;

        /** \brief Compute distance to given actor.
         *  \param actor: pointer to an actor amongst shape's actors.
         *  \param point: point coordinates.
         *  \returns distance to actor.
        */
        static double distance_to_actor(vtkSmartPointer<vtkActor> actor, const double point[3]);
        
        /** \brief Compute distance to given actor.
         *  \param actor: pointer to an actor amongst shape's actors.
         *  \param point: point coordinates.
         *  \returns distance to actor.
        */
        static double distance_to_actor(vtkSmartPointer<vtkActor> actor, const std::vector<double> & point);

        /** \brief Compute distance to given actor.
         *  \param actor: pointer to an actor amongst shape's actors.
         *  \param point: pointer to Point object.
         *  \returns distance to actor.
        */
        static double distance_to_actor(vtkSmartPointer<vtkActor> actor, std::shared_ptr<const Point> point);

        /** \brief Find closest actor to given point.
         *  \param point: point coordinates.
         *  \returns a tuple containing distance to actor and actor pointer.
        */
        std::tuple<double, vtkSmartPointer<vtkActor>> closest_actor(const double point[3]) const;

        /** \brief Find closest actor to given point.
         *  \param point: point coordinates.
         *  \returns a tuple containing distance to actor and actor pointer.
        */
        std::tuple<double, vtkSmartPointer<vtkActor>> closest_actor(const std::vector<double> & point) const;

        /** \brief Find closest actor to given point.
         *  \param point: pointer to Point object.
         *  \returns a tuple containing distance to actor and actor pointer.
        */
        std::tuple<double, vtkSmartPointer<vtkActor>> closest_actor(std::shared_ptr<const Point> point) const;

        /** \brief Find first actor intersecting with given line.
         *  \param point1: coordinates of line start.
         *  \param point2: coordinates of line end.
         *  \returns pointer to found actor, or nullptr if none could be found.
        */
        vtkSmartPointer<vtkActor> intersecting_actor(const double point1[3], const double point2[3]) const;

        /** \brief Find first actor intersecting with given line.
         *  \param point1: coordinates of line start.
         *  \param point2: coordinates of line end.
         *  \returns pointer to found actor, or nullptr if none could be found.
        */
        vtkSmartPointer<vtkActor> intersecting_actor(const std::vector<double> & point1, const std::vector<double> & point2) const;

        /** \brief Find first actor intersecting with given line.
         *  \param point1: pointer to line start point.
         *  \param point2: pointer to line end point.
         *  \returns pointer to found actor, or nullptr if none could be found.
        */
        vtkSmartPointer<vtkActor> intersecting_actor(std::shared_ptr<const Point> point1, std::shared_ptr<const Point> point2) const;

    };


    /** \brief Crossover class to permit sub-classing the Shape3D class in Python. */
    class PyShape3D : public Shape3D {
    public:
        using Shape3D::Shape3D;

         /** \brief Position to color mapping function.
         *  \param pos: position.
         *  \returns index along color scale.
         */
       double color_mapping_function(const double pos[3]) override {
            PYBIND11_OVERRIDE(double, Shape3D, color_mapping_function, pos);
        }
 
         /** \brief Get actors that can be interactive (clickable, hoverable, ...).
         *  \returns vector of actors and associated messages.
        */
        std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>> get_interactive() override {
            using ret_type = std::vector<std::tuple<vtkSmartPointer<vtkActor>, std::string>>;
            PYBIND11_OVERRIDE(ret_type, Shape3D, get_interactive);
        }
   };

    /** \fn void py_shape3d_exports(py::module_ & mod)
     *  \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_shape3d_exports(py::module_ & mod);

}