doc: build-doxygen-xml
	cd doc && make html

build-doxygen-xml:
	mkdir -p build/doxydoc/source
	./third-party/doxygen/build/bin/doxygen Doxyfile

test:
	cd tests && make test

ChangeLog: changelog/*
	cd changelog && for f in ChangeLog.*; do cat $f log >> temp; mv temp log; done;
	cp -f changelog/log ChangeLog
	@echo "Ignore that 'not found' warning"

coverage:
	cd tests && make coverage

install-dependencies:
	cd third-party && make

clean:
	rm -rf build/*
