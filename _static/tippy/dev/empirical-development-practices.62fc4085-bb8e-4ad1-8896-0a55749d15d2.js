selector_to_html = {"a[href=\"#empirical-development-practices\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\"><a class=\"toc-backref\" href=\"#id1\" role=\"doc-backlink\">Empirical Development Practices</a><a class=\"headerlink\" href=\"#empirical-development-practices\" title=\"Link to this heading\">\uf0c1</a></h1><p>Although the Empirical project is primarily maintained by the Primary Investigator of the Digital Evolution laboratory at Michigan State University, Dr. Charles Ofria, a large, rotating menagerie of graduate students, undergraduate students, alumni, and collaborators also regularly contribute to the library.\nSeveral active research projects depend on the library.\nThe scope of the project and the continuous turnover of our developer base have necessitated investment in extensive social and technical tooling on our part.</p><p>Our development process revolves around a standard fork and pull request workflow.\nWe use GitHub actions for continuous integration, ensuring that code merged in to our main branch meets certain quality-control criteria.\nWe check that merged code does not break existing unit tests or any demonstration code bundled with the repository.\nWe maintain an extensive unit testing suite, which, as of June 2021, covers 82% of our code base.\nWe use Codecov to measure code coverage as part of our continuous integration.\nThis allows us to enforce that merged code provides unit tests for any new content.\nIn addition to automated quality checks, we enforce manual code reviews on all pull requests.</p>"}
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
