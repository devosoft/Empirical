#include <emscripten.h>
#include <iostream>
#include <vector>
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
    Selection(int id); //append constructor

  public:
    Selection(const char* selector, bool all = false);
    //Selection(Selection* selector, bool all = false); //Do we need this?
    //~Selection(); //tilde is creating demangling issues
    
    
    template <typename T>
    void SetAttrNumeric(const char* name, T value){
      /* Assigns [value] to the selections='s [name] attribute.
	 This method handles numeric values - use SetAttrString
	 for non-numeric values. */

      //TODO: Make sure the user actually uses a number
      EM_ASM_ARGS({js.selections[$0].attr(Pointer_stringify($1), $2)},
		  this->id, name, value);
    }

    void SetAttrString(const char* name, const char* value){
      /* Assigns [value] to the selections='s [name] attribute.
	 This method handles numeric values - use SetAttrString
	 for non-numeric values. */

      //TODO: Make sure the user actually uses a number
      EM_ASM_ARGS({js.selections[$0].attr(Pointer_stringify($1), 
		 Pointer_stringify($2))}, this->id, name, value);
    }

    Selection Append(const char* name){
      int new_id = EM_ASM_INT_V({return js.selections.length});
      EM_ASM_ARGS({
	  var new_selection = js.selections[$0].append(Pointer_stringify($1));
	  js.selections.push(new_selection);
	}, this->id, name);
      return Selection(new_id);
    }

    void SetStyleString(const char* name, const char* value, bool priority=false){
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
    void SetStyleNumeric(const char* name, T value, bool priority=false){
      if (priority){
	EM_ASM_ARGS({js.selections[$0].style(Pointer_stringify($1), 
	$2, "important")}, this->id, name, value);
      }
      else {
	EM_ASM_ARGS({js.selections[$0].style(Pointer_stringify($1), 
	$2)}, this->id, name, value);
      }
    }
    
    template <typename C>
    Selection Data(C values){
      int new_id = EM_ASM_INT_V({return js.selections.length});
      EM_ASM_ARGS({
	  js.selections.push(js.selections[$0].data([1,2,3]))}, 
	  this->id, values);
      return Selection(new_id);
    }

    //TODO:
    //T GetAttrNumeric()
    //GetAttrString

    //GetClassed()
    //SetClassed()
    //GetStyle()
    //GetProperty()
    //SetProperty()
    //GetText()
    //SetText()
    //SetHtml()
    //GetHtml()
    //Insert()
    //Remove()
    //Data() //this one's a biggie
    //Enter() //also pretty important
    //Exit() //this one too
    //Filter()
    //Datum()
    //Sort()
    //Order()
    //On() //also notable
    //Transition()
    //Interrupt()
    //Subselection
    //Each()
    //Call()
    //Empty()
    //Node()
    //Size()
    


  };

  Selection::Selection(const char* selector, bool all){
    this->id = EM_ASM_INT_V({return js.selections.length});
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
};

int main()
{
  std::cout << n_selections() << std::endl;
  D3::Selection s = D3::Selection("body");
  s.SetAttrNumeric("test", 4);
  D3::Selection s2 = s.Append("svg");
  D3::Selection update = s2.Data(2);
  update.SetAttrNumeric("cx", 25);
  update.SetAttrNumeric("cy", 25);
  update.SetAttrNumeric("r", 25);
  update.SetStyleString("fill", "purple");
  EM_ASM({js.selections[js.selections.length-1] = js.selections[js.selections.length-1].enter().append("circle")});
  update.SetAttrNumeric("cx", 25);
  update.SetAttrNumeric("cy", 25);
  update.SetAttrNumeric("r", 25);
  update.SetStyleString("fill", "purple");

  
}
