#ifndef EMP_TUTORIAL_H
#define EMP_TUTORIAL_H

#ifdef __EMSCRIPTEN__
  #include "web/web.h"
#endif

#include "base/vector.h"
#include "base/Ptr.h"
#include <iostream>
#include <stdio.h>

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>

#ifdef __EMSCRIPTEN__
  namespace UI = emp::web;
#endif
class Trigger;
class State;
class Tutorial;





class Trigger {

  friend class Tutorial;
  friend class State;

protected:

  virtual ~Trigger(){;}

  emp::Ptr<Tutorial> tutorial_ptr; // pointer to the Tutorial so we can notify it when this trigger fires.

  bool active = false;

    // The same trigger can be used to move between multiple pairs of states, so store all the pairs here.
    std::unordered_map<std::string, std::string> next_state_map;

    std::function<void()> callback;


    bool IsActive() {return active;}

    void Notify( );

    void SetTutorial(emp::Ptr<Tutorial> tut) {tutorial_ptr = tut;}

    // Given a state, what state are we set to move to next?
    std::string GetNextState(std::string current_state) {
      emp_assert(HasState(current_state));
      return next_state_map[current_state];
    }

    // not used, either delete or make Tutorial method for it
    std::vector<std::string> GetStates() { 
      std::vector<std::string> states;
      for (auto state_pair : next_state_map)
        states.push_back(state_pair.first);
      return states;
    }

    // Does the given state contain this trigger?
    bool HasState(std::string state_name) {
      return (next_state_map.find(state_name) != next_state_map.end());
    }

    // How many states contain this trigger?
    int GetStateCount() {
      return next_state_map.size();
    }

    void ManualFire(std::string current_state) {
      emp_assert(HasState(current_state));
      Notify();
    }

    void SetCallback(std::function<void()> cb) { callback = cb; } 


    virtual void Activate() = 0;
    virtual void Deactivate() = 0;


    // Helper functions to keep bookkeeping stuff out of Activate/Deactivate. 
    // Makes it simpler to override those functions in custom classes.
    void PerformActivation() {
      if (active) return;
      std::cout << "in Trigger Perform activation" << std::endl;
      Activate();
      active = true;
    }
    void PerformDeactivation() {
      if (!active) return;
      Deactivate();
      active = false;
    }
    
    // Add a pair of states that this trigger is associated with (it can move the tutorial from state to next_state).
    void AddStatePair(std::string state, std::string next_state) {
      emp_assert(!HasState(state));
      emp_assert(state != next_state);
      next_state_map[state] = next_state;
    }

    void RemoveState(std::string state_name) {
      emp_assert(HasState(state_name));
      next_state_map.erase(state_name);

    }

};



#ifdef __EMSCRIPTEN__
template <typename T>
class EventListenerTrigger : public Trigger {

  friend class Tutorial;
  friend class State;

protected:

  UI::internal::WidgetFacet<T>& widget;
  std::string event_name;
  std::string handler_id;

public:

  EventListenerTrigger(UI::internal::WidgetFacet<T>& _widget, const std::string& _event_name):
    widget(_widget), event_name(_event_name) {
    }

    void Activate() override{
        std::cout << "In Activate! Event name: " << event_name.c_str() << std::endl;
        widget.On(event_name.c_str(), [this]() { this->Notify(); }, event_name + "_tutorial_handler");

    }

    void Deactivate() override {
      std::cout << "In Deactivate! Event name: " << event_name.c_str() << std::endl;
      widget.RemoveListener(event_name, event_name + "_tutorial_handler");
    }

};
#endif




class ManualTrigger : public Trigger {

friend class Tutorial;
friend class State;

private:

  ManualTrigger() {}

    void Activate() override {}
    void Deactivate() override {}

};


class VisualEffect {

  friend class Tutorial;
  friend class State;


private:

  bool active = false;

  // Set of all states using this visual
  std::unordered_set<std::string> states_set;

  virtual void Activate() = 0;
  virtual void Deactivate() = 0;

