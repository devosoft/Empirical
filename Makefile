test: test-native test-examples test-web
	make test-native
	make test-examples
	make test-web

test-examples: test-native-examples test-web-examples

test-native: test-native-regular test-native-fulldebug test-native-opt test-native-examples

test-native-regular:
	cd tests && make test

test-native-fulldebug:
	cd tests && make fulldebug

test-native-opt:
	cd tests && make opt

test-native-examples:
	cd examples && make native-test
	cd examples && make native-test-debug

test-web-examples:
	cd examples && make web-test

test-web:
	cd tests && make test-web

../cookiecutter-empirical-project:
	cd .. && git clone --recursive https://github.com/devosoft/cookiecutter-empirical-project.git

test-cookiecutter: ../cookiecutter-empirical-project
	. third-party/env/bin/activate && \
	export COOKIECUTTER_EMP_DIR=../../Empirical/source && \
	cd ../cookiecutter-empirical-project && make clean && make test

doc: build-doxygen-xml
	cd doc && ./make_docs.sh

coverage:
	cd tests && make coverage

build-doxygen-xml:
	./third-party/doxygen/build/bin/doxygen Doxyfile

install-dependencies:
	git submodule init
	git submodule update
	cd third-party && make

install-doc-dependencies:
	git submodule init
	git submodule update
	cd third-party && make install-doc-dependencies

install-test-dependencies:
	cd third-party && make install-test-dependencies

install-coverage-dependencies:
	cd third-party && make install-coverage-dependencies

clean:
	rm -rf build/*
	cd tests && make clean
clean-dep:
	cd third-party && make clean
