build-dependencies: download-doxygen build-doxygen

download-doxygen:
	bash download_doxygen.sh

build-doxygen: download-doxygen
	mkdir -p doxygen/build && cd doxygen/build && cmake -g "Unix makefiles" .. && make

install-sphinx:
	bash ./install_sphinx.sh

clean:
	rm -rf doxygen/build/*
