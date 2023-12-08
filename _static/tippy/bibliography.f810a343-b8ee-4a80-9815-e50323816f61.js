selector_to_html = {"a[href=\"https://doi.org/10.1093/sysbio/21.2.225\"]": "\n<div>\n    <h3>\"Good\" and \"Bad\" Phenograms</h3>\n    \n    <p><b>Authors:</b> M. J. Sackin</p>\n    \n    <p><b>Publisher:</b> Oxford University Press (OUP)</p>\n    <p><b>Published:</b> 2012-11-20</p>\n</div>", "a[href=\"https://doi.org/10.1371/journal.pone.0000296\"]": "\n<div>\n    <h3>Mammals on the EDGE: Conservation Priorities Based on Threat and Phylogeny</h3>\n    \n    <p><b>Authors:</b> Nick J.B. Isaac, Samuel T. Turvey, Ben Collen, Carly Waterman, Jonathan E.M. Baillie</p>\n    \n    <p><b>Publisher:</b> Public Library of Science (PLoS)</p>\n    <p><b>Published:</b> 2007-3-13</p>\n</div>", "a[href=\"https://doi.org/10.1111/brv.12252\"]": "\n<div>\n    <h3>A guide to phylogenetic metrics for conservation, community ecology and macroecology</h3>\n    \n    <p><b>Authors:</b> Caroline M. Tucker, Marc W. Cadotte, Silvia B. Carvalho, T. Jonathan Davies, Simon Ferrier, Susanne A. Fritz, Rich Grenyer, Matthew R. Helmus, Lanna S. Jin, Arne O. Mooers, Sandrine Pavoine, Oliver Purschke, David W. Redding, Dan F. Rosauer, Marten Winter, Florent Mazel</p>\n    \n    <p><b>Publisher:</b> Wiley</p>\n    <p><b>Published:</b> 2016-1-20</p>\n</div>", "a[href=\"https://doi.org/10.1016/0006-3207(91)90030-D\"]": "\n<div>\n    <h3>What to protect?\u2014Systematics and the agony of choice</h3>\n    \n    <p><b>Authors:</b> R.I. Vane-Wright, C.J. Humphries, P.H. Williams</p>\n    \n    <p><b>Publisher:</b> Elsevier BV</p>\n    <p><b>Published:</b> 2003-9-12</p>\n</div>", "a[href=\"https://doi.org/10.2307/2992186\"]": "\n<div>\n    <h3>Tree Balance</h3>\n    \n    <p><b>Authors:</b> Kwang-Tsao Shao, Robert R. Sokal</p>\n    \n    <p><b>Publisher:</b> Oxford University Press (OUP)</p>\n    <p><b>Published:</b> 2006-5-10</p>\n</div>", "a[href=\"https://doi.org/10.1086/303378\"]": "\n<div>\n    <h3>Exploring the Phylogenetic Structure of Ecological Communities: An Example for Rain Forest Trees</h3>\n    \n    <p><b>Authors:</b> Campbell O. Webb</p>\n    \n    <p><b>Publisher:</b> University of Chicago Press</p>\n    <p><b>Published:</b> 2002-7-26</p>\n</div>", "a[href=\"https://doi.org/10.1371/journal.pone.0203401\"]": "\n<div>\n    <h3>Sound Colless-like balance indices for multifurcating trees</h3>\n    \n    <p><b>Authors:</b> Arnau Mir, Luc\u00eda Rotger, Francesc Rossell\u00f3</p>\n    \n    <p><b>Publisher:</b> Public Library of Science (PLoS)</p>\n    <p><b>Published:</b> 2018-9-25</p>\n</div>", "a[href=\"https://doi.org/10.1016/j.tree.2012.10.015\"]": "\n<div>\n    <h3>Phylogenetic diversity and nature conservation: where are we?</h3>\n    \n    <p><b>Authors:</b> Marten Winter, Vincent Devictor, Oliver Schweiger</p>\n    \n    <p><b>Publisher:</b> Elsevier BV</p>\n    <p><b>Published:</b> 2012-12-4</p>\n</div>", "a[href=\"https://doi.org/10.1016/0006-3207(92)91201-3\"]": "\n<div>\n    <h3>Conservation evaluation and phylogenetic diversity</h3>\n    \n    <p><b>Authors:</b> Daniel P. Faith</p>\n    \n    <p><b>Publisher:</b> Elsevier BV</p>\n    <p><b>Published:</b> 2003-9-11</p>\n</div>", "a[href=\"#bibliography\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Bibliography<a class=\"headerlink\" href=\"#bibliography\" title=\"Link to this heading\">\uf0c1</a></h1>"}
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
