/** \file vtkevents.h
 *  \brief Header file for VTK events.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#pragma once
#include <vtkCommand.h>
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkAnimationCue.h>

namespace pygraver::render {

    class Model;

    /** \brief Base class for VTK callbacks with associated Model object. */
    class vtkModelCallback : public vtkCommand {
    protected:
        /** \brief Pointer to a model that may be affected by render events. */
        Model * model;

    public:
        /** \brief Associate model with callback.
         *  \param model: pointer to the model to associate.
         */
        void set_model(Model * model) {
            this->model = model;
        }
    };

    /** \brief Callback following for render events.
     * 
     *  This is used to adapt overlay if display size has changed.
    */
    class vtkRefreshCallback : public vtkModelCallback {
    public:
        /** \brief Create a new instance. This is necessary for VTK. */
        static vtkRefreshCallback* New() {
            return new vtkRefreshCallback;
        }

        /** \brief Callback function.
         *  \param caller: pointer to the object calling the function.
         */
        void Execute(vtkObject* caller, unsigned long, void*) override;

    };


    /** \brief Callback for timer events. */
    class vtkTimerCallback : public vtkModelCallback {
    public:
        /** \brief Create a new instance. This is necessary for VTK. */
        static vtkTimerCallback* New() {
            return new vtkTimerCallback;
        }

        /** \brief Callback function. */
        void Execute(vtkObject*, unsigned long, void*) override;

    };


    /** \brief Class producing a fading animation on a widget. */
    class vtkWidgetFader {
    private:
        /** \brief Pointer to the widget upon which to act. */
        vtkAbstractWidget* widget;

        /** \brief Fading animation direction. If true, fading goes from opaque to transparent;
         *  if false, fading goes from transparent to opaque.
         */
        bool forward = true;

        /** \brief Indicate if animation is running (true = running, false = idle). */
        bool running = false;

        /** \brief Start animation. */
        void Start();

        /** \brief Animation ticker.
         *  \param calldata: data associated with animation.
         */
        void Tick(vtkObject*, unsigned long, void* calldata);

        /** \brief Stop animation. */
        void End();

    public:
        /** \brief Associate widget.
         *  \param widget: pointer to the widget to associate.
        */
        void set_widget(vtkAbstractWidget* widget) { this->widget = widget; }

        /** \brief Add observers to given animation cue object.
         *  \param cue: pointer to animation cue object. 
        */
        void AddObserversToCue(vtkAnimationCue* cue) {
            cue->AddObserver(vtkCommand::StartAnimationCueEvent, this, &vtkWidgetFader::Start);
            cue->AddObserver(vtkCommand::EndAnimationCueEvent, this, &vtkWidgetFader::End);
            cue->AddObserver(vtkCommand::AnimationCueTickEvent, this, &vtkWidgetFader::Tick);
        }

        /** \brief Tell if animation is running.
         *  \returns true if animation is running, false otherwise.
        */
        bool is_running() const {
            return this->running;
        }

    };


    /** \brief Custom interactor style class. */
    class vtkCustomInteractorStyle : public vtkInteractorStyleTrackballCamera {
     private:
        /** \brief Pointer to associated model. */
        Model * model;

        /** \brief Storage for ongoing widget animations. */
        std::vector<std::unique_ptr<vtkWidgetFader>> anims;

    public:
        /** \brief Create a new instance. This is necessary for VTK. */
        static vtkCustomInteractorStyle* New() {
            return new vtkCustomInteractorStyle;
        }

        /** \brief Callback when a key gets pressed. */
        void OnKeyPress() override;

        /** \brief Callback when left mouse button is released. */
        void OnLeftButtonUp() override;
 
        /** \brief Associate model with callback.
         *  \param model: pointer to the model to associate.
         */
       void set_model(Model * model) {
            this->model = model;
        }
    };

}