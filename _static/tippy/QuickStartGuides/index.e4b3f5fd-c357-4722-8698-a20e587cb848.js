selector_to_html = {"a[href=\"4-UsingProjectTemplate.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Using the Project Template<a class=\"headerlink\" href=\"#using-the-project-template\" title=\"Link to this heading\">\uf0c1</a></h1><p>Goals of the project template - Simple web interface - Easy toggle -\nEasy debug</p><p>Organizing core code files</p>", "a[href=\"0-Overview.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Overview<a class=\"headerlink\" href=\"#overview\" title=\"Link to this heading\">\uf0c1</a></h1><p>The goal of Empirical is to simplify the development of efficient,\nreliable, and accessible scientific software.</p><p>We have several quick-start guides for different portions of the code.</p>", "a[href=\"#quick-start-guides\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Quick Start Guides<a class=\"headerlink\" href=\"#quick-start-guides\" title=\"Link to this heading\">\uf0c1</a></h1><p>Contents:</p>", "a[href=\"todos/todo.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Work In Progress<a class=\"headerlink\" href=\"#work-in-progress\" title=\"Link to this heading\">\uf0c1</a></h1><p>The following pages are a work in progress \ud83d\udea7.\n<a class=\"reference internal\" href=\"../dev/contribution-guidelines-and-review.html\"><span class=\"doc std std-doc\">Contributions</span></a> are welcome!</p>", "a[href=\"3-WebTools.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Web Tools<a class=\"headerlink\" href=\"#web-tools\" title=\"Link to this heading\">\uf0c1</a></h1><p>The web tools in Empirical are structured to allow the developer to\nfully control components of a web page from C++. Note that you must have\nthe emscripten compiler installed for web utilities to function\nproperly.</p><p>Empirical web Widgets include Text, Buttons, Images, Tables, or many\nother HTML components. All widgets are derived from <code class=\"docutils literal notranslate\"><span class=\"pre\">emp::Widget</span></code> and\nstructured such that multiple widgets can properly refer to and modify\nthe same component.</p>", "a[href=\"1-HelloWorld.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Hello World<a class=\"headerlink\" href=\"#hello-world\" title=\"Link to this heading\">\uf0c1</a></h1><p>Empirical allows you to compile your C++ to target</p>", "a[href=\"2-BaseTools.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Base Tools<a class=\"headerlink\" href=\"#base-tools\" title=\"Link to this heading\">\uf0c1</a></h1><p>A handful of tools are available in the include/emp/base/ folder in\nEmpirical. These mimic basic functionality available in C++ or its\nstandard library, but provide extra protection against common memory use\nerrors and additional information to the developer.  These protections\ncan be turned off by comiling with -DNDEBUG.</p>"}
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