  // Helper functions to keep bookkeeping stuff out of Activate/Deactivate. 
  // Makes it simpler to override those functions in custom classes.
  void PerformActivation() {
    if (active) return;
    Activate();
    active = true;
  }
  void PerformDeactivation() {
    if (!active) return;
    Deactivate();
    active = false;
  }


  void AddState(std::string state_name) {
    states_set.insert(state_name);
  }

  void RemoveState(std::string state_name) {
    states_set.erase(state_name);
  }

  // How many states contain this visual?
  int GetStateCount() {
    return states_set.size();
  }

  bool IsActive() {return active;}
};


#ifdef __EMSCRIPTEN__
template <typename T>
class CSSEffect : public VisualEffect {

  friend class Tutorial;
  friend class State;

protected:

    UI::internal::WidgetFacet<T>& widget;

    std::unordered_map<std::string, std::string> new_attributes_map;
    std::unordered_map<std::string, std::string> saved_attributes_map;

    CSSEffect(UI::internal::WidgetFacet<T>& _widget, std::string attr, std::string val) : widget(_widget) {
      new_attributes_map[attr] = val;
    }

    void Activate() override {
      for (auto attr_pair : new_attributes_map) {
        saved_attributes_map[attr_pair.first] = widget.GetCSS(attr_pair.first); // store the starting value to reset it after
        widget.SetCSS(attr_pair.first, attr_pair.second);
      }
    }

    void Deactivate() override {
      for (auto attr_pair : new_attributes_map) {
        widget.SetCSS(attr_pair.first, saved_attributes_map[attr_pair.first]);
        //std::cout << "resetting attribute " << attr_pair.first << " to " << saved_attributes_map[attr_pair.first] << std::endl;
      }
    }


};
#endif


#ifdef __EMSCRIPTEN__
template <typename T>
class PopoverEffect : public VisualEffect {

  friend class Tutorial;
  friend class State;

UI::Div parent_widget;
UI::internal::WidgetFacet<T>& widget;
UI::Div popover_container;
UI::Div popover_text;
UI::Div popover_arrow;
//UI::Widget& original_parent_widget;
std::string message;
std::string popover_id;
  public:

  //PopoverEffect(UI::internal::WidgetFacet<T>& _widget, UI::Widget & _orig_parent, std::string _message) : 
  PopoverEffect(UI::internal::WidgetFacet<T>& _widget, std::string _message) : 
      parent_widget(_widget.GetID() + "_popover_parent"),
      widget(_widget),
      popover_container(_widget.GetID() + "_popover_container"),
      popover_text(_widget.GetID() + "_popover_text"),
      popover_arrow(_widget.GetID() + "_popover_arrow"),
      //original_parent_widget(_orig_parent),
      message(_message){
  }

  void Activate() {

    emp_assert(parent_widget != nullptr);

    std::cout << "Adding popover" << std::endl;
    widget.WrapWithInPlace(parent_widget);
    std::cout << "1" << std::endl;
    parent_widget.SetCSS("position", "relative");
    std::cout << "2" << std::endl;
    popover_text << message;
    popover_text.SetAttr("class", "popup_text");
    popover_arrow.SetAttr("class", "popup_arrow");
    std::cout << "3" << std::endl;
    popover_container << popover_text;
    popover_container << popover_arrow;
    popover_container.SetAttr("class", "popup_container popup_show");
    std::cout << "4" << std::endl;
    parent_widget << popover_container;
    if(widget.GetCSS("float") != ""){
      parent_widget.SetCSS("float", widget.GetCSS("float"));
    }
    std::cout << "finished adding" << std::endl;
  }


  void Deactivate() {
  std::cout << "Removing popover" << std::endl;
  popover_container.SetAttr("class", "popup_container");
  std::cout << "1" << std::endl;
  parent_widget->RemoveChild(widget);
  std::cout << "2" << std::endl;
  parent_widget->parent->ReplaceChild(parent_widget, widget);
  std::cout << "Removed popover" << std::endl;
}

};
#endif


#ifdef __EMSCRIPTEN__
class OverlayEffect : public VisualEffect {

  friend class Tutorial;
  friend class State;

private:

  UI::Div& parent;
  UI::Div overlay;
  std::string color;
  float opacity;
  int z_index;
  bool intercept_mouse;

