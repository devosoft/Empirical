selector_to_html = {"a[href=\"#base-array-h-and-base-vector-h\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">base/array.h and base/vector.h<a class=\"headerlink\" href=\"#base-array-h-and-base-vector-h\" title=\"Link to this heading\">\uf0c1</a></h2><p>These files setup the <code class=\"docutils literal notranslate\"><span class=\"pre\">emp::array&lt;...&gt;</span></code> and <code class=\"docutils literal notranslate\"><span class=\"pre\">emp::vector&lt;...&gt;</span></code> template\nobjects, which behave almost identically to <code class=\"docutils literal notranslate\"><span class=\"pre\">std::array&lt;...&gt;</span></code> and\n<code class=\"docutils literal notranslate\"><span class=\"pre\">std::vector&lt;...&gt;</span></code>, respectively. The one difference is that they do\nbounds checking when they are indexed into or specific size matters. As\nwith asserts, these additional bounds checks are removed when compiled\nwith the <code class=\"docutils literal notranslate\"><span class=\"pre\">NDEBUG</span></code> option.</p>", "a[href=\"#base-tools\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Base Tools<a class=\"headerlink\" href=\"#base-tools\" title=\"Link to this heading\">\uf0c1</a></h1><p>A handful of tools are available in the include/emp/base/ folder in\nEmpirical. These mimic basic functionality available in C++ or its\nstandard library, but provide extra protection against common memory use\nerrors and additional information to the developer.  These protections\ncan be turned off by comiling with -DNDEBUG.</p>", "a[href=\"#base-ptr-h\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">base/Ptr.h<a class=\"headerlink\" href=\"#base-ptr-h\" title=\"Link to this heading\">\uf0c1</a></h2><p>The <code class=\"docutils literal notranslate\"><span class=\"pre\">emp::Ptr&lt;...&gt;</span></code> template provides an alternate method of building\npointers, but with the ability to turn on additional debugging\nfacilities; unlike assert, array, and vector, no debugging is performed\nby default due to substantial run-time costs. For example declaring a\nvariable as <code class=\"docutils literal notranslate\"><span class=\"pre\">emp::Ptr&lt;int&gt;</span></code> is the same as declaring it <code class=\"docutils literal notranslate\"><span class=\"pre\">int</span> <span class=\"pre\">*</span></code>.</p><p>If the <code class=\"docutils literal notranslate\"><span class=\"pre\">EMP_TRACK_MEM</span></code> option is set (<code class=\"docutils literal notranslate\"><span class=\"pre\">-DEMP_TRACK_MEM</span></code> compiler flag)\nthen all pointer usage is tracked and many simple memory errors will be\nidentified during execution, such as using or deleting an unallocated\npointer.</p>", "a[href=\"#base-assert-h\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">base/assert.h<a class=\"headerlink\" href=\"#base-assert-h\" title=\"Link to this heading\">\uf0c1</a></h2><p>This file adds an <code class=\"docutils literal notranslate\"><span class=\"pre\">emp_assert</span></code> macro that can handle all of the same\nfunctionality as the standard library assert, but with additional\nfeatures. Specifically, additional arguments may be added that are\nprinted when the assert is triggered. For example, the line</p>"}
skip_classes = ["headerlink", "sd-stretched-link"]

window.onload = function () {
    for (const [select, tip_html] of Object.entries(selector_to_html)) {
        const links = document.querySelectorAll(` ${select}`);
        for (const link of links) {
            if (skip_classes.some(c => link.classList.contains(c))) {
                continue;
            }

            tippy(link, {
                content: tip_html,
                allowHTML: true,
                arrow: true,
                placement: 'auto-start', maxWidth: 500, interactive: false,

            });
        };
    };
    console.log("tippy tips loaded!");
};
