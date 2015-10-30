doc:
	mkdir -p build/doxydoc/source
	./third-party/doxygen/build/bin/doxygen Doxyfile
	cd doc && make html

install-dependencies:
	cd third-party && make

clean:
	rm -rf build/*
