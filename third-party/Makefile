install-dependencies: build-doxygen install-pip-deps

download-doxygen:
	bash download_doxygen.sh

build-doxygen: download-doxygen
	mkdir -p doxygen/build && cd doxygen/build && cmake -g "Unix makefiles" .. && make

install-pip-deps:
	bash ./install_pip_deps.sh

clean:
	rm -rf doxygen/build/*
