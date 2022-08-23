/** \file model.h
 *  \brief Header file for Model class.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vector>
#include <unordered_map>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkAbstractWidget.h>

#include "../types/point.h"
#include "../types/path.h"
#include "../types/pathgroup.h"
#include "../types/surface.h"
#include "shape3d.h"
#include "vtkpybind.h"
#include "../log.h"

namespace pygraver::render {
    
    using namespace pygraver::types;

    /** \brief Check that given color is valid.
     *  \param color: RGBA color.
     *  \throws invalid_argument if color is invalid.
    */
    static void check_color(const std::vector<uint8_t> & color) {
        if (color.size()!=4) {
            PYG_LOG_E("Color vector size: {}", color.size());
            throw std::invalid_argument("Color must be a 4-component vector.");
        }
    }

    /** \brief Base class for 3D model. */
    class Model {
    protected:
        /** \brief Pointer to the associated renderer. */
        vtkSmartPointer<vtkRenderer> renderer;

        /** \brief Pointer to the associated window. */
        vtkSmartPointer<vtkRenderWindow> window;

        /** \brief List of pointers to registered widgets. */
        std::vector<vtkSmartPointer<vtkAbstractWidget>> widgets;

        /** \brief List of pointers to registered shapes. */
        std::vector<std::shared_ptr<Shape3D>> shapes;

        /** \brief Background color. */
        std::vector<uint8_t> bg_color = {255, 255, 255};

        /** \brief Create window object. */
        void create_window();

    public:
        /** \brief Constructor. */
        Model();

        ~Model();

        /** \brief Get model's renderer.
         * 
         *  If none is available, tries to create a window and a renderer.
         * 
         *  \returns pointer to model's renderer object.
        */
        const vtkSmartPointer<vtkRenderer> get_renderer() {
            if (this->renderer == nullptr)
                this->create_window();
            return this->renderer;
        }

        /** \brief Get render window.
         * 
         *  If none is available, tries to create one.
         * 
         *  \returns pointer to render window object.
        */
        const vtkSmartPointer<vtkRenderWindow> get_render_window() {
            if (this->window == nullptr)
                this->create_window();
            return this->window;
        }

        /** \brief Get registered widgets.
         *  \returns list of pointers to registered widgets.
        */
        std::vector<vtkSmartPointer<vtkAbstractWidget>> & get_widgets() {
            return this->widgets;
        }

        /** \brief Get registered shapes.
         *  \returns list of pointers to registered shapes.
        */
        std::vector<std::shared_ptr<Shape3D>> & get_shapes() {
            return this->shapes;
        }

        /** \brief Add shape.
         *  \param shape: pointer to 3D shape object.
        */
        void add_shape(std::shared_ptr<Shape3D> shape);

        /** \brief Remove shape.
         *  \param shape: pointer to 3D shape object.
        */
        void remove_shape(std::shared_ptr<Shape3D> shape);

        /** \brief Test if given shape is registered.
         *  \param shape: pointer to 3D shape object.
         *  \returns true if shape is registered, false if not.
        */
        bool has_shape(std::shared_ptr<Shape3D> shape);

        /** \brief Add widget.
         *  \param widget: pointer to widget object.
        */
        void add_widget(vtkSmartPointer<vtkAbstractWidget> widget);

        /** \brief Remove widget.
         *  \param widget: pointer to widget object.
        */
        void remove_widget(vtkSmartPointer<vtkAbstractWidget> widget);

        /** \brief Test if given widget is registered.
         *  \param widget: pointer to widget object.
         *  \returns true if widget is registered, false if not.
        */
        bool has_widget(vtkSmartPointer<vtkAbstractWidget> widget);

        /** \brief Set background color.
         *  \param color: 3-valued vector of color components (RGB).
        */
        void set_background_color(const std::vector<uint8_t> & color);

        /** \brief Get background color.
         *  \returns background color components.
        */
        std::vector<uint8_t> & get_background_color() { return this->bg_color; }

        /** \brief Render model. */
        void render();

        /** \brief Timer callback function.
         * 
         *  This is used in conjunction with Python to force handing back priority
         *  to Python sub-threads; this requires sub-classing the Model class
         *  and define the timer_callback in Python.
        */
        virtual void timer_callback() {}


    };


    /** \brief Crossover class to permit sub-classing the Model class in Python. */
    class PyModel : public Model {
    public:
        using Model::Model;

        /** \brief Timer callback function. */
        void timer_callback() override {
            PYBIND11_OVERRIDE(void, Model, timer_callback);
        }
    };


    /** \fn void py_model_exports(py::module_ & mod)
     *  \brief Export function for Python wrapper.
     *  \param mod: module or submodule to add content to.
     */
    void py_model_exports(py::module_ & mod);

}
