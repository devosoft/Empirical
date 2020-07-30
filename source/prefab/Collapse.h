#ifndef EMP_COLLAPSE_H
#define EMP_COLLAPSE_H

#include "../web/Div.h"
#include "../web/Widget.h"
#include "../web/_FacetedWidget.h"
#include "../tools/string_utils.h"

namespace emp {
namespace prefab{
  namespace internal {

    /// CollpaseController class adds necessary html attributes to in_controller
    /// to function as the controller for a group or groups of target areas.
    /// Only ever called by CollapseCoupling class.
    class CollapseController {
      private:
        web::Div controller; // reference to controller passed to constructor
      public:
        /// @param in_controller web element that cause target area(s) to expand/collapse when clicked
        /// @param controls_class class of the target(s) controller should control
        /// @param expanded whether or not the target(s) are initially in an expanded/open state
        template <typename T>
        CollapseController(
            T in_controller,
            std::string controls_class,
            bool expanded, std::string id=""
          ) : controller(id){

          controller = in_controller;
          controller.SetAttr(
              "role", "button",
              "data-toggle", "collapse"
          );
          controller.AddAttr(
              "data-target", "." + controls_class,
              "aria-controls", "." + controls_class,
              "class", "collapse_toggle"
          );

          if(expanded){
              controller.SetAttr("aria-expanded", "true");
          }
          else{
              controller.SetAttr("aria-expanded", "false");
              controller.AddAttr("class", "collapsed");
          }
        }

        web::Div & GetLinkDiv() {return controller;}
    };
  }

  /// CollapseCoupling class maintains a group of targets and controllers.
  /// When a controller is clicked on a web page, all the associated targets
  /// will change state (expand/collapse).
  class CollapseCoupling {
    private:
      // all web elements that expand/collapse with this coupling
      emp::vector<web::Widget> targets;
      // all web elements that control expanding/collapsing with this coupling
      emp::vector<web::Widget> controllers;
      // class associated with this
      std::string target_class;
      // counter used to generate unique class names, shared by all instances of this class
      inline static int counter = 0;
    public:
      /// Constructor which takes web elements as the controller and target
      /// It will put each in a vector and call the constructor which takes
      /// vectors as controller and target parameters.
      CollapseCoupling(
        web::Widget in_controller,
        web::Widget in_target,
        bool expanded=false, std::string in_class=""
      ) : CollapseCoupling(emp::vector<web::Widget>{in_controller},
                            emp::vector<web::Widget>{in_target}, expanded, in_class) {
      }

      /// Constructor which takes vectors of web elements as the controller and target
      /// It will create the collapse coupling by adding necessary HTML attributes
      /// to all controllers and targets
      CollapseCoupling(
        emp::vector<web::Widget> in_controllers,
        emp::vector<web::Widget> in_targets,
        bool expanded=false,
        std::string in_class="")
      {
          // if a class is defined by the user, use it
          // Otherwise generate a unique class
          if(in_class == ""){
              target_class = "emp__collapse_class_" + std::to_string(counter);
              counter++;
          }
          else{
              target_class = in_class;
          }
          // add controllers to this coupling
          for(auto & widget : in_controllers){
              AddController(widget, expanded);
          }
          // add targets to this coupling
          for(auto & widget : in_targets){
              AddTarget(widget, expanded);
          }

      }

      /// Constructor which takes strings as the controller and target
      /// It will put each in a Div element and call the constructor which takes
      /// web elements as controller and target parameters.

      // TODO: Ideally, this constructor will be tempalted and can handle any input that is
      // not a Widget or vector of Widgets.
      // When we tried this before, all input would go through to this constructor.
      // This caused issues when it tried to stream widgets into a div but the widget
      // already had another parent.
      // Note: Maybe if we web::internal::FacetedWidget intead of web::Widget in the
      // first constructor, templating this constructor will work?
      CollapseCoupling(
        const std::string in_controller,
        const std::string in_target,
        bool expanded=false,
        std::string in_class=""
      ): CollapseCoupling(web::Div{} << in_controller, web::Div{} << in_target, expanded, in_class) {
      }

      /// Adds a controller to the vector of controllers for this CollapseCouple
      /// @param in_controller new controller to add to coupling is of type Widget
      /// @param expaned initial state of the target(s), is it expaned or not?
      void AddController(web::Widget in_controller, bool expanded){
          internal::CollapseController controller(in_controller, target_class, expanded);
          controllers.push_back(controller.GetLinkDiv());
      }

      /// Adds a controller to the vector of controllers for this CollapseCouple.
      /// @param in_controller new controller to add to coupling is of type string
      /// @param expaned initial state of the target(s), is it expaned or not?
      // TODO: Ideally, this method would be templated, but running into same issues
      // as when trying to do this with the constructor
      void AddController(const std::string in_controller, bool expanded){
          AddController(web::Div{} << in_controller, expanded);
      }

      /// Adds a target to the vector of targets for this CollapseCouple
      /// @param widget new target to add to coupling is a web element
      /// @param expaned initial state of the target(s), is it expaned or not?
      void AddTarget(web::internal::FacetedWidget widget, bool expanded){
          if(expanded){
              widget.AddAttr("class", "collapse show");
          }
          else{
              widget.AddAttr("class", "collapse");
          }
          widget.AddAttr("class", target_class);
          targets.push_back(widget);
      }

      /// Adds a target to the vector of targets for this CollapseCouple
      /// @param widget new target to add to coupling is a string
      /// @param expaned initial state of the target(s), is it expaned or not?
      // If the target is not a web widget, place it in a div and call the other AddTarget function
      // TODO: Ideally, this method would be templated, but running into same issues
      // as when trying to do this with the constructor
      void AddTarget(const std::string in_target, bool expanded){
          AddTarget(web::Div{} << in_target, expanded);
      }

      /// Returns the target_class associated with this CollapseCouple
      std::string GetTargetClass(){
          return target_class;
      }

      // Functions used to retrieve controllers and targets
      // TODO: In the future, add capability to call controllers and targets by name
      // (Like dictionary accesses, key/value pairs)

      /// Returns the vector of all controllers associated with this CollapseCouple
      emp::vector<web::Widget> & GetControllerDivs(){
          return controllers;
      }

      /// Returns the controller at the given index
      web::Widget & GetControllerDiv(int index=0){
          return controllers[index];
      }

      /// Returns the vector of all targets associated with this CollapseCouple
      emp::vector<web::Widget> & GetTargetDivs(){
          return targets;
      }

      /// Returns the target at the given index
      web::Widget & GetTargetDiv(int index=0){
          return targets[index];
      }
  };
}
}

#endif