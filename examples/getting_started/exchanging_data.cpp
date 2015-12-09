#include <emscripten.h>
#include <iostream>

int main() {
    int val1 = 21;
    int val2 = EM_ASM_INT({ return $0 * 2; }, val1);

    std::cout << "val2 == " << val2 << std::endl;
}
