#include <emscripten.h>
#include <string>

// Function to trigger alerts straight from C++
//

void Alert(const std::string & msg) {
    EM_ASM_ARGS({
        var msg = Pointer_stringify($0); // convert message to JS string
        alert(msg)
    }, msg.c_str());
}

int main() {
    Alert("Hello from C++!");
}
