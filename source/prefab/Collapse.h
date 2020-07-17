#ifndef EMP_COLLAPSE_H
#define EMP_COLLAPSE_H

#include "../web/Div.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"

namespace emp {
namespace prefab{
namespace internal {

    // CollpaseController wraps web widgets in the necessary html to function 
    // as the controller for a group or groups of target areas.
    class CollapseController {
        private:
            web::Div controller;
        public:
            template <typename T>
            CollapseController(T in_controller, std::string controls_class, bool expanded, std::string id="") : controller(id){
                if(in_controller.HasAttr("data-target")){
                    // the controller passed to constructor already controls another target area
                    // just append the new controls_class to the existing data-target and aria-controls attributes
                    controller = in_controller;
                    std::string all_classes = controller.GetAttr("data-target");
                    all_classes += ", ." + controls_class;
                    controller.SetAttr("data-target", all_classes);
                    controller.SetAttr("aria-controls", all_classes);
                }
                else{
                    // the controller passed to the constructor needs all the attributes necessary for controllers
                    controller << in_controller;
                    controller.SetAttr(
                        "data-toggle", "collapse",
                        "role", "button",
                        "class", "collapse_toggle",
                        "data-target", "." + controls_class,
                        "aria-controls", "." + controls_class
                    );
                }

                
                if(expanded){
                    controller.SetAttr("aria-expanded", "true");
                }
                else{
                    controller.SetAttr("aria-expanded", "false");
                    // check to see if controller already has collapsed class, 
                    // no need to add it again if it does
                    if(controller.GetAttr("class").find("collapsed") == std::string::npos){
                        controller.AddClass("collapsed");
                    }
                }
            }

            web::Div & GetLinkDiv() {return controller;}
    };
}

// CollapseCoupling maintains a group of targets and controllers. 
// When a controller is clicked on a web page, all the associated targets 
// will change state (expand/collapse).
class CollapseCoupling {
    private:
        emp::vector<web::Div> targets;
        emp::vector<web::Div> controllers;
        std::string target_class;
        static int counter; // used to generate unique class names, shared by all instances of this class
    public:
        template <typename T, typename S>
        CollapseCoupling(T in_controller, S in_target, bool expanded=false, std::string in_class="")
        : CollapseCoupling(web::Div{} << in_controller, web::Div{} << in_target, expanded, in_class) {
            // stream controller and target into their own div and call constructor that takes widgets as parameters
            ;
        }
        CollapseCoupling(web::Widget in_controller, web::Widget in_target, bool expanded=false, std::string in_class="")
        : CollapseCoupling(emp::vector<web::Widget>{in_controller}, emp::vector<web::Widget>{in_target}, expanded, in_class) {
            // place the widgets into their own vector and call the constructor that takes vectors as parameters
            ;
        }
        CollapseCoupling(emp::vector<web::Widget> in_controllers, emp::vector<web::Widget> in_targets, bool expanded=false, std::string in_class="")
        {
            // if a class is defined by the user, use it
            // Otherwise generate a unique class
            if(in_class.compare("") == 0){
                target_class = "emp__collapse_class_" + std::to_string(counter);
                counter++;
            }
            else{
                target_class = in_class;
            }
            // add controllers to this object
            for(emp::vector<web::Widget>::iterator it = in_controllers.begin(); it != in_controllers.end(); ++it){
                AddController(*it, expanded);
            }
            // add targets to this object
            for(emp::vector<web::Widget>::iterator it = in_targets.begin(); it != in_targets.end(); ++it){
                AddTarget(*it, expanded);
            }
        }

        // Adds a controller to the vector of controllers for this CollapseCouple
        void AddController(web::Widget in_controller, bool expanded){
            internal::CollapseController controller(in_controller, target_class, expanded);
            controllers.push_back(controller.GetLinkDiv());
        }
        // If the controller is not a web widget, place it in a div and call the other AddController function
        template<typename T>
        void AddController(T in_controller, bool expanded){
            AddController(web::Div{} << in_controller, expanded);
        }

        // Adds a target to the vector of targets for this CollapseCouple
        void AddTarget(web::Widget in_target, bool expanded){
            web::Div target;
            target << in_target;
            if(expanded){
                target.SetAttr("class", "collapse show");
            }
            else{
                target.SetAttr("class", "collapse");
            }
            target.AddClass(target_class);
            targets.push_back(target);
        }                
        // If the target is not a web widget, place it in a div and call the other AddTarget function
        template<typename T>
        void AddTarget(T in_target, bool expanded){
            AddTarget(web::Div{} << in_target, expanded);
        }

        // Returns the target_class associated with this CollapseCouple
        std::string GetTargetClass(){
            return target_class;
        }

        // Functions used to retrieve controllers and targets
        // TODO: In the future, add capability to call controllers and targets by name 
        // (Like dictionary accesses, key/value pairs)
        
        // Returns the vector of all controllers associated with this CollapseCouple
        emp::vector<web::Div> & GetControllerDiv(){
            return controllers;
        }
        // Returns the controller at the given index
        web::Widget & GetControllerDiv(int index){
            return controllers[index];
        }
        // Returns the vector of all targets associated with this CollapseCouple
        emp::vector<web::Div> & GetTargetDiv(){
            return targets;
        }
        // Returns the target at the given index
        web::Div & GetTargetDiv(int index){
            return targets[index];
        }
};
int CollapseCoupling::counter = 0; // static vars must not be initialized within the class
}
}

#endif