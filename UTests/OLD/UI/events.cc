#include <emscripten.h>

#include "../../tools/alert.h"
#include "../../emtools/init.h"
#include "../../UI/events.h"

void TestAlert() {
  emp::Alert("Page loaded!!");
}

int main() {

  emp::Initialize();

  emp::UI::OnDocumentReady(TestAlert);

}