  OverlayEffect(UI::Div& _parent, std::string _color, float _opacity, int _z_index, bool _intercept_mouse) : 
  parent(_parent), color(_color), opacity(_opacity), z_index(_z_index), intercept_mouse(_intercept_mouse) {std::cout << "Overlay Constructor" << std::endl;}

  void Activate() {

    UI::Div over("overlay");
    overlay = over;

    overlay.SetAttr("class", "Tutorial-Overlay-Effect");
    overlay.SetCSS("background-color", color);
    overlay.SetCSS("opacity", opacity);
    overlay.SetCSS("z_index", z_index);
    overlay.SetCSS("position", "absolute");
    overlay.SetCSS("width", "100%");
    overlay.SetCSS("height", "100%");
    overlay.SetCSS("top", "0px");
    overlay.SetCSS("left", "0px");
    if (!intercept_mouse)
      overlay.SetCSS("pointer-events", "none");

    parent << overlay;

  }

  void Deactivate() {

    overlay -> parent -> RemoveChild(overlay);
    std::cout << "removed overlay" << std::endl;
}

};
#endif


class State {

    friend class Tutorial;

private:

    std::unordered_set<std::string> trigger_id_set;
    std::unordered_set<std::string> visual_id_set;


    std::string name;
    std::function<void()> callback;

    State(){;}
    State(std::string _name) : name(_name)  {}

    void SetCallback(std::function<void()> cb) { callback = cb; } 

    bool HasTrigger(std::string trigger_id) {
      return (trigger_id_set.find(trigger_id) != trigger_id_set.end());
    }

    bool HasVisualEffect(std::string visual_id) {
      return (visual_id_set.find(visual_id) != visual_id_set.end());
    }

    // add the trigger id to set of id's
    void AddTrigger(std::string trigger_id) {
      emp_assert(!HasTrigger(trigger_id));
      trigger_id_set.insert(trigger_id);
    }

    // remove the trigger id from set of id's
    void RemoveTrigger(std::string trigger_id) {
      emp_assert(HasTrigger(trigger_id));
      trigger_id_set.erase(trigger_id);
    }

    // add the visual id to set of id's
    void AddVisualEffect(std::string visual_id) {
      emp_assert(!HasVisualEffect(visual_id));
      visual_id_set.insert(visual_id);
    }

    // remove the visual id from set of id's
    void RemoveVisualEffect(std::string visual_id) {
      visual_id_set.erase(visual_id);
    }

    // Activate all triggers and visuals for this state. Called when the state is entered.
    void Activate(const std::unordered_map<std::string, emp::Ptr<Trigger>>& trigger_ptr_map, 
                  const std::unordered_map<std::string, emp::Ptr<VisualEffect>>& visual_ptr_map) {
      std::cout << "Activate state: " << name << std::endl;
      std::cout << "Activateing " << trigger_id_set.size() << " triggers!"  << std::endl;
      std::cout << "Activateing " << visual_id_set.size() << " visuals!"  << std::endl;

      // Activate all triggers
      for(auto trigger_id : trigger_id_set) {
        trigger_ptr_map.at(trigger_id) -> PerformActivation();
        //trigger_ptr_map.at(trigger_id) -> SetActive();

      }

      // Activate all visuals
      for(auto visual_id : visual_id_set) {
        visual_ptr_map.at(visual_id) -> PerformActivation();
        //visual_ptr_map.at(visual_id) -> SetActive();
      }

    }

    // Deactivate all triggers and visuals for this state. Called when the state is exited.
    void Deactivate(const std::unordered_map<std::string, emp::Ptr<Trigger>>& trigger_ptr_map, 
                    const std::unordered_map<std::string, emp::Ptr<VisualEffect>>& visual_ptr_map) {

      // Deactivate all triggers          
      for(auto trigger_id : trigger_id_set) {
        trigger_ptr_map.at(trigger_id) -> PerformDeactivation();
        //trigger_ptr_map.at(trigger_id) -> SetInactive();
      }

      // Deactivate all visuals
      for(auto visual_id : visual_id_set) {
        visual_ptr_map.at(visual_id) -> PerformDeactivation();
        //visual_ptr_map.at(visual_id) -> SetInactive();
      }

      std::cout << "Deactivate state: " << name << std::endl;
      std::cout << "Removing " << trigger_id_set.size() << " triggers!" << std::endl;
      std::cout << "Removing " << visual_id_set.size() << " visuals!" << std::endl;
    }

