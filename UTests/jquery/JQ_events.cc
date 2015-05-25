#include <emscripten.h>

#include "../../tools/alert.h"
#include "../../emtools/init.h"
#include "../../emtools/JSWrap.h"
#include "../../jquery/JQ_events.h"

void TestAlert() {
  emp::Alert("Page loaded!!");
}

int main() {

  emp::Initialize();

  emp::OnDocumentReady(TestAlert);

}
