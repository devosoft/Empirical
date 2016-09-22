doc: build-doxygen-xml
	cd doc && make html

build-doxygen-xml:
	mkdir -p build/doxydoc/source
	./third-party/doxygen/build/bin/doxygen Doxyfile

travis: 
	cd third-party && bash install_emscripten.sh
	make test

test:
	cd tests && make test

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