    // how many Triggers does this state have?
    size_t GetTriggerCount() {
      return trigger_id_set.size();
    }

    // how many VisualEffects does this state have?
    size_t GetVisualEffectCount() {
      return visual_id_set.size();
    }


};



class Tutorial {

  friend void Trigger::Notify(); // Trigger's Notify() can access our private members. Needed so it can call OnTrigger().


private:

  bool active = false;

  std::unordered_map<std::string, State> states;  // Store all the states for this Tutorial
  std::unordered_map<std::string, emp::Ptr<Trigger>> trigger_ptr_map; // Store all the triggers for this Tutorial
  std::unordered_map<std::string, emp::Ptr<VisualEffect>> visual_ptr_map; // Store all the visualeffects for this Tutorial

  std::string current_state;

  size_t num_triggers_added = 0;
  size_t num_visuals_added = 0;

  // Retrieve a State object given its ID.
  State & GetState(std::string & state_name) {
      //add assert
      return states.at(state_name);
  }

  void DeleteTrigger(std::string trigger_id) {
    delete trigger_ptr_map[trigger_id];
    trigger_ptr_map.erase(trigger_id);
  }

  void DeleteVisualEffect(std::string visual_id) {
    delete visual_ptr_map[visual_id];
    visual_ptr_map.erase(visual_id);
  }

  // Retrieve a pointer to the Trigger with the given ID
  emp::Ptr<Trigger> GetTrigger(std::string trigger_id) {
    return trigger_ptr_map[trigger_id];
  }
  

  // Retrieve a pointer to the Trigger with the given ID
  emp::Ptr<VisualEffect> GetVisualEffect(std::string visual_id) {
    return visual_ptr_map[visual_id];
  }

  // A Trigger calls this when it's fired, passing a pointer to itself.
  void OnTrigger(emp::Ptr<Trigger> trigger) {

      std::cout << "Leaving state " << current_state << std::endl;

      // Deactivate current state
      GetState(current_state).Deactivate(trigger_ptr_map, visual_ptr_map);

      //move to the next state
      current_state = trigger -> GetNextState(current_state) ;
      GetState(current_state).Activate(trigger_ptr_map, visual_ptr_map);

      std::cout << "Entering state " << current_state << std::endl;

      // Stop here if this state has no triggers
      if (GetState(current_state).GetTriggerCount() == 0)
      {
        Stop();
      }

      // execute callbacks for the trigger and state
      if (trigger -> callback) trigger -> callback();
      if (GetState(current_state).callback) GetState(current_state).callback();
      

    }




public:

    // -------------------------------- INTERFACE ----------------------------------------

    // These are the only functions to be called outside of this file :P


  bool IsActive() {
    return active;
  }

  std::string GetCurrentState() {
    if (active) return current_state;
    return "";
  }

    // Is the given trigger id an existing trigger?
  bool HasTrigger(std::string trigger_id) {
    return trigger_ptr_map.find(trigger_id) != trigger_ptr_map.end();
  }

    // Is the given visual id an existing trigger?
  bool HasVisualEffect(std::string visual_id) {
    std::cout << "In HasVisualEffect" << std::endl;
    std::cout << (visual_ptr_map.find(visual_id) != visual_ptr_map.end()) << std::endl;
    return visual_ptr_map.find(visual_id) != visual_ptr_map.end();
  }

  // Is the given state name an existing state?
    bool HasState(std::string state_name) {
      return states.find(state_name) != states.end(); // any other checks?
    }

