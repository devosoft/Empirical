install-doc-dependencies: build-doxygen install-pip-deps
	git submodule update --init

install-test-dependencies: install-pip-deps install-npm-deps
	bash ./install_emscripten.sh
	git submodule update --init

install-coverage-dependencies:
	git submodule update --init
	cd force-cover && make

install-dependencies: install-doc-dependencies install-test-dependencies install-coverage-dependencies

download-doxygen:
	bash download_doxygen.sh

build-doxygen: download-doxygen
	mkdir -p doxygen/build && cd doxygen/build && cmake -g "Unix makefiles" .. && make

prepare-virtualenv:
	python -m virtualenv env

install-pip-deps: prepare-virtualenv
	bash ./install_pip_deps.sh

install-npm-deps: 
	npm install

clean:
	rm -rf doxygen/build/*
	rm -rf env
