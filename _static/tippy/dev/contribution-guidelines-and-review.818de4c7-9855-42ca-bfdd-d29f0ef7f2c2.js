selector_to_html = {"a[href=\"#c-standards\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">C++ standards<a class=\"headerlink\" href=\"#c-standards\" title=\"Link to this heading\">\uf0c1</a></h2><p>We use C++17 features throughout the project and currently that is the\nde-facto standard version to use.</p><p>All code should be in header files for ease of inclusion into Emscripten\nprojects.</p>", "a[href=\"#coding-guidelines-and-review-checklist\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Coding guidelines and review checklist<a class=\"headerlink\" href=\"#coding-guidelines-and-review-checklist\" title=\"Link to this heading\">\uf0c1</a></h1><p>This document is for those who want to contribute code or documentation\nfixes to the Empirical project and describes our coding standards as\nwell as our code review process.</p><p>This document has been adapted from the <a class=\"reference external\" href=\"https://khmer.readthedocs.org/en/v1.4.1/dev/coding-guidelines-and-review.html\">khmer\nproject</a></p>", "a[href=\"#general-standards\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">General Standards<a class=\"headerlink\" href=\"#general-standards\" title=\"Link to this heading\">\uf0c1</a></h2><p>All plain-text files should have line widths of 100 characters or less\nunless that is unsupported for the particular file format or creates a\nmajor loss in readability.</p><p>All contributions should have their spelling checked before being\ncommitted to the codebase.  For example, the VSCode plug-in\n\u201cCode Spell Checker\u201d is a good choice.</p>", "a[href=\"#guidelines-based-on-emscripten-limitations\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Guidelines based on Emscripten Limitations<a class=\"headerlink\" href=\"#guidelines-based-on-emscripten-limitations\" title=\"Link to this heading\">\uf0c1</a></h2><p>Please see the <a class=\"reference external\" href=\"https://kripken.github.io/emscripten-site/docs/porting/guidelines/portability_guidelines.html\">Emscripten doc\npage</a>\nfor a full list.</p>", "a[href=\"#commenting-in-files\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Commenting in files<a class=\"headerlink\" href=\"#commenting-in-files\" title=\"Link to this heading\">\uf0c1</a></h2><p>All code should be well-commented such that it can be understood by a skilled\nC++ programmer that is not familiar with the code base.  Comments should be\nwritten in Doxygen format where appropriate.</p><p>Each file should have a header at the top to describe the goals of that file.\nThis header should include copyright information as well as the name of the file,\na brief description, and its status.  Statuses include:</p>", "a[href=\"#checklist\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Checklist<a class=\"headerlink\" href=\"#checklist\" title=\"Link to this heading\">\uf0c1</a></h2><p>To submit code to Empirical, open a Pull Request on Github. When you are ready for it to be reviewed, add the \u201cMerge Ready\u201d label. Before requesting a review, you should ensure that all checks on Github pass and confirm that the following automatically-checkable things are true:</p>"}
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
