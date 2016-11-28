install-dependencies: build-doxygen install-pip-deps install-npm-deps
	git submodule update --init

travis-install-dependencies: build-doxygen install-npm-deps
	sudo pip install -r requirements.txt

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
