doc: build-doxygen-xml
	cd doc && make html

build-doxygen-xml:
	mkdir -p build/doxydoc/source
	./third-party/doxygen/build/bin/doxygen Doxyfile

diff-cover: coverage
	git diff origin/master > scripts/diff
	cd scripts && python3 dumb_diff_cover.py -i ignore_list diff

travis: 
	cd third-party && make travis-install-dependencies
	make diff-cover

test:
	cd tests && make test

coverage:
	cd tests && make coverage

coverage-html:
	cd tests && make coverage-html

install-dependencies:
	cd third-party && make

install-testing-dependencies:
	cd third-party && make install-testing-dependencies

clean:
	rm -rf build/*
	cd tests && make clean
clean-dep:
	cd third-party && make clean
