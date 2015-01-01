#ifndef EMP_EVENTS_H
#define EMP_EVENTS_H

#include "Kinetic.h"

namespace emp {

  class EventChain;
  
  class Event {
  protected:
    Event * next;       // which event should go next?
    bool next_simul;    // shouild the next event be simultaneous with this one (is possible)?
    
  public:
    Event() : next(NULL), next_simul(false) { ; }
    virtual ~Event() { if (next) delete next; }

    Event * GetNext() const { return next; }
    bool GetNextSimul() const { return next_simul; }

    virtual void Trigger(EventChain * chain) = 0;

    Event * Then(Event * _next) { next = _next; next_simul=false; return next; }
    Event * With(Event * _next) { next = _next; next_simul=true; return next; }
  };

  class Event_Tween : public Event { 
  private:
    Tween & tween;

  public:
    Event_Tween(Tween & _tween) : tween(_tween) { ; }
    ~Event_Tween() { ; }

    void Trigger(EventChain * chain) {
      // If there is something to be run at the same time, make sure to do so!
      if (next && next_simul) {
        tween.Play();
        next->Trigger(chain);
      }

      // Otherwise, setup a callback before playing so that the chain knows when to take the next step.
      else {
        tween.SetFinishedCallback((Callback *) chain, (int *) next);
        tween.Play();
      }
    }
  };

  class EventChain : public emp::Callback {
  private:
    Event * first;
    Event * last;
    int length;
    bool is_running;

    Event * BuildEvent(Tween & tween) { return new Event_Tween(tween); }

  public:
    EventChain() : first(NULL), last(NULL), length(0), is_running(false) { ; }
    ~EventChain() { Clear(); }

    int GetSize() { return length; }
    bool IsRunning() const { return is_running; }

    void Clear() { if (first) delete first; first = NULL; last = NULL; }

    void DoCallback(int * next) {
      // The previous event must have fiinshed.  Move on to the next one!
      if (next) ((Event *) next)->Trigger(this);
      else is_running = false;
    }

    template <class T> EventChain & First(T & _in) {
      EMPassert(is_running == false);
      if (first != NULL) Clear();      // Since we're starting over, clear any existing events.
      first = last = BuildEvent(_in);
      length = 1;
      return *this;
    }

    template <class T> EventChain & Then(T & _in) {
      EMPassert(first != NULL && last != NULL);
      EMPassert(is_running == false);
      last = last->Then( BuildEvent(_in) );
      length++;
      return *this;
    }

    template <class T> EventChain & With(T & _in) {
      EMPassert(first != NULL && last != NULL);
      EMPassert(is_running == false);
      last = last->With( BuildEvent(_in) );
      length++;
      return *this;
    }

    void Trigger() {
      EMPassert(is_running == false);
      is_running = true;
      if (first) first->Trigger(this);
    }
  };

};

#endif
