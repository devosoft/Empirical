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

  public:
    Selection(int id); //append constructor
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
    Selection SetAttr(const char* name, T value){
      /* Assigns [value] to the selection's [name] attribute.
	 This method handles numeric values - use SetAttrString
	 for non-numeric values. */

      EM_ASM_ARGS({js.selections[$0].attr(Pointer_stringify($1), $2)},
		  this->id, name, value);
      return *this;
    }

    Selection SetAttr(const char* name, const char* value){
      /* Assigns [value] to the selections='s [name] attribute.
	 This method handles numeric values - use SetAttrString
	 for non-numeric values. 
	 
	 This will break if someone happens to use a string that
	 is identical to a function name... but that's unlikely, right?
      */

      EM_ASM_ARGS({
	  var in_string = Pointer_stringify($2);
	  var fn = window["emp"][in_string];
	  if (typeof fn === "function"){
	    js.selections[$0].attr(Pointer_stringify($1), fn);
	  } else {
	    js.selections[$0].attr(Pointer_stringify($1), in_string);
	  }}, this->id, name, value);
      return *this;
    }


    Selection Append(const char* name){
      int new_id = EM_ASM_INT_V({return js.selections.length});

      EM_ASM_ARGS({
	  var new_selection = js.selections[$0].append(Pointer_stringify($1));
	  js.selections.push(new_selection);
	}, this->id, name);
      return Selection(new_id);
    }
    
    Selection SetStyle(const char* name, const char* value, bool priority=false){
      if (priority){
	EM_ASM_ARGS({
	    var in_string = Pointer_stringify($2);
	    var fn = window["emp"][in_string];
	    if (typeof fn === "function"){
	      js.selections[$0].style(Pointer_stringify($1), fn, "important");
	    } else {
	      js.selections[$0].style(Pointer_stringify($1), 
				      in_string, "important");
	    }}, this->id, name, value);
      }
      else {
	EM_ASM_ARGS({
	    var in_string = Pointer_stringify($2);
	    var fn = window["emp"][in_string];
	    if (typeof fn === "function"){
	      js.selections[$0].style(Pointer_stringify($1), fn);
	    } else {
	      js.selections[$0].style(Pointer_stringify($1), in_string);
	    }}, this->id, name, value);
      }
      return *this;
    }

    template <typename T>
    Selection SetStyle(const char* name, T value, bool priority=false){
      if (priority){
	EM_ASM_ARGS({js.selections[$0].style(Pointer_stringify($1), 
	$2, "important")}, this->id, name, value);
      }
      else {
	EM_ASM_ARGS({js.selections[$0].style(Pointer_stringify($1), 
	$2)}, this->id, name, value);
      }
      return *this;
    }

    Selection SetProperty(const char* name, const char* value){
      EM_ASM_ARGS({
	  var in_string = Pointer_stringify($2);
	  var fn = window["emp"][in_string];
	  if (typeof fn === "function"){
	    js.selections[$0].property(Pointer_stringify($1), fn);
	  } else {
	    js.selections[$0].property(Pointer_stringify($1), in_string);
	  }}, this->id, name);
      return *this;
    }

    template <typename T>
    Selection SetProperty(const char* name, T value){
      EM_ASM_ARGS({js.selections[$0].property(Pointer_stringify($1), 
					   $2)}, this->id, name);
      return *this;
    }
    
    //TODO: Try to make this generalize better - there shouldn't
    //need to be a different function for each specific type.
    Selection Data(int32_t* values, int length, const char* key=""){
      int update_id = EM_ASM_INT_V({return js.selections.length});
      
      EM_ASM_ARGS({
	  var d = [];
	  for (i=0; i<$2; i++){
	    d.push(getValue($1+(i*4), 'i32'));
	  }
	  var in_string = Pointer_stringify($3);
	  var fn = window["emp"][in_string];
	  if (typeof fn === "function"){
	    var update_selection = js.selections[$0].data(d, fn);
	  } else {
	    var update_selection = js.selections[$0].data(d);
	  }
	  
	  js.selections.push(update_selection);
	}, this->id, values, length, key);
      Selection update = Selection(update_id);
      update.enter = true;
      update.exit = true;
      return update;
    }

    Selection EnterAppend(const char* type){

      int new_id = EM_ASM_INT_V({return js.selections.length});
      
      if (!this->enter){
	std::cout << "No valid enter selection" << std::endl;
	//TODO: this is an error. throw exception
      }
      EM_ASM_ARGS({
	  var append_selection = js.selections[$0]
	    .enter().append(Pointer_stringify($1));
	  js.selections.push(append_selection);
	    }, this->id, type);
      return Selection(new_id);
    }

    Selection ExitRemove(){
      int new_id = EM_ASM_INT_V({return js.selections.length});
      
      if (!this->exit){
	std::cout << "No valid enter selection" << std::endl;
	//TODO: this is an error. throw exception
      }
      EM_ASM_ARGS({
	  var exit_selection = js.selections[$0].exit().remove();
	  js.selections.push(exit_selection);
	    }, this->id);
      return Selection(new_id);
    }


    Selection Exit(){
      /* Usually the only thing you want to do with the exit selection
	 is remove it's contents, in which case you should use the
	 ExitRemove method. However, advanced users may want to operate
	 on the exit selection, which is why this method is provided.*/
	 
      int new_id = EM_ASM_INT_V({return js.selections.length});
      
      if (!this->exit){
	std::cout << "No valid enter selection" << std::endl;
	//TODO: this is an error. throw exception
      }
      EM_ASM_ARGS({
	  var exit_selection = js.selections[$0].exit();
	  js.selections.push(exit_selection);
	    }, this->id);
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
      EM_ASM_ARGS({
	  var in_string = Pointer_stringify($1);
	  var fn = window["emp"][in_string];
	  if (typeof fn === "function"){
	    js.selections[$0].text(fn);
	  } else {
	    js.selections[$0].text(in_string);
	  }}, this->id, text);
    }

    void SetHtml(const char* value){
      EM_ASM_ARGS({
	  var in_string = Pointer_stringify($1);
	  var fn = window["emp"][in_string];
	  if (typeof fn === "function"){
	    js.selections[$0].html(fn);
	  } else {
	    js.selections[$0].html(in_string);
	  }}, this->id, value);
    }    

    Selection Transition(const char* name=""){
      int new_id = EM_ASM_INT_V({return js.selections.length});
      EM_ASM_ARGS({
	  var transition = js.selections[$0].transition(Pointer_stringify($1));
	  js.selections.push(transition);
	}, this->id, name);
      return Selection(new_id);
    }    

    void Interrupt(const char* name=""){
      EM_ASM_ARGS({
	  js.selections[$0].interrupt(Pointer_stringify($1));
	}, this->id, name);
    }    

    Selection Insert(const char* name, const char* before=NULL){
      int new_id = EM_ASM_INT_V({return js.selections.length});

      if (before){
	EM_ASM_ARGS({
	    var new = js.selections[$0].insert(Pointer_stringify($1), 
						  Pointer_stringify($2));
	    js.selections.push(new);
	  }, this->id, name, before);
      } else {
	EM_ASM_ARGS({
	    var new = js.selections[$0].insert(Pointer_stringify($1));
	    js.selections.push(new);
	  }, this->id, name);
      }
      return Selection(new_id);
    }    


    Selection EnterInsert(const char* name, const char* before=NULL){
      int new_id = EM_ASM_INT_V({return js.selections.length});

      if (!this->enter){
	std::cout << "No valid enter selection" << std::endl;
	//TODO: this is an error. throw exception
      }

      if (before){
	EM_ASM_ARGS({
	    var new = js.selections[$0].enter().insert(Pointer_stringify($1), 
						  Pointer_stringify($2));
	    js.selections.push(new);
	  }, this->id, name, before);
      } else {
	EM_ASM_ARGS({
	    var new = js.selections[$0].enter().insert(Pointer_stringify($1));
	    js.selections.push(new);
	  }, this->id, name);
      }
      this->enter = false;
      return Selection(new_id);
    }    


    void Remove(){
      EM_ASM_ARGS({js.selections[$0].remove()}, 
		  this->id);
    }    

    bool Empty(){
      int empty = EM_ASM_INT({return Number(js.selections[$0].empty())}, 
			 this->id);
      return (bool)empty;
    }    

    int Size(){
      return EM_ASM_INT({return js.selections[$0].size()}, 
			 this->id);
    }  

    void Order(){
      EM_ASM_ARGS({js.selections[$0].order()}, 
			this->id);
    }

    void On(const char* type, const char* listener="null", bool capture=false){
   
      EM_ASM_ARGS({
	  var in_string = Pointer_stringify($2);
	  var fn = window["emp"][in_string];
	  if (typeof fn === "function"){
	    js.selections[$0].on(Pointer_stringify($1), 
		function(){
		     var new_id = js.selections.length;
		     js.selections.push(d3.select(this));
		     fn(new_id);}, $3);
	  } else {
	    if (in_string != "null"){
	      //Should probably throw a proper error
	      console.log("Warning - on called with invalid listener")
	    }
	    js.selections[$0].on(Pointer_stringify($1), null);
	  }
	  
	}, this->id, type, listener, capture);
    }

    char* GetText(){
      
      int buffer = EM_ASM_INT({
	  var text = js.selections[$0].text();
	  var buffer = Module._malloc(text.length+1);
	  Module.writeStringToMemory(text, buffer);
	  return buffer;
	}, this->id);

      return (char *)buffer;
    }


    char* GetAttrString(const char* name){
      int buffer = EM_ASM_INT({
	  var text = js.selections[$0].attr(Pointer_stringify($1));
	  var buffer = Module._malloc(text.length+1);
	  Module.writeStringToMemory(text, buffer);
	  return buffer;
	}, this->id, name);

      return (char *)buffer;
    }


    char* GetStyleString(const char* name){
      int buffer = EM_ASM_INT({
	  var text = js.selections[$0].style(Pointer_stringify($1));
	  var buffer = Module._malloc(text.length+1);
	  Module.writeStringToMemory(text, buffer);
	  return buffer;
	}, this->id, name);

      return (char *)buffer;
    }


    char* GetHtml(){
      int buffer = EM_ASM_INT({
	  var text = js.selections[$0].html();
	  var buffer = Module._malloc(text.length+1);
	  Module.writeStringToMemory(text, buffer);
	  return buffer;
	}, this->id);

      return (char *)buffer;
    }

    char* GetProperty(const char* name){
      int buffer = EM_ASM_INT({
	  var text = js.selections[$0].property(Pointer_stringify($1));
	  var buffer = Module._malloc(text.length+1);
	  Module.writeStringToMemory(text, buffer);
	  return buffer;
	}, this->id, name);

      return (char *)buffer;
    }


    //TODO:
    //
    //GetClassed()
    //SetClassed()
        
    //Filter() //requires callbacks
    //Datum() //requires callbacks
    //Sort() //requires callbacks

    //Each() //requires callbacks
    //Call() //requires callbacks
    //Node() //Is the node a selection? Do we even need this?

    //EnterCall //requires callbacks
    //EnterSelect (when would you actually do that?)
    


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

  Selection Select(const char* selector){
    /*Slightly prettier UI for constructor*/
    return Selection(selector);
  }

  Selection SelectAll(const char* selector){
    /*Slightly prettier UI for constructor*/
    return Selection(selector, true);
  }

  Selection ShapesFromData(int32_t* values, int length, const char* shape){
    Selection s = Select("svg").SelectAll(shape).Data(values, length);
    s.EnterAppend(shape);
    return s;
  }

}

