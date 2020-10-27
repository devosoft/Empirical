#include "web_UI.h"

#include <string>

VM_UI_base * UI;

extern "C" int empMain()
{
  UI = new TubeIC_UI();  // Only build once at the very beginning.

  return 0;
}


extern "C" int empLoadString(char * _string)
{
  // Parse the input code (which will automatically load it into the main hardware.
  ParseString(_string);

  return 0;
}
