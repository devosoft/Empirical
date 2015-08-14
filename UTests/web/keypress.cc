#include "../../web/web.h"

namespace web = emp::web;
web::Document doc("emp_base");

int main()
{
  doc << "Testing!";
}
