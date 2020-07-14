#ifndef EMP_CODE_BLOCK_H
#define EMP_CODE_BLOCK_H

#include "../web/Element.h"
#include "../web/Widget.h"
#include "../tools/string_utils.h"
#include "../base/errors.h"

namespace emp {
    namespace prefab {
        class CodeBlock: public web::Element {
            private:
                web::Element code{emp::to_string("code")};
            public:
                CodeBlock(const std::string code_str, const std::string lang, const std::string & in_name=""): web::Element("pre", in_name){
                    this->SetAttr("class", lang);
                    this->DoUpdateJS([](){
                        emscripten_run_script("hljs.initHighlighting.called = false; hljs.initHighlighting();");
                    });
                    code << emp::to_web_safe_string(code_str);
                    *this << code;
                    }
                // TODO: << operator throw error
                // template <typename T>
                // void operator<<(T invalid){
                //     emp::LibraryError("Not allowed to add code to the code block after construction due to JavaScript callback order");
                // }
        };
    }
}

#endif