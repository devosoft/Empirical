wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
gunzip emsdk-portable.tar.gz
tar -xf emsdk-portable.tar
cd emsdk-portable
./emsdk update
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
