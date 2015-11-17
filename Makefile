doc: build-doxygen-xml
	cd doc && make html

build-doxygen-xml:
	mkdir -p build/doxydoc/source
	./third-party/doxygen/build/bin/doxygen Doxyfile

test:
	cd tests && make test

ChangeLog: changelog/*
	cd changelog && make
	mv changelog/log ChangeLog

coverage:
	cd tests && make coverage

install-dependencies:
	cd third-party && make

clean:
	rm -rf build/*
