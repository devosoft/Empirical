#include "../../jquery/jquery.h"

int main() {

  emp::JQ::Initialize();

  int test_num = 42;

  emp::JQ::document.AddTextBox() 
    << "<h1>This is a test site!</h1>"
    << "<p>This value is " << test_num << ".</p>";

}

