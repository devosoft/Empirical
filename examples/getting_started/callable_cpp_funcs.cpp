#include <emscripten.h>

/* NOTE: The make command for this one is different!
 * You should take a look.
 */

extern "C" {
    double SquareVal(double val) {
        return val * val;
    }
}

int main() {
    EM_ASM({ // these alternating brackets are tricky....
            SquareVal = Module.cwrap('SquareVal', 'number', ['number']);
            var x = 12.5;
            alert('Computing ' + x + ' * ' + x + ' = ' + SquareVal(x));
    });
}
