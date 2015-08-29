#include <emscripten.h>
#include <iostream>
#include <string>



/*class Node(){
 private:
  int id;
  
 public:
  Node();
  ~Node();
  string GetData(){EM_ASM(allocate(intArrayFromString($dom_nodes.nodes[this->id].data), 'i8', ALLOC_STACK))};
  
};



Node::Node(){
  
}*/

extern "C" {
  extern int n_selections();
}

namespace D3 {

  class Selection{
  private:
    int id;
    bool enter;
    bool exit;
    Selection(int id); //append constructor

  public:
    Selection(const char* selector, bool all = false);
    //Selection(Selection* selector, bool all = false); //Do we need this?
    //~Selection(); //tilde is creating demangling issues

    Selection Select(const char* selector){
      int new_id = EM_ASM_INT_V({return js.selections.length});
      EM_ASM_ARGS({
	  var new_selection = js.selections[$0].select(Pointer_stringify($1));
	  js.selections.push(new_selection);
	}, this->id, selector);
      return Selection(new_id);
    }
    
    Selection SelectAll(const char* selector){
      int new_id = EM_ASM_INT_V({return js.selections.length});
      EM_ASM_ARGS({
	  var new_selection = js.selections[$0].selectAll(Pointer_stringify($1));
	  js.selections.push(new_selection);
	}, this->id, selector);
      return Selection(new_id);
    }

    template <typename T>
    void SetAttr(const char* name, T value){
      /* Assigns [value] to the selections='s [name] attribute.
	 This method handles numeric values - use SetAttrString
	 for non-numeric values. */

      EM_ASM_ARGS({js.selections[$0].attr(Pointer_stringify($1), $2)},
		  this->id, name, value);
    }

    void SetAttr(const char* name, const char* value){
      /* Assigns [value] to the selections='s [name] attribute.
	 This method handles numeric values - use SetAttrString
	 for non-numeric values. */

      EM_ASM_ARGS({js.selections[$0].attr(Pointer_stringify($1), 
		       eval(Pointer_stringify($2)))}, this->id, name, value);
    }

    Selection Append(const char* name){
      int new_id = EM_ASM_INT_V({return js.selections.length});

      std::cout << "creating " << new_id << std::endl; 
      EM_ASM_ARGS({
	  var new_selection = js.selections[$0].append(Pointer_stringify($1));
	  js.selections.push(new_selection);
	}, this->id, name);
      return Selection(new_id);
    }
    
    void SetStyle(const char* name, const char* value, bool priority=false){
      if (priority){
	EM_ASM_ARGS({js.selections[$0].style(Pointer_stringify($1), 
	Pointer_stringify($2), "important")}, this->id, name, value);
      }
      else {
	EM_ASM_ARGS({js.selections[$0].style(Pointer_stringify($1), 
	Pointer_stringify($2))}, this->id, name, value);
      }
    }

    template <typename T>
    void SetStyle(const char* name, T value, bool priority=false){
      if (priority){
	EM_ASM_ARGS({js.selections[$0].style(Pointer_stringify($1), 
	$2, "important")}, this->id, name, value);
      }
      else {
	EM_ASM_ARGS({js.selections[$0].style(Pointer_stringify($1), 
	$2)}, this->id, name, value);
      }
    }
    
    //TODO: Try to make this generalize better - there shouldn't
    //need to be a different function for each specific type.
    Selection Data(int32_t* values, int length){
      int update_id = EM_ASM_INT_V({return js.selections.length});
      std::cout << update_id << std::endl;
      EM_ASM_ARGS({
	  var d = [];
	  for (i=0; i<$2; i++){
	    d.push(getValue($1+(i*4), 'i32'));
	  }
	  var update_selection = js.selections[$0].data(d);
	  js.selections.push(update_selection);
	}, 
	    this->id, values, length);
      Selection update = Selection(update_id);
      update.enter = true;
      update.exit = true;
      return update;
    }

    Selection EnterAppend(const char* type){

      int new_id = EM_ASM_INT_V({return js.selections.length});

      std::cout << "id of update " << this->id << " id of enter" << new_id << std::endl;
      if (!this->enter){
	std::cout << "No valid enter selection" << std::endl;
	//TODO: this is an error. throw exception
      }
      EM_ASM_ARGS({
	  var append_selection = js.selections[$0].enter().append(Pointer_stringify($1));
	  js.selections.push(append_selection);
	    }, this->id, type);
      return Selection(new_id);
    }

    int GetAttrInt(const char* name){
      return EM_ASM_INT({
	  return js.selections[$0].attr(Pointer_stringify($1));
	}, this->id, name);
    }

    double GetAttrDouble(const char* name){
      return EM_ASM_DOUBLE({
	  return js.selections[$0].attr(Pointer_stringify($1));
	}, this->id, name);
    }

    int GetStyleInt(const char* name){
      return EM_ASM_INT({
	  return js.selections[$0].style(Pointer_stringify($1));
	}, this->id, name);
    }

    double GetStyleDouble(const char* name){
      return EM_ASM_INT({
	  return js.selections[$0].style(Pointer_stringify($1));
	}, this->id, name);
    }

    void SetText(const char* text){
      EM_ASM_ARGS({js.selections[$0].text(Pointer_stringify($1))}, 
		  this->id, text);
    }

    void SetHtml(const char* value){
      EM_ASM_ARGS({js.selections[$0].html(Pointer_stringify($1))}, 
		  this->id, value);
    }    

    void insert(const char* name, const char* before=NULL){
      if (before){
	EM_ASM_ARGS({js.selections[$0].insert(Pointer_stringify($1), 
					      Pointer_stringify($2))}, 
		    this->id, name, before);
      } else {
	EM_ASM_ARGS({js.selections[$0].insert(Pointer_stringify($1))}, 
		    this->id, name);
      }
    }    

    void Remove(){
      EM_ASM_ARGS({js.selections[$0].remove()}, 
		  this->id);
    }    



    //TODO:
    //
    //GetAttrString
    //GetStyleString()

    //GetClassed()
    //SetClassed()
    
    //GetProperty()
    //SetProperty()
    //GetText()
    //GetHtml()
    //Enter() //also pretty important
    //Exit() //this one too
    //Filter()
    //Datum()
    //Sort()
    //Order()
    //On() //also notable
    //Transition()
    //Interrupt()

    //Each()
    //Call()
    //Empty()
    //Node()
    //Size()
    


  };

  Selection::Selection(const char* selector, bool all){
    this->id = EM_ASM_INT_V({return js.selections.length});
    this->enter = false;
    this->exit = false;
    if (all){
      EM_ASM_ARGS({
	  js.selections[$0] = 
	  d3.selectAll(Pointer_stringify($1))}, this->id, selector);
    }
    else {
      EM_ASM_ARGS({
	  js.selections[$0] = 
	  d3.select(Pointer_stringify($1))}, this->id, selector);
    }
  }

  Selection::Selection(int id){
    this->id = id;
    this->enter = false;
    this->exit = false;
  }

  /* I don't know why you'd actually want to do this
  Selection::Selection(Selection* selector, bool all){
    this->id = EM_ASM_INT_V({return js.selections.length});
    if (all){
      EM_ASM_ARGS({
	  js.selections[$0] = 
	  d3.selectAll($1.id)}, this->id, selector);
    }
    else {
      EM_ASM_ARGS({
	  js.selections[$0] = 
	  d3.select(Pointer_stringify($1))}, this->id, selector);
    }
  }
  */

//void bind_data(std::vector data){
// 
//}
}