    // Launch into the tutorial at a particular state
    void StartAtState(std::string state_name){

      // Deactivate current state
      if (active)
        GetState(current_state).Deactivate(trigger_ptr_map, visual_ptr_map);

      current_state = state_name;

      // Stop here if new state is an end state
      if (GetState(current_state).GetTriggerCount() == 0)
      {
        Stop();
        return;
      }

      std::cout << "visual size in Start: " << GetState(current_state).GetVisualEffectCount() << std::endl;
      GetState(current_state).Activate(trigger_ptr_map, visual_ptr_map);
      active = true;

      // state callback, if any
      if (GetState(current_state).callback)
        GetState(current_state).callback();
    }

    // End the tutorial
    void Stop() {

      if (!active)
        return;

      // Deactivate current state
      if (HasState(current_state))
        GetState(current_state).Deactivate(trigger_ptr_map, visual_ptr_map);

      active = false;

      std::cout << "Tutorial Finished!" << std::endl;
    }

    // Create and store a new state with given name
    Tutorial& AddState(std::string state_name, std::function<void()> callback=nullptr) {
      emp_assert(!HasState(state_name));
      states.emplace(std::make_pair(state_name, State(state_name)));
      GetState(state_name).SetCallback(callback);

      return *this;
    }

    

    Tutorial& AddManualTrigger(std::string cur_state, std::string next_state, std::string trigger_id="", 
                          std::function<void()> callback=nullptr) {

      if (trigger_id.empty())
        trigger_id = std::string("unnamed_trigger_") + std::to_string(num_triggers_added);

      emp_assert(!HasState(trigger_id));

      emp::Ptr<Trigger> trigger_ptr = new ManualTrigger();
      trigger_ptr -> SetTutorial(this);
      trigger_ptr -> AddStatePair(cur_state, next_state);

      trigger_ptr_map[trigger_id] = trigger_ptr;
      GetState(cur_state).AddTrigger(trigger_id);

      if (cur_state == current_state) {      
        trigger_ptr -> Activate();
      }

      trigger_ptr -> SetCallback(callback);

      num_triggers_added++;

      return *this;
    }


#ifdef __EMSCRIPTEN__
    template <typename T>
    Tutorial& AddEventListenerTrigger(std::string cur_state, std::string next_state,
                                UI::internal::WidgetFacet<T>& w, std::string event_name,
                                std::string trigger_id="", std::function<void()> callback=nullptr)
    {
      if (trigger_id.empty())
        trigger_id = std::string("unnamed_trigger_") + std::to_string(num_triggers_added);

      emp_assert(!HasState(trigger_id));

      emp::Ptr<Trigger> trigger_ptr = new EventListenerTrigger<T>(w, event_name);
      trigger_ptr -> SetTutorial(this);
      trigger_ptr -> AddStatePair(cur_state, next_state);

      trigger_ptr_map[trigger_id] = trigger_ptr;
      GetState(cur_state).AddTrigger(trigger_id);

      if (cur_state == current_state) {
        trigger_ptr -> Activate();
      }

      trigger_ptr -> SetCallback(callback);

      num_triggers_added++;

      return *this;
    }
#endif


    Tutorial& AddExistingTrigger(std::string cur_state, std::string next_state, std::string trigger_id) {

      emp::Ptr<Trigger> trigger_ptr = trigger_ptr_map[trigger_id];
      trigger_ptr -> AddStatePair(cur_state, next_state);
      GetState(cur_state).AddTrigger(trigger_id);

      return *this;
    }


    template<typename T, typename ... Args>
    Tutorial& AddCustomTrigger(std::string cur_state, std::string next_state, Args&&... args, 
                          std::string trigger_id="", std::function<void()> callback=nullptr) {

      std::cout << "The trigger id is: " << trigger_id << std::endl;


      static_assert(std::is_base_of<Trigger, T>::value, "T must derive from Trigger");
      emp::Ptr<Trigger> trigger_ptr = new T(std::forward<Args>(args)...);
      //std::cout << "Created trigger of type: " << static_cast<Trigger*>(trigger_ptr)->GetType() << std::endl;

      trigger_ptr -> SetTutorial(this);
      trigger_ptr -> AddStatePair(cur_state, next_state);

      trigger_ptr_map[trigger_id] = trigger_ptr;
      GetState(cur_state).AddTrigger(trigger_id);

      if (cur_state == current_state) {
        trigger_ptr -> Activate();
      }

      num_triggers_added++;

      return *this;

    }


