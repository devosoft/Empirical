// This file is a monstrosity, because including multiple emscripten-generated
// Javascript files in karma will cause horrible problems.


#include "web/Document.h"
#include "web/Element.h"
#include "web/web.h"

#include <iostream>
#include <functional>

emp::web::Document doc("emp_base");

void do_wrapping() {
  emp::web::Element parent("p", "parent");
  parent << "parent";

  doc << parent;

  emp::web::Button child(
    [](){ std::cout << "click" << std::endl; },
    "child"
  );

  parent << child;

  emp::web::Div wrapper("wrapper");
  wrapper << "wrapper";

  child.WrapWith(wrapper);

  // secondary wrapping test
  parent.WrapWith(
    emp::web::Div("wrapper2").SetCSS("background-color", "red")
  ).SetCSS("background-color", "blue");

  doc.Div("wrapper2") << "<br/><br/>";

}

int main() {
  do_wrapping();
}
