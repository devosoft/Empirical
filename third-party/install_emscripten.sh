#wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
#gunzip emsdk-portable.tar.gz -v
#tar -xf emsdk-portable.tar -v
#cd emsdk_portable
#./emsdk update
#./emsdk install latest
#./emsdk activate latest
#source ./emsdk_env.sh

git clone https://github.com/emilydolson/emsdk_portable_repo.git
cd emsdk_portable_repo/emsdk_portable
./emsdk activate
source ./emsdk_env.sh
