selector_to_html = {"a[href=\"api/Empower.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Empower.hpp<a class=\"headerlink\" href=\"#empower-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"#empower\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Empower<a class=\"headerlink\" href=\"#empower\" title=\"Link to this heading\">\uf0c1</a></h1><h2>API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>", "a[href=\"api/MemoryImage.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">MemoryImage.hpp<a class=\"headerlink\" href=\"#memoryimage-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/StructType.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">StructType.hpp<a class=\"headerlink\" href=\"#structtype-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/Type.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Type.hpp<a class=\"headerlink\" href=\"#type-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/Struct.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Struct.hpp<a class=\"headerlink\" href=\"#struct-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/TypeManager.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">TypeManager.hpp<a class=\"headerlink\" href=\"#typemanager-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/Var.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">Var.hpp<a class=\"headerlink\" href=\"#var-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"api/VarInfo.html\"]": "<h1 class=\"tippy-header\" style=\"margin-top: 0;\">VarInfo.hpp<a class=\"headerlink\" href=\"#varinfo-hpp\" title=\"Link to this heading\">\uf0c1</a></h1>", "a[href=\"#api\"]": "<h2 class=\"tippy-header\" style=\"margin-top: 0;\">API<a class=\"headerlink\" href=\"#api\" title=\"Link to this heading\">\uf0c1</a></h2>"}
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
