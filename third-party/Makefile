travis-install-dependencies: build-doxygen
	sudo pip install -r requirements.txt

install-dependencies: build-doxygen install-pip-deps


download-doxygen:
	bash download_doxygen.sh

build-doxygen: download-doxygen
	mkdir -p doxygen/build && cd doxygen/build && cmake -g "Unix makefiles" .. && make

prepare-virtualenv:
	python -m virtualenv env

install-pip-deps: prepare-virtualenv
	bash ./install_pip_deps.sh

clean:
	rm -rf doxygen/build/*
	rm -rf env
