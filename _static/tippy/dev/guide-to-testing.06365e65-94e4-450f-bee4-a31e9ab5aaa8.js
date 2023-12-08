selector_to_html = {"a[href=\"#running-tests-with-docker\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Running Tests with Docker<a class=\"headerlink\" href=\"#running-tests-with-docker\" title=\"Link to this heading\">\uf0c1</a></h2><p>A <a class=\"reference external\" href=\"https://hub.docker.com/r/devosoft/empirical\">devosoft/empirical</a> Docker image has been set up\nto make recreating a development environment on your machine easier.\nThe first step is to download Docker. <a class=\"reference external\" href=\"https://docs.docker.com/get-docker/\">https://docs.docker.com/get-docker/</a></p><p>To download and run the Docker image, enter the following commands in the Docker terminal</p>", "a[href=\"#guide-to-testing-in-empirical\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Guide to Testing in Empirical<a class=\"headerlink\" href=\"#guide-to-testing-in-empirical\" title=\"Link to this heading\">\uf0c1</a></h1><p>This document details how testing works in Empirical, both for writing\nand understanding tests. Empirical makes use of the <a class=\"reference external\" href=\"https://github.com/philsquared/Catch\">Catch testing\nframework</a>, the documentation of\nwhich is available\n<a class=\"reference external\" href=\"https://github.com/philsquared/Catch/blob/master/docs/Readme.md\">here</a>.</p>", "a[href=\"#writing-tests\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Writing Tests<a class=\"headerlink\" href=\"#writing-tests\" title=\"Link to this heading\">\uf0c1</a></h2><p>It is required that contributions to the Empirical library have test\ncoverage. Though writing tests can be a complex task in some cases the\nCatch testing framework is extremely easy to use.</p><p>In general the best way to understand how to write tests is to look at\nthe existing tests. I recommend skimming through <code class=\"docutils literal notranslate\"><span class=\"pre\">test_tools.cc</span></code> for an\noverview.</p>", "a[href=\"#running-tests\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Running Tests<a class=\"headerlink\" href=\"#running-tests\" title=\"Link to this heading\">\uf0c1</a></h2><p>In the root directory of Empirical, use the maketarget <code class=\"docutils literal notranslate\"><span class=\"pre\">test</span></code>, like so:</p>", "a[href=\"#tidyness-enforcement\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Tidyness Enforcement<a class=\"headerlink\" href=\"#tidyness-enforcement\" title=\"Link to this heading\">\uf0c1</a></h2><p>As part of our continuous integration, we test for several tidyness violations, including</p>"}
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
