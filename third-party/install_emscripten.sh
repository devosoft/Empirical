#wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
#gunzip emsdk-portable.tar.gz -v
#tar -xf emsdk-portable.tar -v
#cd emsdk_portable
#./emsdk update
#./emsdk install latest
#./emsdk activate latest
#source ./emsdk_env.sh

#git clone https://github.com/emilydolson/emsdk_portable_repo.git
#cd emsdk_portable_repo/emsdk_portable
#./emsdk activate
#source ./emsdk_env.sh

# Get the emsdk repo
git clone https://github.com/emscripten-core/emsdk.git

# Enter that directory
cd emsdk

# Fetch the latest version of the emsdk (not needed the first time you clone)
git pull

# Download and install the latest SDK tools.
./emsdk install 1.38.48

# Make the "latest" SDK "active" for the current user. (writes ~/.emscripten file)
./emsdk activate 1.38.48

# Activate PATH and other environment variables in the current terminal
source ./emsdk_env.sh
