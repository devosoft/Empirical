/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020
 *
 *  @file Modal.cpp
 */

#include <iostream>

#include "emp/prefab/Modal.hpp"
#include "emp/web/Button.hpp"
#include "emp/web/web.hpp"

namespace UI = emp::web;

UI::Document doc("emp_base");
int main(){
  emp::prefab::Modal modal;
  doc << modal;

  modal.AddHeaderContent("<h3>Modal Header Section</h3>");
  modal.AddBodyContent("This is the content of the modal");

  modal.AddFooterContent("Modal Footer Section");
  UI::Button close_btn([](){;}, "Close");
  close_btn.SetAttr("class", "btn btn-secondary");
  modal.AddFooterContent(close_btn);
  modal.AddButton(close_btn);

  modal.AddClosingX();

  UI::Button modal_btn([](){;}, "Show Modal");
  doc << modal_btn;
  modal_btn.SetAttr("class", "btn btn-info");
  modal.AddButton(modal_btn);

  std::cout << "end of main... !" << std::endl;

}
