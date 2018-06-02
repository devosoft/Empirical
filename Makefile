test:
	cd tests && make test
	cd tests && make fulldebug
	cd tests && make opt
	cd tests && make test-web

doc: build-doxygen-xml
	cd doc && ./make_docs.sh

build-doxygen-xml:
	./third-party/doxygen/build/bin/doxygen Doxyfile

travis: 
	make install-dependencies
	cd third-party && bash install_emscripten.sh
	cd third-party && make install-npm-deps
	make test
	make doc

install-dependencies:
	git submodule init
	git submodule update
	cd third-party && make

install-testing-dependencies:
	cd third-party && make install-testing-dependencies

clean:
	rm -rf build/*
	cd tests && make clean
clean-dep:
	cd third-party && make clean
