selector_to_html = {"a[href=\"#pull-request-cleanup-commit-squashing\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\"><a class=\"toc-backref\" href=\"#id7\" role=\"doc-backlink\">Pull request cleanup (commit squashing)</a><a class=\"headerlink\" href=\"#pull-request-cleanup-commit-squashing\" title=\"Link to this heading\">\uf0c1</a></h2><p>Submitters are invited to reduce the numbers of commits in their pull\nrequests either via [git rebase -i upstream/master]{.title-ref} or this\nrecipe:</p>", "a[href=\"#building-empirical-and-running-the-tests\"]": "<h3 class=\"tippy-header\" style=\"margin-top: 0;\"><a class=\"toc-backref\" href=\"#id3\" role=\"doc-backlink\">Building Empirical and running the tests</a><a class=\"headerlink\" href=\"#building-empirical-and-running-the-tests\" title=\"Link to this heading\">\uf0c1</a></h3><p>Congratulations! You\u2019re ready to develop!</p>", "a[href=\"#one-time-preparation\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\"><a class=\"toc-backref\" href=\"#id2\" role=\"doc-backlink\">One-time Preparation</a><a class=\"headerlink\" href=\"#one-time-preparation\" title=\"Link to this heading\">\uf0c1</a></h2><p>b.  Run the install-dependencies maketarget:</p>", "a[href=\"#after-your-first-issue-is-successfully-merged\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\"><a class=\"toc-backref\" href=\"#id5\" role=\"doc-backlink\">After your first issue is successfully merged\u2026</a><a class=\"headerlink\" href=\"#after-your-first-issue-is-successfully-merged\" title=\"Link to this heading\">\uf0c1</a></h2><p>You\u2019re now an experienced GitHub user! Go ahead and take some more\ntasks; you can broaden out beyond the low hanging fruit if you like.</p>", "a[href=\"#your-second-contribution\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\"><a class=\"toc-backref\" href=\"#id6\" role=\"doc-backlink\">Your second contribution\u2026</a><a class=\"headerlink\" href=\"#your-second-contribution\" title=\"Link to this heading\">\uf0c1</a></h2><p>Here are a few pointers on getting started on your second (or third, or\nfourth, or nth contribution).</p><p>So, assuming you\u2019ve found an issue you\u2019d like to work on there are a\ncouple things to do to make sure your local copy of the repository is\nready for a new issue\u2013specifically, we need to make sure it\u2019s in sync\nwith the remote repository so you aren\u2019t working on a old copy. So:</p>", "a[href=\"#claiming-an-issue-and-starting-to-develop\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\"><a class=\"toc-backref\" href=\"#id4\" role=\"doc-backlink\">Claiming an issue and starting to develop</a><a class=\"headerlink\" href=\"#claiming-an-issue-and-starting-to-develop\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"#getting-started-with-empirical-development\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\"><a class=\"toc-backref\" href=\"#id1\" role=\"doc-backlink\">Getting started with Empirical development</a><a class=\"headerlink\" href=\"#getting-started-with-empirical-development\" title=\"Link to this heading\">\uf0c1</a></h1><p>This document is intended to help those just getting started with\nEmpirical development. It details the initial one-time dependency\ninstalls and any similar routines necessary to get started with\ndevelopment.</p><p>Start by making your own copy of Empirical and setting yourself up for\ndevelopment; then, build Empirical and run the tests; and finally, claim\nan issue and start developing!</p>"}
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
