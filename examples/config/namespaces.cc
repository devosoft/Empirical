#include <iostream>

#include "../../source/config/config.h"

EMP_BUILD_CONFIG( MyConfig,
  GROUP(DEFAULT_GROUP, "General Settings"),
  VALUE(DEBUG_MODE, bool, false, "Should we output debug information?"),
  VALUE(RANDOM_SEED, int, 0, "Random number seed (0 for based on time)"),

  GROUP(TEST_GROUP, "These are settings with the sole purpose of testing cConfig.\nFor example, are multi-line descriptions okay?"),
  VALUE(TEST_BOOL, bool, false, "This is a bool value.\nWhat happens\n  ...if we have multiple\n    lines?"),
  VALUE(TEST_STRING, std::string, "default", "This is a string!"),
  CONST(TEST_CONST, int, 91, "This is an unchanging const!"),
  VALUE(TEST_STRING_SPACE, std::string, "abc def   ghi", "This is a string with spaces."),
  VALUE(TEST_DUP, int, 20, "This is a test of the same name in multiple namespaces.")	  
)

EMP_BUILD_CONFIG( MyConfig_internal,
  GROUP(DEFAULT_GROUP, "BASIC SETTINGS"),
  VALUE(TEST_INT1, int, 1, "This is my first integer test."),
  VALUE(TEST_INT2, int, 2, "This is my second integer test."),
  VALUE(TEST_DUP, int, 3333, "This is a test of the same name in multiple namespaces.")	  
)

int main()
{
  MyConfig config;
  MyConfig_internal config2;
  config.AddNameSpace(config2, "internal");
  config.Read("namespaces.cfg");
  //config.Write("namespaces.cfg");
  //config.WriteMacros("test-macro.h");
  std::cout << "We are in namespaces!" << std::endl;

  std::cout << "Config values:";
  std::cout << "  config.DEBUG_MODE() = " << config.DEBUG_MODE() << std::endl;
  std::cout << "  config.RANDOM_SEED() = " << config.RANDOM_SEED() << std::endl;
  std::cout << "  config.TEST_BOOL() = " << config.TEST_BOOL() << std::endl;
  std::cout << "  config.TEST_STRING() = " << config.TEST_STRING() << std::endl;
  std::cout << "  config.TEST_CONST() = " << config.TEST_CONST() << std::endl;
  std::cout << "  config.TEST_STRING_SPACE() = " << config.TEST_STRING_SPACE() << std::endl;
  std::cout << "  config.TEST_DUP() = " << config.TEST_DUP() << std::endl;

  std::cout << "\nConfig2 values:";
  std::cout << "  config2.TEST_INT1() = " << config2.TEST_INT1() << std::endl;
  std::cout << "  config2.TEST_INT2() = " << config2.TEST_INT2() << std::endl;
  std::cout << "  config2.TEST_DUP() = " << config2.TEST_DUP() << std::endl;
}
