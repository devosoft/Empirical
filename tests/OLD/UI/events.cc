#include <emscripten.h>

#include "../../tools/alert.h"
#include "../../web/init.h"
#include "../../UI/events.h"

void TestAlert() {
  emp::Alert("Page loaded!!");
}

int main() {

  emp::Initialize();

  emp::UI::OnDocumentReady(TestAlert);

}