    Tutorial& RemoveTrigger(std::string trigger_id, std::string state_name) {
      emp_assert(HasTrigger(trigger_id));

      emp::Ptr<Trigger> trigger_ptr = GetTrigger(trigger_id);

      // deactivate the trigger if active
      if (trigger_ptr -> IsActive())
        trigger_ptr -> Deactivate();

      // remove state from trigger
      trigger_ptr -> RemoveState(state_name);

      // remove the trigger from state
      GetState(state_name).RemoveTrigger(trigger_id);

      // remove from tutorial if necessary
      if (trigger_ptr -> GetStateCount() == 0)
        DeleteTrigger(trigger_id);

        return *this;
    }


    Tutorial& FireTrigger(std::string trigger_id) {
      emp_assert(HasTrigger(trigger_id));
      GetTrigger(trigger_id) -> ManualFire(current_state);

      return *this;
    }


    Tutorial& ActivateTrigger(std::string trigger_id) {
      emp_assert(HasTrigger(trigger_id));
      std::cout << "Trying to activate trigger" << std::endl;

      emp::Ptr<Trigger> trigger_ptr = GetTrigger(trigger_id);
      trigger_ptr -> PerformActivation();

      return *this;
    }

    Tutorial& DeactivateTrigger(std::string trigger_id) {
      emp_assert(HasTrigger(trigger_id));

      std::cout << "Try to deactivate trigger" << std::endl;

      emp::Ptr<Trigger> trigger_ptr = GetTrigger(trigger_id);
      trigger_ptr -> PerformDeactivation();

      return *this;
    }


#ifdef __EMSCRIPTEN__
    template <typename T>
    Tutorial& AddCSSEffect(std::string state_name, UI::internal::WidgetFacet<T>& w,
                      std::string attr, std::string val, std::string visual_id="")
    {
      emp::Ptr<VisualEffect> visual_ptr = new CSSEffect<T>(w, attr, val);
      visual_ptr -> AddState(state_name);

      if (visual_id.empty())
        visual_id = std::string("unnamed_visual_") + std::to_string(num_visuals_added);


      visual_ptr_map[visual_id] = visual_ptr;
      GetState(state_name).AddVisualEffect(visual_id);

      if (state_name == current_state){
        visual_ptr -> Activate();
      }

      num_visuals_added++;

      return *this;
    }
#endif

#ifdef __EMSCRIPTEN__
    template <typename T>
    Tutorial& AddPopoverEffect(std::string state_name, UI::internal::WidgetFacet<T>& w,
                                std::string message, std::string visual_id="")
    {
      emp::Ptr<VisualEffect> visual_ptr = new PopoverEffect<T>(w, message);
      visual_ptr -> AddState(state_name);

      if (visual_id.empty())
        visual_id = std::string("unnamed_visual_") + std::to_string(num_visuals_added);



      visual_ptr_map[visual_id] = visual_ptr;
      GetState(state_name).AddVisualEffect(visual_id);

      if (state_name == current_state){
        visual_ptr -> Activate();
      }

      num_visuals_added++;

      return *this;
    }
#endif


#ifdef __EMSCRIPTEN__
    Tutorial& AddOverlayEffect(std::string state_name, UI::Div& parent, std::string color="black", float opacity=0.4, 
                              int z_index=1000, bool intercept_mouse=false, std::string visual_id="") {

      std::cout << "Add Overlay Effect" << std::endl;
      emp_assert(HasState(state_name));
      

      emp::Ptr<VisualEffect> visual_ptr = new OverlayEffect(parent, color, opacity, z_index, intercept_mouse);
      visual_ptr -> AddState(state_name);

      if (visual_id.empty())
        visual_id = std::string("unnamed_visual_") + std::to_string(num_visuals_added);

      std::cout << visual_id << std::endl;
      emp_assert(!HasVisualEffect(visual_id));

      visual_ptr_map[visual_id] = visual_ptr;
      GetState(state_name).AddVisualEffect(visual_id);

      if (state_name == current_state){
        visual_ptr -> Activate();
      }

      num_visuals_added++;

      return *this;
    }
#endif

