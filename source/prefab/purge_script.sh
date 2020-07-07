#! /bin/bash

# We use js-Delivr as a content delivery network to serve the static files for prefab web tools
# After updating DefaultConfigPanelStyle.css or LoadingModal.js, run this script to see your 
# changes take effect in web application (This may take a few minutes)

# You may also purge the cache for just one file by going to the appropiate URL
# http://purge[dot]jsdelivr[dot]net/gh/devosoft/Empirical@prefab/source/prefab/DefaultConfigPanelStyle.css
# http://purge[dot]jsdelivr[dot]net/gh/devosoft/Empirical@prefab/source/prefab/LoadingModal.js

# If you have questions about using js-Delivr's purging API, 
# contact Dmitry
# dak[at]prospectone[dot]io

# Note: replace [at] with '@' and [dot] with '.'

curl -X POST http://purge.jsdelivr.net \
-H 'cache-control: no-cache' \
-H 'content-type: application/json' \
-d'{
"path":[
"/gh/devosoft/Empirical@prefab/source/prefab/DefaultConfigPanelStyle.css",
"/gh/devosoft/Empirical@prefab/source/prefab/LoadingModal.js"
]
}'


