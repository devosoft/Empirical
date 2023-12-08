selector_to_html = {"a[href=\"#selections-and-transitions-selections-and-transitions-api\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Selections and Transitions {#Selections_and_Transitions_API}<a class=\"headerlink\" href=\"#selections-and-transitions-selections-and-transitions-api\" title=\"Link to this heading\">\uf0c1</a></h2><p>::: {.doxygenfile project=\u201dEmpirical\u201d}\nemp/web/d3/selection.hpp\n:::</p>", "a[href=\"#scales-scales-api\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Scales {#Scales_API}<a class=\"headerlink\" href=\"#scales-scales-api\" title=\"Link to this heading\">\uf0c1</a></h2><p>::: {.doxygenfile project=\u201dEmpirical\u201d}\nemp/web/d3/scales.hpp\n:::</p>", "a[href=\"#svg-shapes-and-paths-svg-shapes-api\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">SVG Shapes and Paths {#SVG_Shapes_API}<a class=\"headerlink\" href=\"#svg-shapes-and-paths-svg-shapes-api\" title=\"Link to this heading\">\uf0c1</a></h2><p>::: {.doxygenfile project=\u201dEmpirical\u201d}\nemp/web/d3/svg_shapes.hpp\n:::</p>", "a[href=\"#basic-d3-objects-d3-base\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Basic D3 Objects {#D3_Base}<a class=\"headerlink\" href=\"#basic-d3-objects-d3-base\" title=\"Link to this heading\">\uf0c1</a></h2><p>::: {.doxygenfile project=\u201dEmpirical\u201d}\nemp/web/d3/d3_init.hpp\n:::</p>", "a[href=\"#d3-wrapper-api\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">D3 Wrapper API<a class=\"headerlink\" href=\"#d3-wrapper-api\" title=\"Link to this heading\">\uf0c1</a></h1><h2>Basic D3 Objects {#D3_Base}<a class=\"headerlink\" href=\"#basic-d3-objects-d3-base\" title=\"Link to this heading\">\uf0c1</a></h2><p>::: {.doxygenfile project=\u201dEmpirical\u201d}\nemp/web/d3/d3_init.hpp\n:::</p>", "a[href=\"#axes-axes-api\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Axes {#Axes_API}<a class=\"headerlink\" href=\"#axes-axes-api\" title=\"Link to this heading\">\uf0c1</a></h2><p>::: {.doxygenfile project=\u201dEmpirical\u201d}\nemp/web/d3/axis.hpp\n:::</p>", "a[href=\"#datasets-dataset-api\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">Datasets {#Dataset_API}<a class=\"headerlink\" href=\"#datasets-dataset-api\" title=\"Link to this heading\">\uf0c1</a></h2><p>::: {.doxygenfile project=\u201dEmpirical\u201d}\nemp/web/d3/svg_shapes.hpp\n:::</p>"}
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
