selector_to_html = {"a[href=\"#how-to-add-docs-to-the-sphinx-documentation\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">How to add docs to the Sphinx documentation<a class=\"headerlink\" href=\"#how-to-add-docs-to-the-sphinx-documentation\" title=\"Link to this heading\">\uf0c1</a></h2><p>Sphinx is the system used to generate the developer guide and similar\nreference documentation. A primer to using ReStructured Text, the markup\nlanguage used by Sphinx, can be found\n<a class=\"reference external\" href=\"http://docutils.sourceforge.net/docs/user/rst/quickstart.html\">here</a>.\nYou can also look at any of the [.rst]{.title-ref} files in the\n[doc/]{.title-ref} directory to get a feel for how thinks work.</p><p>New documents must be included in the <code class=\"docutils literal notranslate\"><span class=\"pre\">toctree</span></code> in the <code class=\"docutils literal notranslate\"><span class=\"pre\">index.md</span></code>\nfile for the directory the added file lives in. For example, if you add\n<code class=\"docutils literal notranslate\"><span class=\"pre\">CowFacts.md</span></code> to the <code class=\"docutils literal notranslate\"><span class=\"pre\">CoolFacts/</span></code> directory you must add <code class=\"docutils literal notranslate\"><span class=\"pre\">CowFacts.md</span></code>\nto the toctree found in <code class=\"docutils literal notranslate\"><span class=\"pre\">CoolFacts/CowFacts.md</span></code>:</p>", "a[href=\"#how-to-include-doxygen-s-autodocs-within-sphinx-files\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">How to include Doxygen\u2019s autodocs within Sphinx files<a class=\"headerlink\" href=\"#how-to-include-doxygen-s-autodocs-within-sphinx-files\" title=\"Link to this heading\">\uf0c1</a></h2><p>Through the use of the Breathe extension it is incredibly easy to\ninclude Doxygen autodocs within a Sphinx documentation file.</p><p>Suppose we have a C++ implementation file name <code class=\"docutils literal notranslate\"><span class=\"pre\">potato.h</span></code> that has\ninline comment documentation as detailed above and that <code class=\"docutils literal notranslate\"><span class=\"pre\">potato.h</span></code> is a\ncomponent of a module named <code class=\"docutils literal notranslate\"><span class=\"pre\">ingredients</span></code> that was just created.</p>", "a[href=\"#how-to-comment-for-doxygen-autodoc\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">How to Comment for Doxygen Autodoc<a class=\"headerlink\" href=\"#how-to-comment-for-doxygen-autodoc\" title=\"Link to this heading\">\uf0c1</a></h2><p>Doxygen has an entire <a class=\"reference external\" href=\"https://www.stack.nl/~dimitri/doxygen/manual/docblocks.html\">documentation\nsection</a> on\nhow to comment your code. We\u2019ll provide a trimmed version here so that\nyou can get started quickly.</p><p>Doxygen will examine all comments to determine if they are documentation\ncomments or just code comments. To make a documentation comment you must\nadd either an extra * or /, depending on the kind of comment:</p>", "a[href=\"#empirical-documentation-documentation\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Empirical Documentation Documentation<a class=\"headerlink\" href=\"#empirical-documentation-documentation\" title=\"Link to this heading\">\uf0c1</a></h1><p>This is a quick primer on how to document things within Empirical.</p><p>Empirical makes use of the Sphinx documentation system based off of XML\ninformation gathered from Doxygen via a plugin named Breathe. This means\nthat Doxygen will automatically build documentation for anything written\nin a C++ source file and Sphinx will be used to organize how that\ndocumentation is displayed.</p>"}
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
