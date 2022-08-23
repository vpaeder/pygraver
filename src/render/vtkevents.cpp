/** \file vtkevents.cpp
 *  \brief Implementation file for VTK events.
 *
 *  Author: Vincent Paeder
 *  License: MIT
 */
#include <vtkTextWidget.h>
#include <vtkTextRepresentation.h>
#include <vtkTextProperty.h>
#include <vtkTextActor.h>

#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkAnimationScene.h>
#include <vtkProperty2D.h>

#include "vtkevents.h"
#include "model.h"

namespace pygraver::render {

    void vtkRefreshCallback::Execute(vtkObject* caller, unsigned long, void*) {
        auto window = reinterpret_cast<vtkRenderWindow*>(caller);
        auto sz = window->GetSize();
        auto widgets = this->model->get_widgets();
        double h = 1 - 50.0/double(sz[1]);
        for (auto & widget: widgets) {
            auto repr = dynamic_cast<vtkTextRepresentation*>(widget->GetRepresentation());
            if (repr == nullptr) continue;
            double wsz[2];
            repr->GetTextActor()->GetSize(this->model->get_renderer(), wsz);
            auto cursz = repr->GetPosition2Coordinate()->GetValue();
            h -= cursz[1]+10.0/double(sz[1]);
            // had to add 4 pixels for a reason that I don't really understand
            double w = double(wsz[0]+repr->GetPaddingRight()+repr->GetPaddingLeft()+4)/sz[0];
            repr->SetPosition(1-w, h); // move to the right border
            repr->SetPosition2(w, cursz[1]);
        }
    }


    void vtkTimerCallback::Execute(vtkObject*, unsigned long, void*) {
        this->model->timer_callback();
    }


    void vtkCustomInteractorStyle::OnKeyPress() {
        // Get the keypress
        auto rwi = this->Interactor;
        std::string key = rwi->GetKeySym();

        // Disable actor mode
        if (key == "a")
            return;
        
        // Show/hide overlay
        if (key == "Tab") {
            for (auto & anim: this->anims)
                if (anim->is_running()) return;
            
            this->anims.clear();

            auto scene = vtkSmartPointer<vtkAnimationScene>::New();
            scene->SetModeToSequence();
            scene->SetLoop(0);
            scene->SetFrameRate(20);
            scene->SetStartTime(0);
            scene->SetEndTime(0.2);
            scene->AddObserver(vtkCommand::AnimationCueTickEvent, this->model->get_renderer()->GetRenderWindow(), &vtkWindow::Render);
            
            for (auto & widget: this->model->get_widgets()) {
                auto cue = vtkSmartPointer<vtkAnimationCue>::New();
                auto anim = std::make_unique<vtkWidgetFader>();
                cue->SetStartTime(0);
                cue->SetEndTime(0.2);
                anim->set_widget(widget);
                anim->AddObserversToCue(cue);
                scene->AddCue(cue);
                this->anims.emplace_back(std::move(anim));
            }
            scene->Play();
        }

        // Forward events
        vtkInteractorStyleTrackballCamera::OnKeyPress();
    }


    void vtkCustomInteractorStyle::OnLeftButtonUp() {
        vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    }


    void vtkWidgetFader::Start() {
        this->running = true;
        auto repr = dynamic_cast<vtkTextRepresentation*>(this->widget->GetRepresentation());
        if (repr == nullptr) return;
        auto actor = repr->GetTextActor();
        this->forward = actor->GetVisibility();
        actor->VisibilityOn();
        if (this->forward) {
            actor->GetProperty()->SetOpacity(1);
        } else {
            actor->GetProperty()->SetOpacity(0);
            this->widget->SetEnabled(1);
        }
    }

    void vtkWidgetFader::Tick(vtkObject*, unsigned long, void* calldata) {
        auto info = reinterpret_cast<vtkAnimationCue::AnimationCueInfo*>(calldata);
        const double t = (info->AnimationTime - info->StartTime) / (info->EndTime - info->StartTime);
        auto repr = dynamic_cast<vtkTextRepresentation*>(this->widget->GetRepresentation());
        if (repr == nullptr) return;
        auto actor = repr->GetTextActor();
        if (this->forward)
            actor->GetProperty()->SetOpacity(1-t);
        else
            actor->GetProperty()->SetOpacity(t);
    }

    void vtkWidgetFader::End() {
        auto repr = dynamic_cast<vtkTextRepresentation*>(this->widget->GetRepresentation());
        if (repr != nullptr) {
            auto actor = repr->GetTextActor();
            this->widget->SetEnabled(!this->forward);
            if (this->forward)
                actor->VisibilityOff();
        }
        this->running = false;
    }

}