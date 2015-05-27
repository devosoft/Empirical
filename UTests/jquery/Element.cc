#include "../../jquery/jquery.h"

int main() {

  emp::JQ::Initialize();

  int test_num = 42;

  emp::JQ::document
    << "<h1>This is a test site!</h1>"
    << "<p>The value of test_num is " << test_num << ".</p>";

  emp::JQ::document.Update();
}