    template<typename T, typename ... Args>
    Tutorial& AddCustomVisualEffect(std::string state_name, Args&&... args,
                               std::string visual_id) {

      emp_assert(!HasVisualEffect(visual_id));

      static_assert(std::is_base_of<VisualEffect, T>::value, "T must derive from VisualEffect");
      emp::Ptr<VisualEffect> visual_ptr = new T(std::forward<Args>(args)...);

      visual_ptr -> AddState(state_name);

      if (visual_id.empty())
        visual_id = std::string("unnamed_visual_") + std::to_string(num_visuals_added);

      visual_ptr_map[visual_id] = visual_ptr;
      GetState(state_name).AddVisualEffect(visual_id);

      if (state_name == current_state) {
        visual_ptr -> Activate();
      }

      num_visuals_added++;

      return *this;

    }

    Tutorial& RemoveVisualEffect(std::string visual_id, std::string state_name) {
      emp_assert(HasVisualEffect(visual_id));

      emp::Ptr<VisualEffect> visual_ptr = GetVisualEffect(visual_id);

      // deactivate the trigger if active
      if (visual_ptr -> IsActive())
        visual_ptr -> Deactivate();

      // remove state from visual
      visual_ptr -> RemoveState(state_name);

      // remove visual from state
      GetState(state_name).RemoveVisualEffect(visual_id);

      // remove from tutorial if necessary
      if (visual_ptr -> GetStateCount() == 0)
        DeleteVisualEffect(visual_id);

        return *this;
    }

     Tutorial& ActivateVisualEffect(std::string visual_id) {
      emp_assert(HasVisualEffect(visual_id));

      emp::Ptr<VisualEffect> visual_ptr = GetVisualEffect(visual_id);
      visual_ptr -> PerformActivation();

      return *this;
    }

    Tutorial& DeactivateVisualEffect(std::string visual_id) {
      std::cout << "In DeactivateVisualEffect" << std::endl;
      std::cout << visual_id << std::endl;
      emp_assert(HasVisualEffect(visual_id));

      emp::Ptr<VisualEffect> visual_ptr = GetVisualEffect(visual_id);
      visual_ptr -> PerformDeactivation();

      return *this;
    }


    Tutorial& SetStateCallback(std::string state_name, std::function<void()> fun) {
          emp_assert(HasState(state_name));
          GetState(state_name).callback = fun;

          return *this;
      }

    Tutorial& SetTriggerCallback(std::string trigger_id, std::function<void()> fun) {
        emp_assert(HasTrigger(trigger_id));
        GetTrigger(trigger_id) -> callback = fun;

        return *this;
    }




    bool IsTriggerActive(std::string trigger_id) {
      emp_assert(HasTrigger(trigger_id));

      return GetTrigger(trigger_id) -> IsActive();
    }

    int GetTriggerCount(std::string trigger_id) {
      emp_assert(HasTrigger(trigger_id));

      return GetTrigger(trigger_id) -> GetStateCount();
    }


    bool IsVisualEffectActive(std::string visual_id) {
      emp_assert(HasVisualEffect(visual_id));

      return GetVisualEffect(visual_id) -> IsActive();
    }

    int GetStateVisualEffectCount(std::string state_name, std::string visual_id) {
      emp_assert(HasState(state_name));
      emp_assert(HasVisualEffect(visual_id));

      return GetState(state_name).GetVisualEffectCount();
    }


    bool StateHasTrigger(std::string state_name, std::string trigger_id) {
      emp_assert(HasState(state_name));
      emp_assert(HasTrigger(trigger_id));

      return GetState(state_name).HasTrigger(trigger_id);
    }

    bool StateHasVisual(std::string state_name, std::string visual_id) {
      emp_assert(HasState(state_name));
      emp_assert(HasVisualEffect(visual_id));

      return GetState(state_name).HasVisualEffect(visual_id);
    }

  

};



void Trigger::Notify(){
    tutorial_ptr -> OnTrigger(this);
}

#endif
