build-docs:
	mkdir -p build/doxydoc/source
	./third-party/doxygen/build/bin/doxygen Doxyfile

clean:
	rm -rf build/*
