build-docs:
	mkdir -p build/doxydoc/source
	./third-party/doxygen/build/bin/doxygen Doxyfile

test:
	cd tests && make test

coverage:
	cd tests && make coverage

clean:
	rm -rf build/*
